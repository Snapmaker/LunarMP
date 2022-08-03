//
// Created by Cyril on 2022/7/26.
//

#include "ModelNesting.h"
#include <algorithm>
#include <set>
#include <map>

#include "../polygon/PolygonUtils.h"
#include "../polygon/AngleRange.h"

namespace lunarmp {

bool ModelNesting::readFile(std::string input_file) {
    std::ifstream ifs(input_file);
    if (!ifs) {
        std::cout << "Invaild input!" << std::endl;
        return false;
    }
    std::string str((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>()); //?

    rapidjson::Document doc;
    doc.Parse(str.c_str());

    if(doc.HasMember("plate")){
        Plate plate;
        const rapidjson::Value& plateV = doc["plate"];
        if(plateV.HasMember("polygon")) {
            const rapidjson::Value& polyV = plateV["polygon"];
            if (polyV.HasMember("x") && polyV.HasMember("y")) {
                const rapidjson::Value& x = polyV["x"];
                const rapidjson::Value& y = polyV["y"];
                if (x.IsArray() && y.IsArray()){
                    Polygon_2 outer;
                    for (int i = 0; i < x.Size(); i++) {
                        outer.push_back(Point_2(x[i].GetDouble(), y[i].GetDouble()));
                    }
                    plate.area = outer.area();
                    plate.abs_area = std::fabs(plate.area);
                    plate.polygon = Polygon_with_holes_2(outer);
//                    printPolygonWithHoles(plate.polygon);
                }
            }
        }
        plates.emplace_back(plate);
    }

    if (doc.HasMember("parts")) {
        const rapidjson::Value& partsV = doc["parts"];
        if (partsV.IsArray()) {
            for (int i = 0; i < partsV.Size(); i++) {
                const rapidjson::Value& partV = partsV[i];
                Part part;
                if (partV.HasMember("id")) {
                    part.id = partV["id"].GetInt();
                }
                if (partV.HasMember("polygon")) {
                    const rapidjson::Value& polys = partV["polygon"];
                    if (polys.IsArray()) {
                        Polygon_2 outer;
                        std::vector<Polygon_2> inner;
                        for (int j = 0; j < polys.Size(); j++) {
                            const rapidjson::Value& poly = polys[j];
                            Polygon_2 p;
                            if (poly.HasMember("x") && poly.HasMember("y")) {
                                const rapidjson::Value& x = poly["x"];
                                const rapidjson::Value& y = poly["y"];
                                if (x.IsArray() && y.IsArray()){
                                    for (int k = 0; k < x.Size(); k++) {
                                        p.push_back(Point_2(x[k].GetDouble(), y[k].GetDouble()));
                                    }
                                }
                            }
                            if (j == 0) {
                                outer = p;
                                part.area = outer.area();
                                part.abs_area = std::fabs(part.area);
                            }
                            else {
                                inner.emplace_back(p);
                            }
                        }
                        part.polygon = Polygon_with_holes_2(outer, inner.begin(), inner.end());
//                        printPolygonWithHoles(part.polygon);
                    }
                }
                if (partV.HasMember("center")) {
                    const rapidjson::Value& c = partV["center"];
                    part.center = Point_2(c["x"].GetDouble(), c["y"].GetDouble());
                }
                parts.emplace_back(part);
            }
        }
    }
    if (doc.HasMember("angle")) {
        rotate = doc["angle"].GetInt();
    }
    if (doc.HasMember("offset")) {
        offset = doc["offset"].GetInt();
    }
    return true;
}

//void ModelNesting::writeFile(std::string output_file) {
//    rapidjson::Document doc;
//    doc.SetObject();
//    rapidjson::Document::AllocatorType& allocator = allocator;
//    rapidjson::Value res(rapidjson::kArrayType);
//
//    for (Part part : result_parts) {
//        rapidjson::Value p(rapidjson::kObjectType);
//        // model id
//        p.AddMember("id", part.id, allocator);
//        // polygon
//        std::vector<Point_2> points(part.rotate_polygon.vertices_begin(), part.rotate_polygon.vertices_end());
//        rapidjson::Value poly(rapidjson::kArrayType);
//        for (Point_2 p : points) {
//            rapidjson::Value point(rapidjson::kObjectType);
//            point.AddMember("x", p.x(), allocator);
//            point.AddMember("y", p.y(), allocator);
//            poly.PushBack(point, allocator);
//        }
//        p.AddMember("polygon", poly, allocator);
//        // position
//        rapidjson::Value position(rapidjson::kObjectType);
//        position.AddMember("x", part.position.x(), allocator);
//        position.AddMember("y", part.position.y(), allocator);
//        p.AddMember("position", position, allocator);
//        // center
//        rapidjson::Value center(rapidjson::kObjectType);
//        center.AddMember("x", part.center.x(), allocator);
//        center.AddMember("y", part.center.y(), allocator);
//        p.AddMember("center", center, allocator);
//        // in place
//        p.AddMember("in_place", part.in_place, allocator);
//
//        res.PushBack(p, allocator);
//    }
//
//    std::ofstream ofs(output_file);
//    rapidjson::OStreamWrapper osw(ofs);
//
//    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
//    doc.Accept(writer);
//}
//
//void ModelNesting::updatePolygonPosition(Polygon_with_holes_2& polygon, Point_2 pos) {
//
//}

void ModelNesting::getRotatePolygons(Polygon_with_holes_2& rotate_polygon, int i, Point_2& rotateCenter, Point_2& center) {
    rotatePolygons(rotate_polygon, i, center);
    roundAndMulPolygons(rotate_polygon);

    Point_2 offset = Point_2(-rotate_polygon.bbox().xmin(), -rotate_polygon.bbox().ymin());
    movePolygons(rotate_polygon, offset);
    rotateCenter = add(center, offset);
}

void ModelNesting::polygon2Vectors(Polygon_2& polygon, std::vector<TraceLine>& vectors) {
    std::vector<Point_2> points = getVertices(polygon);
    for (int i = 0; i < points.size(); i++) {
        Point_2 p1 = points[i];
        Point_2 p2 = points[(i+1) % points.size()];
        TraceLine tl(Segment_2(p1, p2));
        tl.v = p2 - p1;
        vectors.emplace_back(tl);
    }
}

void ModelNesting::calculateTraceLines(Polygon_2& anglePolygon, Polygon_2& linesPolygon, std::vector<TraceLine>& trace_lines) {
    std::vector<TraceLine> line_vectors;
    polygon2Vectors(linesPolygon, line_vectors);
//    printTraceLines(line_vectors, false);
//    std::cout << "!!!!!!!!\n";
    std::vector<Point_2> points = getVertices(anglePolygon);

    for (int i = 0; i < points.size(); i++) {
        Point_2 p1 = i == 0 ? points[points.size()-1] : points[i-1];
        Point_2 p2 = points[i];
        Point_2 p3 = points[(i+1) % points.size()];

        Vector_2 angle_vector1 = p1 - p2;
        Vector_2 angle_vector2 = p3 - p2;

        int angle_start = angle(angle_vector1) - 90;
        int angle_end = angle(angle_vector2) + 90;

        AngleRange range = AngleRange(angle_start, angle_end);
        range.init();
        if (range.get_range() >= 180) {
            continue;
        }
        for (TraceLine line : line_vectors) {
            int angle_vector = angle(line.v) - 90;

            if (range.between(angle_vector)) {
                trace_lines.emplace_back(TraceLine(line.l, p2));
            }
        }
    }
}

void ModelNesting::generateTraceLine(Polygon_2& plate, Polygon_2& part, Point_2& center, std::vector<Segment_2>& trace_lines) {
    // 1. part-angle is in contact with plate-edge
    std::vector<TraceLine> trace_lines1;
    calculateTraceLines(part, plate, trace_lines1);
    for (TraceLine tLine : trace_lines1) {
        Point_2 p1 = add(center, sub(tLine.l.source(), tLine.p));
        Point_2 p2 = add(center, sub(tLine.l.target(), tLine.p));
        trace_lines.emplace_back(Segment_2(p1, p2));
    }

    // 2. part-edge is in contact with plate-angle
    std::vector<TraceLine> trace_lines2;
    calculateTraceLines(plate, part, trace_lines2);
    for (TraceLine tLine : trace_lines2) {
        Point_2 p1(add(center, sub(tLine.p, tLine.l.source())));
        Point_2 p2(add(center, sub(tLine.p, tLine.l.target())));
        trace_lines.emplace_back(Segment_2(p1, p2));
    }
//    printTraceLines(trace_lines, false);
}

bool cmpDirection1(Point_2& a, Point_2& b) {
    return isEqual(a.x(), b.x()) ? a.y() < b.y() : a.x() < b.x();
}

bool cmpDirection2(Point_2& a, Point_2& b) {
    return isEqual(a.x(), b.x()) ?b.y() < a.y() : b.x() < a.x();
}

bool cmp2(Segment_2& a, Segment_2& b) {
    if (!isEqual(a.source().x(), b.source().x())) {
        return a.source().x() < b.source().x();
    }
    if (!isEqual(a.source().y(), b.source().y())) {
        return a.source().y() < b.source().y();
    }
    if (!isEqual(a.target().x(), b.target().x())) {
        return a.target().x() < b.target().x();
    }
    if (!isEqual(a.target().y(), b.target().y())) {
        return a.target().y() < b.target().y();
    }
    return false;
}

void ModelNesting::processCollinear(std::vector<Segment_2>& trace_lines) {
//    printTraceLines(trace_lines, true);
    if (trace_lines.size() == 0) {
//        logWarning("processCollinear: None Lines\n");
        return ;
    }
//    std::cout << "size: " << trace_lines.size() << std::endl;

    std::vector<std::vector<Point_2>>trace_inter_points;
    for (int i = 0; i < trace_lines.size(); i++) {
        const Segment_2 tLine1 = trace_lines[i];

        std::vector<Point_2> inter_points;
        for (int j = 0; j < trace_lines.size(); j++) {
            if (i == j) {
                continue;
            }
            const Segment_2 tLine2 = trace_lines[j];

            if (parallel(tLine1, tLine2)) {
                if (tLine1.has_on(tLine2.source())) {
                    inter_points.emplace_back(tLine2.source());
                }
                if (tLine1.has_on(tLine2.target())) {
                    inter_points.emplace_back(tLine2.target());
                }
            }
            else {
                const auto result = intersection(tLine1, tLine2);
                if (result) {
                    if (const Point_2* p = boost::get<Point_2 >(&*result)) {
                        inter_points.emplace_back(Point_2(approximate((*p).x()), approximate((*p).y())));
                    }
                }
            }
        }
//        printPoints(inter_points);
        trace_inter_points.emplace_back(inter_points);
    }
    std::vector<Segment_2> new_trace_lines;
    for (int i = 0; i < trace_lines.size(); i++) {
        Segment_2 tLine = trace_lines[i];
        std::vector<Point_2> inter_points = trace_inter_points[i];

        if (inter_points.size() > 1) {
            bool sort_direction = getDirection(tLine.source(), tLine.target());
            if (sort_direction) {
                std::sort(inter_points.begin(), inter_points.end(), cmpDirection1);
            }
            else {
                std::sort(inter_points.begin(), inter_points.end(), cmpDirection2);
            }
        }

        Point_2 last_point = tLine.source();
        for (Point_2 p : inter_points) {
            if (last_point == p) {
                continue;
            }
            new_trace_lines.emplace_back(Segment_2(last_point, p));
            last_point = p;
        }
        if (!isEqualPoint(last_point, tLine.target())) {
            new_trace_lines.emplace_back(Segment_2(last_point, tLine.target()));
        }
    }

    // Delete the same segment
    sort(new_trace_lines.begin(), new_trace_lines.end(), cmp2);
//    coutLines(new_trace_lines);
    std::vector<Segment_2> res_trace_lines;
    res_trace_lines.emplace_back(new_trace_lines[0]);
    Segment_2 last_trace_line = res_trace_lines[0];
    for (int i = 1; i < new_trace_lines.size(); i++) {
        const Segment_2 tl = new_trace_lines[i];
        if (isEqualPoint(last_trace_line.source(), tl.source()) &&
            isEqualPoint(last_trace_line.target(), tl.target())) {
            continue;
        }
        last_trace_line = tl;
        res_trace_lines.emplace_back(last_trace_line);
    }
    trace_lines = res_trace_lines;
//    std::cout << "----------------------\n";
//    coutLines(trace_lines);
//    printTraceLines(new_trace_lines, true);
//    std::cout << "----------------------\n";
}

void ModelNesting::deleteOutTraceLine(std::vector<Segment_2>& trace_lines, Polygon_2& platePolygon, Point_2& center){
    if (trace_lines.size() == 0) {
        logWarning("deleteOutTraceLine: None Lines\n");
        return ;
    }
    std::vector<Segment_2> new_trace_lines;

    for (int i = 0; i < trace_lines.size(); i++) {
        Segment_2 line = trace_lines[i];
        if (line.source() == line.target()) {
            continue;
        }
        if (!platePolygon.has_on_bounded_side(line.source()) || !platePolygon.has_on_bounded_side(line.target())) {
            continue;
        }
        new_trace_lines.emplace_back(trace_lines[i]);
    }
    trace_lines = new_trace_lines;
//    coutLines(trace_lines);

}

bool lessThanLowerPoint(Point_2 point1, Point_2 point2) {
    Point_2 origin(0.0, 0.0);
    const Vector_2 v1(point1 - origin);
    const Vector_2 v2(point2 - origin);
    return v1.squared_length() < v2.squared_length();
}

int ModelNesting::searchLowerStartPointIndex(std::vector<Segment_2>& nfpLines){
    Point_2 point = nfpLines[0].source();
    int index = 0;
    for (int i = 0; i < nfpLines.size(); i++) {
        const Segment_2& line = nfpLines[i];
        if (lessThanLowerPoint(line.source(), point)) {
            point = line.source();
            index = i;
        }
    }
    return index;
}

long long toPointHash(Point_2 point) {
    return (long long)round(point.x() * 1e4) * 1e7 + (long long)round(point.y() * 1e4);
}

std::vector<Segment_2> filterTraceLine(std::vector<Segment_2>& trace_lines, std::set<int>& delete_set){
    std::vector<Segment_2> res;
    for (int i = 0; i < trace_lines.size(); i++) {
        if (delete_set.find(i) == delete_set.end()) {
            res.emplace_back(trace_lines[i]);
        }
    }
    return res;
}

void coutMap(std::map<long long, std::vector<int>>& m) {
    for(auto it : m){
        std::vector<int> val = it.second;
        std::cout << "key: " << it.first << std::endl;
        std::cout << "value:   size:" << val.size() << std::endl;
        std::cout << "    ";
        for (int index : val) {
            std::cout << index << "    ";
        }
        std::cout << std::endl;
    }
}

void coutSet(std::set<int>& s) {
    std::cout << "set: \n";
    std::cout << "    ";
    for (int it : s) {
        std::cout << it << "    ";
    }
    std::cout << std::endl;
}

void ModelNesting::traverTraceLines(std::vector<Segment_2>& trace_lines, std::vector<Segment_2>& new_trace_lines) {
    new_trace_lines.clear();
//    log("traverTraceLines\n");
    while (true) {
        if (trace_lines.size() == 0) {
//            logWarning("traverTraceLines: None Lines\n");
            return;
        }

        int lower_index = searchLowerStartPointIndex(trace_lines);

        std::set<int> trace_index_set;
        trace_index_set.insert(lower_index);

        std::vector<int> trace_index;
        trace_index.emplace_back(lower_index);

        std::map<long long, std::vector<int>> trace_index_map;
//        std::map<Point_2, int>::iterator it;
        for (int i = 0; i < trace_lines.size(); i++) {
            const Segment_2 trace_line = trace_lines[i];
            long long key = toPointHash(trace_line.source());
            if (trace_index_map.find(key) == trace_index_map.end()) {
                trace_index_map[key] = std::vector<int>();
             }
             trace_index_map[key].emplace_back(i);
        }
//        coutMap(trace_index_map);

        int current_point_index = lower_index;
        int circle_cnt = 0;
        while(true) {
            long long key = toPointHash(trace_lines[current_point_index].target());
            if (trace_index_map.find(key) == trace_index_map.end()) {
                break;
            }
            std::vector<int> next_indexs = trace_index_map[key];
            int current_angle = angle(trace_lines[current_point_index].source() - trace_lines[current_point_index].target());
            int tmp_angle = 180;
            int next_point_index = -1;

            for (int i = 0; i < next_indexs.size(); i++) {
                int index = next_indexs[i];
                int next_angle = angle(trace_lines[index].source() - trace_lines[index].target());
                int diff_angle = next_angle - current_angle;

                if (diff_angle > 180) {
                    diff_angle -= 360;
                }
                if (diff_angle < -180) {
                    diff_angle += 360;
                }
                if (diff_angle < tmp_angle) {
                    tmp_angle = diff_angle;
                    next_point_index = index;
                }
            }

            if (next_point_index != -1) {
                current_point_index = next_point_index;
                trace_index.emplace_back(next_point_index);
                if (trace_index_set.find(next_point_index) == trace_index_set.end()) {
                    circle_cnt = 0;
                    trace_index_set.insert(next_point_index);
                }
                else {
                    circle_cnt ++;
                    if (circle_cnt >= 2) {
                        break;
                    }
                }
            }
            else {
                break;
            }
        }
//        std::cout << "trace_index:" << std::endl;
//        for (int i = 0; i < trace_index.size(); i++) {
//            std::cout << trace_index[i] << "    ";
//        }
//        std::cout << std::endl;
//        coutSet(trace_index_set);

        int cycle_index = -1;
        int last_index = trace_index[trace_index.size() - 1];
        for (int i = 0 ; i < trace_index.size()-1; i++) {
            int index = trace_index[i];
            if (index == last_index) {
                cycle_index = i;
                break;
            }
        }

        if (cycle_index != -1) {
            std::set<int> trace_cycle_set;
            for (int i = cycle_index; i < trace_index.size(); i++) {
                trace_cycle_set.insert(trace_index[i]);
            }
            for (int i = 0; i < trace_lines.size(); i++) {
                if (trace_cycle_set.find(i) == trace_cycle_set.end()) {
                    continue;
                }
                new_trace_lines.emplace_back(trace_lines[i]);
            }
//            std::cout << "trace_cycle_set" << std::endl;
//            coutSet(trace_cycle_set);
        }
//        std::cout << "new_trace_lines" << std::endl;
//        coutLines(new_trace_lines);

        std::set<int> delete_set;
        for (int i = 0; i < trace_index.size(); i++) {
            int index = trace_index[i];
            delete_set.insert(index);
        }
        trace_lines = filterTraceLine(trace_lines, delete_set);
//        std::cout << "new_trace_lines" << std::endl;
//        coutLines(new_trace_lines);
        if (new_trace_lines.size() > 0) {
            return ;
        }
    }
    return ;
}

void ModelNesting::mergeTraceLines2Polygon(Polygon_2& plate, Point_2& center, std::vector<Segment_2>& trace_lines, std::vector<std::vector<Segment_2>>& nfp_rings) {
    log(" - processCollinear.\n");
    processCollinear(trace_lines);

    log(" - deleteOutTraceLine.\n");
    deleteOutTraceLine(trace_lines, plate, center);

    log(" - find nfps.\n");
    std::vector<Segment_2> new_trace_lines;
    int i = 0;
    while(true) {
        traverTraceLines(trace_lines, new_trace_lines);
        if (new_trace_lines.size() > 0) {
//            coutLines(new_trace_lines);
            nfp_rings.emplace_back(new_trace_lines);
        }
        else {
            break;
        }
    }
    return;
}

Point_2 ModelNesting::searchLowerPosition(std::vector<Segment_2>& nfp_lines) {
    Point_2 res = nfp_lines[0].source();
    for (Segment_2 line : nfp_lines) {
        if (lessThanLowerPoint(line.source(), res)) {
            res = line.source();
        }
        if (lessThanLowerPoint(line.target(), res)) {
            res = line.target();
        }
    }
    return res;
}

void ModelNesting::generateNFP(Plate& plate, Part& part, Part& result_part) {
    log("generateNFP.\n");

    for (int i = 0; i < 360; i += rotate) {
        Polygon_with_holes_2 rotated_poly = part.polygon;
        Point_2 rotated_center;
        getRotatePolygons(rotated_poly, i, rotated_center, part.center);
        std::cout << "angle:" << i << std::endl;

        std::vector<Segment_2> trace_lines;
        generateTraceLine(plate.polygon.outer_boundary(), rotated_poly.outer_boundary(), rotated_center, trace_lines);
        std::vector<std::vector<Segment_2>> nfp_rings;
        mergeTraceLines2Polygon(plate.polygon.outer_boundary(), rotated_center, trace_lines, nfp_rings);

        if (nfp_rings.empty()) {
            continue;
        }

        Point_2 lower_point(-1.0, -1.0);
        for (std::vector<Segment_2> nfp_lines : nfp_rings) {
            Point_2 lower_point_tmp = searchLowerPosition(nfp_lines);
            std::cout << "lower_point_tmp: " << lower_point_tmp << std::endl;

            Polygon_with_holes_2 movePolygon = rotated_poly;
            movePolygons(movePolygon, sub(lower_point_tmp, rotated_center));
//            std::cout << "movePolygon: " << std::endl;
//            printPolygonWithHoles(movePolygon);
            Polygon_with_holes_2 diff_polygons = differencePolygon(plate.polygon.outer_boundary(), movePolygon.outer_boundary());

//            printPolygons(diff_polygons);
            // to
            if(!diff_polygons.outer_boundary().is_empty()) {
                double tmp_area = getPwhArea(diff_polygons);

                if(tmp_area > accuracy * accuracy) {
                    continue;
                }
            }
            lower_point = lower_point_tmp;
        }
        if (lower_point == Point_2(-1.0, -1.0)) {
            continue;
        }

        Part rp(lower_point, i, rotated_center, rotated_poly);
        if (i == 0) {
            result_part = rp;
        }
        else {
            if (lessThanLowerPoint(result_part.position, rp.position)) {
                result_part = rp;
            }
        }
    }
}


void ModelNesting::updateCurrentPlate(Plate& plate, Polygon_with_holes_2& pwh) {
    Polygon_with_holes_2 offset_poly = offsetPolygon(pwh, -plate_offset);
    Point_2 inner_polys;
    for

}

bool ModelNesting::partPlacement(Plate& plate, Part& part, Part& result_part) {
    log("partPlacement.\n");
    generateNFP(plate, part, result_part);
    if (!result_part.rotate_polygon.outer_boundary().is_empty()) {
        return false;
    }

    roundAndMulPoint(result_part.position);

    movePolygons(result_part.rotate_polygon, sub(result_part.position, result_part.center));

    Polygon_with_holes_2 diff_plate_polygons = differencePolygon(plate.polygon.outer_boundary(), result_part.rotate_polygon.outer_boundary());

    updateCurrentPlate(plate, diff_plate_polygons);

    return true;
}

bool cmp (Plate& a, Plate& b) {
    return a.abs_area > b.abs_area;
}
void ModelNesting::sortPlates(std::vector<Plate>& plates) {
    for (int i = 0; i < plates.size(); i++) {
        plates[i].id = i;
    }

    std::sort(plates.begin(), plates.end(), cmp);
}

void ModelNesting::startNFP() {
    for (Part part : parts) {
        if (part.in_place) {
            continue;
        }

        sortPlates(plates);

        for (Plate plate : plates) {
            if (plate.abs_area < part.abs_area) {
                continue;
            }

            Part result_part;
            if (partPlacement(plate, part, result_part)) {
                part = result_part;
                break ;
            }
        }
        result_parts.emplace_back(part);
    }

    for (Part part : result_parts) {
        if (part.is_rotated) {
            roundAndMulPolygons(part.polygon, 1 / accuracy);
            roundAndMulPoint(part.center, 1 / accuracy);
            roundAndMulPoint(part.position, 1 / accuracy);
            part.area /= accuracy;
            part.abs_area /= accuracy;
        }
    }
}

void ModelNesting::modelNesting(std::string input_file, std::string output_file, DataGroup& data_group) {
    // get data_group
    log("read file\n");
    readFile(input_file);

    log("start nfp\n");
    startNFP();
//    offsetPolygon(plates[0].polygon, (FT)1);
//    offsetPolygon(parts[0].polygon, -1);

    //    writeFile(output_file);
}

}
