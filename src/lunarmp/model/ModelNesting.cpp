//
// Created by Cyril on 2022/7/26.
//

#include "ModelNesting.h"

#include "../polygon/PolygonUtils.h"
#include "../polygon/PolygonBool.h"
#include "../polygon/AngleRange.h"

#include <algorithm>
#include <cstdio>
#include <set>
#include <map>

namespace lunarmp {

void Plate::printPlate() {
    std::cout << "id: " << id << std::endl;
    std::cout << "area: " << area << "\tabsArea:" << abs_area << std::endl;
    std::cout << "Polygon: \n";
    printPolygonWithHoles(polygon);
}

void Part::printPart() {
    std::cout << "id: " << id << std::endl;
    std::cout << "area: " << area << "\tabsArea:" << abs_area << std::endl;
    std::cout << "position: " << position << std::endl;
    std::cout << "center: " << center << std::endl;
    std::cout << "Polygon: \n";
    printPolygonWithHoles(polygon);
    coutPwh(polygon);
}

void PartGroup::initialize()  {
    if (models.size() == 0) {
        return ;
    }

    std::vector<Point_2> points;
    std::vector<Point_2> tmp;
    for (Part model : models) {
        tmp = pwhToVertices(model.polygon);
        points.insert(points.end(), tmp.begin(), tmp.end());
        tmp.clear();
    }
    std::vector<Point_2> result;
    CGAL::convex_hull_2( points.begin(), points.end(), std::back_inserter(result));

    convex_hull.polygon = verticesToPwh(result);
    convex_hull.center = getCenter(convex_hull.polygon);
    convex_hull.initializeArea();
}

Polygon_2 readPolygon(const rapidjson::Value& polyV) {
    if (polyV.Size() == 0) {
        logError("--- Empty poly value\n");
        return Polygon_2();
    }

    Polygon_2 polygon;
    for (int i = 0; i < polyV.Size(); i++) {
        const rapidjson::Value& point = polyV[i];
        polygon.push_back(Point_2(point[0].GetDouble(), point[1].GetDouble()));
    }

    return polygon;
}

Polygon_with_holes_2 readPolygonWithHoles(const rapidjson::Value& polysV) {
    const rapidjson::Value& outerV = polysV[0];
    Polygon_2 outer = readPolygon(outerV);
    if (polysV.Size() >= 2) {
        std::vector<Polygon_2> holes;
        for (int i = 1; i < polysV.Size(); i++) {
            const rapidjson::Value& hols = polysV[i];
            holes.emplace_back(readPolygon(hols));
        }
        return Polygon_with_holes_2(outer, holes.begin(), holes.end());
    }

    return Polygon_with_holes_2(outer);
}

Part readPart(const rapidjson::Value& itemV) {
    log("readPart\n");
    Part part;

    if (itemV.HasMember("id")) {
        part.id = itemV["id"].GetInt();
    }
    else {
        logError("Missing parameter 'id' in json part %d.\n", part.id);
        exit(2);
    }

    if (itemV.HasMember("polygon_with_holes")) {
        const rapidjson::Value& pwhV = itemV["polygon_with_holes"];
        part.polygon = Polygon_with_holes_2(readPolygonWithHoles(pwhV));
    }
    else if (itemV.HasMember("polygon")) {
        const rapidjson::Value& pwhV = itemV["polygon"];
        part.polygon = Polygon_with_holes_2(readPolygon(pwhV));
        movePolygons(part.polygon, sub(Point_2(0, 0), findLowerPointInPolygon(part.polygon)));
        if (part.polygon.outer_boundary().area() < 0) {
            reversePolygon(part.polygon);
        }
    }

    if (itemV.HasMember("center")) {
        part.center = Point_2(itemV["center"][0].GetDouble(), itemV["center"][1].GetDouble());
    }
    else {
        part.center = getCenter(part.polygon);
    }

    part.initializeArea();
    return part;
}

bool ModelNesting::readFile(std::string input_file) {
    log("read file\n");

    std::ifstream ifs(input_file);
    if (!ifs) {
        logError("Invaild input!\n");
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
            plate.polygon = Polygon_with_holes_2(readPolygon(plateV["polygon"]));
            move_vector = sub(Point_2(0, 0), findLowerPointInPolygon(plate.polygon));
            movePolygons(plate.polygon, move_vector);
            if (plate.polygon.outer_boundary().area() > 0) {
                reversePolygon(plate.polygon);
            }
            plate.updateArea();
        }
        plates.emplace_back(plate);
    }
    else {
        logError("Missing parameter 'plate' in json file.\n");
        exit(2);
    }

