//
// Created by Cyril on 2022/7/26.
//

#include "ModelNesting.h"
#include <algorithm>


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

    Point_2 offset = Point_2(rotate_polygon.bbox().xmin(), rotate_polygon.bbox().ymin());

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
                TraceLine tLine = TraceLine(line.l);
                tLine.p = p2;
                trace_lines.emplace_back(tLine);
            }
        }
    }
}

void ModelNesting::generateTraceLine(Polygon_2& plate, Polygon_2& part, Point_2& center, std::vector<TraceLine>& trace_lines) {
    // 1. part-angle is in contact with plate-edge
    std::vector<TraceLine> trace_lines1;
    calculateTraceLines(part, plate, trace_lines1);
    for (TraceLine tLine : trace_lines1) {
        Point_2 p1 = add(center, sub(tLine.l.source(), tLine.p));
        Point_2 p2 = add(center, sub(tLine.l.target(), tLine.p));
        trace_lines.emplace_back(TraceLine(Segment_2(p1, p2)));
    }

    // 2. part-edge is in contact with plate-angle
    std::vector<TraceLine> trace_lines2;
    calculateTraceLines(plate, part, trace_lines2);
    for (TraceLine line : trace_lines2) {
        Point_2 p1(add(center, sub(line.p, line.l.source())));
        Point_2 p2(add(center, sub(line.p, line.l.target())));
        trace_lines.emplace_back(TraceLine(Segment_2(p1, p2)));
    }
//    printTraceLines(trace_lines, false);
}

bool cmpDirection1(Point_2& a, Point_2& b) {
    return isEqual(a.x(), b.x()) ? approximate(a.y() - b.y()) : approximate(a.x() - b.x());
}

bool cmpDirection2(Point_2& a, Point_2& b) {
    return isEqual(a.x(), b.x()) ? approximate(b.y() - a.y()) : approximate(b.x() - a.x());
}

bool cmp2(TraceLine& a, TraceLine& b) {
    if (!isEqual(a.l.source().x(), b.l.source().x())) {
        return approximate(a.l.source().x() - b.l.source().x());
    }
    if (!isEqual(a.l.source().x(), b.l.source().x())) {
        return approximate(a.l.source().x() - b.l.source().x());
    }
    if (!isEqual(a.l.source().x(), b.l.source().x())) {
        return approximate(a.l.source().x() - b.l.source().x());
    }
    if (!isEqual(a.l.source().x(), b.l.source().x())) {
        return approximate(a.l.source().x() - b.l.source().x());
    }
    return 0;
}

void ModelNesting::processCollinear(std::vector<TraceLine>& trace_lines) {
    coutTraceLines(trace_lines);

    std::vector<TraceLine> new_trace_lines;
    if (trace_lines.size() == 0) {
        return ;
    }
    std::vector<std::vector<Point_2>>trace_inter_points;
    std::cout << "size: " << trace_lines.size() << std::endl;
    for (int i = 0; i < trace_lines.size(); i++) {
        const TraceLine tLine1 = trace_lines[i];

        std::cout << "i: " << i << std::endl;
        std::vector<Point_2> inter_points;
        for (int j = 0; j < trace_lines.size(); j++) {
            if (i == j) {
                continue;
            }
            const TraceLine tLine2 = trace_lines[j];

            if (parallel(tLine1.l, tLine2.l)) {
//                if (tLine1.l.collinear_has_on(tLine2.l.source())) {
//                    inter_points.emplace_back(tLine2.l.source());
//                }
//                if (tLine1.l.collinear_has_on(tLine2.l.target())) {
//                    inter_points.emplace_back(tLine2.l.target());
//                }
                if (K::CollinearHasOn_2(tLine1, tLine2.l.source())) {
                    inter_points.emplace_back(tLine2.l.source());
                }
            }
//            else {
//                const auto result = intersection(tLine1.l, tLine2.l);
//                if (result) {
//                    if (const Point_2* p = boost::get<Point_2 >(&*result)) {
//                        inter_points.emplace_back(Point_2(approximate((*p).x()), approximate((*p).y())));
//                    }
//                }
//            }
            trace_inter_points.emplace_back(inter_points);
        }
        std::cout << "trace: " << tLine1.l << std::endl;
        printPoints(inter_points);
        std::cout << "\n";
    }

//    for (int i = 0; i < trace_lines.size(); i++) {
//        TraceLine tLine = trace_lines[i];
//        std::vector<Point_2> inter_points = trace_inter_points[i];
//
//        if (inter_points.size() > 2) {
//            bool sort_direction = getDirection(tLine.l.source(), tLine.l.target());
//            if (sort_direction) {
//                std::sort(inter_points.begin(), inter_points.end(), cmpDirection1);
//            }
//            else {
//                std::sort(inter_points.begin(), inter_points.end(), cmpDirection2);
//            }
//        }
//
//        Point_2 last_point = tLine.l.source();
//        for (Point_2 p : inter_points) {
//            if (isEqualPoint(last_point, p)) {
//                continue;
//            }
//            new_trace_lines.emplace_back(TraceLine(Segment_2(last_point, p)));
//            last_point = p;
//        }
//        if (!isEqualPoint(last_point, tLine.l.target())) {
//            new_trace_lines.emplace_back(TraceLine(Segment_2(last_point, tLine.l.target())));
//        }
//    }

//    // Delete the same segment
//    sort(new_trace_lines.begin(), new_trace_lines.end(), cmp2);
//    std::vector<TraceLine> res_trace_lines;
//    res_trace_lines.emplace_back(new_trace_lines[0]);
//    TraceLine last_trace_line = res_trace_lines[0];
//    for (int i = 1; i < new_trace_lines.size(); i++) {
//        const TraceLine tl = new_trace_lines[i];
//        if (isEqualPoint(last_trace_line.l.source(), tl.l.source()) &&
//            isEqualPoint(last_trace_line.l.target(), tl.l.target())) {
//            continue;
//        }
//        last_trace_line = tl;
//        res_trace_lines.emplace_back(tl);
//    }
//    trace_lines = res_trace_lines;
}