    if (doc.HasMember("rotation")) {
        is_rotation = doc["rotation"].GetBool();
        if (!is_rotation) {
            rotate = 360;
        }
        else {
            if (doc.HasMember("rotation_step_degree")) {
                rotate = doc["rotation_step_degree"].GetInt();
            }
            else {
                logError("Missing parameter 'rotation_step_degree' in json file.\n");
                exit(2);
            }
        }
    }
    else {
        logError("Missing parameter 'rotation' in json file.\n");
        exit(2);
    }

    if (doc.HasMember("distance")) {
        offset = doc["distance"].GetInt();
    }
    else {
        logError("Missing parameter 'distance' in json file.\n");
        exit(2);
    }

    if (doc.HasMember("items")) {
        const rapidjson::Value& itemsV = doc["items"];
        if (itemsV.IsArray()) {
            for (int i = 0; i < itemsV.Size(); i++) {
                const rapidjson::Value& itemV = itemsV[i];
                std::string type = itemV["type"].GetString();

                if (type == "polygon_with_holes" || type == "polygon") {
                    parts.emplace_back(readPart(itemV));
                }
                else if (type == "group") {
                    PartGroup group;
                    if (itemV.HasMember("id")) {
                        group.convex_hull.id = itemV["id"].GetInt();
                        group.convex_hull.is_group = itemV["id"].GetInt();
                    }
                    if (itemV.HasMember("polygon_group")) {
                        const rapidjson::Value& groupsV = itemV["polygon_group"];
                        if (groupsV.IsArray()) {
                            for (int j = 0; j < groupsV.Size(); j++) {
                                const rapidjson::Value& groupV = groupsV[j];
                                if (groupV.HasMember("polygon_with_holes") || groupV.HasMember("polygon")) {
                                    group.models.emplace_back(readPart(groupV));
                                }
                                else {
                                    logError("item %d has not 'polygons'!\n", group.convex_hull.id);
                                    exit(2);
                                }
                            }
                            group.initialize();
                            for (Part& part : group.models) {
                                part.is_group = group.convex_hull.id;
                            }
                            parts.emplace_back(group.convex_hull);
                            part_groups[group.convex_hull.id] = group;
                        }
                    }
                }
                else {
                    logError("Missing parameter 'polygon' in group!\n");
                    exit(2);
                }
            }
        }
    }
    else {
        logError("Missing parameter 'items' in json file.\n");
        exit(2);
    }
    return true;
}

rapidjson::Value writePoint(Point_2 p, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value point(rapidjson::kArrayType);
    point.PushBack(approximate(p.x(), 2), allocator);
    point.PushBack(approximate(p.y(), 2), allocator);
    return point;
}

rapidjson::Value writePolygon(Polygon_2 polygon, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value point_array(rapidjson::kArrayType);
    for (Polygon_2::Vertex_const_iterator vit = polygon.vertices_begin(); vit != polygon.vertices_end(); ++vit) {
        point_array.PushBack(writePoint(*vit, allocator), allocator);
    }
    return point_array;
}

void writePolygonWithHoles(rapidjson::Value& polygons, Polygon_with_holes_2 pwh, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value polys(rapidjson::kArrayType);
    polygons.PushBack(writePolygon(pwh.outer_boundary(), allocator), allocator);

    if (pwh.number_of_holes() > 0) {
        for (Polygon_2 poly : pwh.holes()) {
            polygons.PushBack(writePolygon(poly, allocator), allocator);
        }
    }
}