//void ModelNesting::deleteOutTraceLine(std::vector<TraceLine>& trace_lines, Plate platePolygon, Point_2 center){
//
//}

//void ModelNesting::traverTraceLines(std::vector<TraceLine> trace_lines, std::vector<TraceLine>& new_trace_lines) {
//
//}

void ModelNesting::mergeTraceLines2Polygon(Polygon_2& plate, Point_2& center, std::vector<TraceLine>& trace_lines, std::vector<std::vector<TraceLine>>& nfp_rings) {
    log("processCollinear\n");
    processCollinear(trace_lines);
    log("deleteOutTraceLine\n");
//    deleteOutTraceLine(trace_lines, plate, center);
//
//    std::vector<TraceLine> new_trace_lines;
//    while(1) {
//        traverTraceLines(trace_lines, new_trace_lines);
//        if (new_trace_lines.size() > 0) {
//            nfp_rings.emplace_back(new_trace_lines);//?
//        }
//        else {
//            break;
//        }
//    }
}

//Point_2 ModelNesting::searchLowerPosition(std::vector<Segment_2> nfpLines) {
//
//}

bool ModelNesting::generateNFP(Plate& plate, Part& part, Part& result_part) {
    log("generateNFP.\n");

    for (int i = 0; i < 360; i += rotate) {
        Polygon_with_holes_2 rotated_poly = part.polygon;
        Point_2 rotated_center;
        getRotatePolygons(rotated_poly, i, rotated_center, part.center);

        std::vector<TraceLine> trace_lines;
        generateTraceLine(plate.polygon.outer_boundary(), rotated_poly.outer_boundary(), rotated_center, trace_lines);

        std::vector<std::vector<TraceLine>> nfp_rings;
        mergeTraceLines2Polygon(plate.polygon.outer_boundary(), rotated_center, trace_lines, nfp_rings);
//
//        if (nfp_rings.empty()) {
//            continue;
//        }
//
//        Point_2 lower_point;
//        for (std::vector<TraceLine> nfp_lines : nfp_rings) {
//            Point_2 lowerPointTmp = searchLowerPosition(nfp_lines);
//            Point_2 pos(lowerPointTmp.x()-rotated_center.x(), lowerPointTmp.y()-rotated_center.y());
//
//            Polygon_with_holes_2 movePolygon;
//            updatePolygonPosition(rotated_poly, pos);
//            std::list<Polygon_with_holes_2> diff_polygons = differencePolygon(movePolygon, plate.polygon);
//            // to
//
//        }
    }

    return true;
}

// 没写完
void ModelNesting::updateCurrentPlate(Plate& plate, std::list<Polygon_with_holes_2>& pwhs) {
    log("partPlacement.\n");
    std::list<Polygon_with_holes_2>::const_iterator it;

    for (it = pwhs.begin(); it != pwhs.end(); ++it) {
        Polygon_with_holes_2 pwh = *it;

        if (pwh.is_unbounded()) {
            std::cout << "{ Unbounded polygon." << std::endl;
            //            return false;
        }

//        offsetPolygon(pwh, -1 * plate_offset, "jtMiter"); // to
        Polygon_2 union_poly;

        // outer side
        Polygon_2 outer = pwh.outer_boundary();
        std::vector<Polygon_2> inner;
        // holes
        if (pwh.number_of_holes()) {
            Polygon_with_holes_2::Hole_const_iterator hit;
            for (hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit) {
                Polygon_2 hole = *hit;
            }
        }
    }
}

bool ModelNesting::partPlacement(Plate& plate, Part& part, Part& result_part) {
    log("partPlacement.\n");
    if (!generateNFP(plate, part, result_part)) {
        return false;
    }
    //
    //    result_part.position = roundAndMulPoint(result_part.position);
    //
    //    Point_2 pos(result_part.position.x() - result_part.center.x(), result_part.position.y() - result_part.center.y());
    //    updatePolygonPosition(result_part.rotate_polygon, pos);
    //
    //    std::list<Polygon_with_holes_2> diff_polygons = differencePolygon(plate.polygon, result_part.polygon);
    //
    //    updateCurrentPlate(plate, diff_polygons);


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
            roundAndMulPolygons(part.rotate_polygon, 1 / accuracy);
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

    //    writeFile(output_file);
}

}