void writePart(rapidjson::Value& part_array, Part& part, rapidjson::Document::AllocatorType& allocator, bool is_rotation) {
    rapidjson::Value part_obj(rapidjson::kObjectType);
    part_obj.SetObject();
    part_obj.AddMember("id", part.id, allocator);

    if (part.is_group == -1) {
        if (part.polygon.number_of_holes() > 0) {
            part_obj.AddMember("type", "polygon_with_holes", allocator);
        }
        else {
            part_obj.AddMember("type", "polygon", allocator);
        }

        part_obj.AddMember("plate_number", part.in_plate, allocator);

        if (is_rotation) {
            part_obj.AddMember("rotation_degree", part.rotation_degree, allocator);
        }
        else {
            part_obj.AddMember("rotation_degree", 0, allocator);
        }
    }

    part_obj.AddMember("end_position", writePoint(part.position, allocator), allocator);
    part_obj.AddMember("start_position", writePoint(part.center, allocator), allocator);

    rapidjson::Value polygons(rapidjson::kArrayType);
    if (part.polygon.number_of_holes() > 0) {
        writePolygonWithHoles(polygons, part.polygon, allocator);
        part_obj.AddMember("polygon_with_holes", polygons, allocator);
    }
    else {
        polygons = writePolygon(part.polygon.outer_boundary(), allocator);
        part_obj.AddMember("polygon", polygons, allocator);
    }
    part_array.PushBack(part_obj, allocator);
}

void writeGroup(rapidjson::Value& part_array, PartGroup& group, Part& part, rapidjson::Document::AllocatorType& allocator, bool is_rotation, Point_2& move_vector) {
    rapidjson::Value group_obj(rapidjson::kObjectType);
    group_obj.SetObject();
    group_obj.AddMember("id", group.convex_hull.id, allocator);
    group_obj.AddMember("type", "group", allocator);
    group_obj.AddMember("plate_number", part.in_plate, allocator);

    if (is_rotation == true) {
        group_obj.AddMember("rotation_degree", part.rotation_degree, allocator);
    }
    else {
        group_obj.AddMember("rotation_degree", 0, allocator);
    }

    rapidjson::Value parts_obj(rapidjson::kArrayType);
    for (Part& model : group.models) {
        movePolygons(model.polygon, Point_2(-move_vector.x(), move_vector.y()));
        model.position = sub(model.position, move_vector);
        model.center = getCenter(model.polygon);
        writePart(parts_obj, model, allocator, is_rotation);
    }
    group_obj.AddMember("polygon_group", parts_obj, allocator);
    part_array.PushBack(group_obj, allocator);
}

void ModelNesting::createJson(rapidjson::Document& doc) {
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value part_array(rapidjson::kArrayType);

    for (Part part : result_parts) {
        if (part.is_group == -1) {
            movePolygons(part.polygon, Point_2(-move_vector.x(), move_vector.y()));
            part.position = sub(part.position, move_vector);
            part.center = getCenter(part.polygon);
            writePart(part_array, part, allocator, is_rotation);
        }
        else {
            PartGroup group = part_groups[part.is_group];
            writeGroup(part_array, group, part, allocator, is_rotation, move_vector);
        }
    }

    doc.AddMember("nesting_result", part_array, allocator);
}

bool ModelNesting::writeFile(std::string output_file) {
    log("write\n");
    FILE* file = fopen(output_file.c_str(), "wb");
    if (!file) {
        logError("Couldn't write JSON file: %s\n", output_file.c_str());
        return false;
    }

    rapidjson::Document doc;
    createJson(doc);

    char writeBuffer[4096];
    rapidjson::FileWriteStream os(file, writeBuffer, sizeof(writeBuffer));

    rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
    doc.Accept(writer);
    fclose(file);

    return true;
}

void ModelNesting::getRotatePolygons(Polygon_with_holes_2& rotate_polygon, int i, Point_2& rotateCenter, Point_2& center) {
    rotatePolygons(rotate_polygon, i, center);
    rotate_polygon = roundAndMulPolygons(rotate_polygon);

    Point_2 tmp = getBBoxMinn(rotate_polygon.outer_boundary());
    Point_2 offset = Point_2(-tmp.x(), -tmp.y());
    movePolygons(rotate_polygon, offset);
    rotateCenter = add(center, offset);
}

void ModelNesting::polygon2Vectors(Polygon_2& polygon, std::vector<TraceLine>& vectors) {
    std::vector<Point_2> points = polygonToVertices(polygon);
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
    std::vector<Point_2> points = polygonToVertices(anglePolygon);

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
    if (trace_lines.size() == 0) {
        logError("processCollinear: None TraceLines! \n");
        return ;
    }

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
}

void ModelNesting::deleteOutTraceLine(std::vector<Segment_2>& trace_lines, Polygon_2& platePolygon, Point_2& center){
    if (trace_lines.size() == 0) {
        logWarning("deleteOutTraceLine: None Lines\n");
        return ;
    }
    std::vector<Segment_2> new_trace_lines;

    for (int i = 0; i < trace_lines.size(); i++) {
        Segment_2 line = trace_lines[i];
        if (line.is_degenerate()) {
            continue;
        }
        if (platePolygon.has_on_positive_side(line.source()) || platePolygon.has_on_positive_side(line.target())) {
            continue;
        }
        new_trace_lines.emplace_back(trace_lines[i]);
    }
    trace_lines = new_trace_lines;
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
        }

        std::set<int> delete_set;
        for (int i = 0; i < trace_index.size(); i++) {
            int index = trace_index[i];
            delete_set.insert(index);
        }
        trace_lines = filterTraceLine(trace_lines, delete_set);

        if (new_trace_lines.size() > 0) {
            return ;
        }
    }
    return ;
}

void ModelNesting::mergeTraceLines2Polygon(Polygon_2& plate, Point_2& center, std::vector<Segment_2>& trace_lines, std::vector<std::vector<Segment_2>>& nfp_rings) {
//    log(" - processCollinear.\n");
    processCollinear(trace_lines);

//    log(" - deleteOutTraceLine.\n");
    deleteOutTraceLine(trace_lines, plate, center);

//    log(" - find nfps.\n");
    std::vector<Segment_2> new_trace_lines;
    while(true) {
        traverTraceLines(trace_lines, new_trace_lines);
        if (new_trace_lines.size() > 0) {
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
        // 0. calculate rotated polygon
        Polygon_with_holes_2 rotated_poly = part.polygon;
        Point_2 rotated_center;
        getRotatePolygons(rotated_poly, i, rotated_center, part.center);

        // 1. calculate trace lines
        std::vector<Segment_2> trace_lines;
        generateTraceLine(plate.polygon.outer_boundary(), rotated_poly.outer_boundary(), rotated_center, trace_lines);

        // 2. find nfp rings
        std::vector<std::vector<Segment_2>> nfp_rings;
        mergeTraceLines2Polygon(plate.polygon.outer_boundary(), rotated_center, trace_lines, nfp_rings);

        if (nfp_rings.empty()) {
            log("%d, nfp ring is none!\n", i);
            continue;
        }

        // 3. update lower point
        Point_2 lower_point(-1.0, -1.0);
        for (std::vector<Segment_2> nfp_lines : nfp_rings) {
            Point_2 lower_point_tmp = searchLowerPosition(nfp_lines);

            Polygon_with_holes_2 movePolygon = rotated_poly;
            movePolygons(movePolygon, sub(lower_point_tmp, rotated_center));

            std::vector<Polygon_with_holes_2> diff_polygons = polygonDifference( movePolygon.outer_boundary(), plate.polygon.outer_boundary());

            // to
            if(diff_polygons.size() > 0 && !diff_polygons[0].is_unbounded()) {
                double tmp_area = 0;
                for (Polygon_with_holes_2 pwh : diff_polygons) {
                    tmp_area += getPwhArea(pwh);
                }

                if(tmp_area > accuracy * accuracy) {
                    continue;
                }
            }
            lower_point = lower_point_tmp;
        }

        if (isEqual(lower_point.x(), -1) && isEqual(lower_point.y(), -1)) {
            continue;
        }

        // update result_part
        Part rp = part;
        rp.position = lower_point;
        rp.center = rotated_center;
        rp.polygon = rotated_poly;
        rp.rotation_degree = i;
        rp.initializeArea();

        if (result_part.polygon.is_unbounded()) {
            result_part = rp;
        }
        else {
            if (lessThanLowerPoint(result_part.position, rp.position)) {
                result_part = rp;
            }
        }
    }
    return;
}


void ModelNesting::updateCurrentPlate(Plate& currentPlate, Polygon_with_holes_2& diff_plate_polygons) {
    log("--------updateCurrentPlate\n");
    std::vector<Polygon_with_holes_2> innerPwhs;
    removeSelfIntersect(innerPwhs, diff_plate_polygons, plate_offset);

    if (innerPwhs.size() == 0) {
        currentPlate.abs_area = 0;
    }
    else {
        std::vector<Plate> tmp_plates;
        for (Polygon_with_holes_2 pwh : innerPwhs) {
            Plate new_plate = Plate(pwh.outer_boundary());
            new_plate.updateArea();
            new_plate.id = currentPlate.id;
            tmp_plates.emplace_back(new_plate);
        }
        currentPlate.polygon = tmp_plates[0].polygon;
        currentPlate.updateArea();

        for (int i = 1; i < tmp_plates.size(); i++) {
            plates.emplace_back(tmp_plates[i]);
        }
    }
}

bool cmpPwhs(Polygon_with_holes_2& a, Polygon_with_holes_2& b) {
    return a.outer_boundary().area() > b.outer_boundary().area();
}

bool ModelNesting::partPlacement(Plate& plate, Part& part, Part& result_part) {
    log("- partPlacement.\n");

    generateNFP(plate, part, result_part);

    result_part.position = roundAndMulPoint(result_part.position);
    movePolygons(result_part.polygon, sub(result_part.position, result_part.center));

    std::vector<Polygon_with_holes_2> diff_plate_polygons = polygonDifference(plate.polygon.outer_boundary(), result_part.polygon.outer_boundary());
    if (diff_plate_polygons.size() > 1) {
        std::sort(diff_plate_polygons.begin(), diff_plate_polygons.end(), cmpPwhs);
    }
    result_part.in_plate = plate.id;
    result_part.id = part.id;

    updateCurrentPlate(plate, diff_plate_polygons[0]);

    if (result_part.polygon.number_of_holes() > 0) {
        for (Polygon_2 hole : result_part.polygon.holes()) {
            Plate p_hole(hole);
            p_hole.updateArea();
            p_hole.id = plate.id;
            plates.emplace_back(p_hole);
        }
    }

    return true;
}

bool cmpPlate (Plate& a, Plate& b) {
    return a.abs_area > b.abs_area;
}

void ModelNesting::startNFP() {
    log("start NFP\n");
    for (Part& part : parts) {
        if (part.in_plate != -1) {
            continue;
        }
        std::sort(plates.begin(), plates.end(), cmpPlate);
        for (Plate& plate : plates) {
            if (plate.abs_area < part.abs_area) {
                continue;
            }
            Part result_part;
            if (partPlacement(plate, part, result_part)) {
                part = result_part;
                break;
            }
        }
        int i = 0;
        for (Plate& plate : plates) {
            if (plate.area > 0) {
                reversePolygon(plate.polygon);
            }
        }
        result_parts.emplace_back(part);
    }

    for (Part part : result_parts) {
        if (part.polygon.is_unbounded()) {
            continue;
        }
        part.polygon = roundAndMulPolygons(part.polygon, 1 / accuracy);
        part.center = roundAndMulPoint(part.center, 1 / accuracy);
        part.position = roundAndMulPoint(part.position, 1 / accuracy);
        part.area /= accuracy;
        part.abs_area /= accuracy;
    }
    log("end NFP\n");
}

bool cmpPart(Part& a, Part& b) {
    return a.abs_area > b.abs_area;
}

void ModelNesting::initialize(std::vector<Plate>& plate, std::vector<Part>& part) {
    log("initialize\n");
    Polygon_with_holes_2 simplify_polygon;
    for (int i = 0; i < parts.size(); i++) {
        simplify_polygon = parts[i].polygon;
        simplifyPolygons(simplify_polygon, limit_edge);

        if (offset > 0) {
            std::vector<Polygon_with_holes_2> simplify_polygons;
            polygonOffset(simplify_polygons, simplify_polygon, offset);
            parts[i].polygon = roundAndMulPolygons(simplify_polygons[0], accuracy);
        }
        else {
            parts[i].polygon = roundAndMulPolygons(simplify_polygon, accuracy);
        }
        parts[i].center = roundAndMulPoint(parts[i].center, accuracy);
        parts[i].in_plate = -1;
        parts[i].initializeArea();
    }
    std::sort(parts.begin(), parts.end(), cmpPart);

    for (int i = 0; i < plates.size(); i++) {
        plates[i].id = i;
        simplify_polygon = plates[i].polygon;
        simplifyPolygons(simplify_polygon, limit_edge);
        plates[i].polygon = roundAndMulPolygons(simplify_polygon, accuracy);
        plates[i].updateArea();
    }
}

void ModelNesting::modelNesting(std::string input_file, std::string output_file, DataGroup& data_group) {
    // get data_group
    if (!readFile(input_file)) {
        logError("Invalid Input File.\n");
        exit(2);
    }
    initialize(plates, parts);

    startNFP();

    if (!writeFile(output_file)) {
        logError("Invalid Output File.\n");
        exit(2);
    }
}

}
