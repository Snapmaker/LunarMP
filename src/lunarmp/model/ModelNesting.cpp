//
// Created by Cyril on 2022/7/26.
//

#include "ModelNesting.h"
#include <algorithm>
#include <cmath>

namespace lunarmp {

bool ModelNesting::readFile(std::string input_file) {
    std::ifstream ifs(input_file);
    if (!ifs) {
        std::cout << "Invaild input!" << std::endl;
        return false;
    }
    std::string str((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>());

    rapidjson::Document doc;
    doc.Parse(str.c_str());

    if(doc.HasMember("plate")){
        Plate plate;
        const rapidjson::Value& childValue = doc["plate"];
        if(childValue.HasMember("polygon")) {
            const rapidjson::Value& child2Value = childValue["polygon"];
            if (child2Value.HasMember("x") && child2Value.HasMember("y")) {
                const rapidjson::Value& x = child2Value["x"];
                const rapidjson::Value& y = child2Value["y"];
                if (x.IsArray() && y.IsArray()){
                    for (int i = 0; i < x.Size(); i++) {
                        plate.polygon.push_back(Point_2(x[i].GetDouble(), y[i].GetDouble()));
//                        const rapidjson::Value& x_pos = x[i];
//                        const rapidjson::Value& y_pos = y[i];
//                        std::cout << "x: " << x_pos.GetDouble() << "y: " << y_pos.GetDouble() << std::endl;
                    }
                }
            }
        }
//        plate.print();
        plates.emplace_back(plate);
    }

    if (doc.HasMember("parts")) {
        const rapidjson::Value& partsV = doc["parts"];
//        std::cout << partsV.Size() << "\n";
        if (partsV.IsArray()) {
            for (int i = 0; i < partsV.Size(); i++) {
                Part part;
                const rapidjson::Value& partV = partsV[i];
//                if (partV.IsNull()) continue;
                if (partV.HasMember("id")) {
                    part.id = partV["id"].GetInt();
                }
                if (partV.HasMember("polygon")) {
                    const rapidjson::Value& poly = partV["polygon"];
                    if (poly.HasMember("x") && poly.HasMember("y")) {
                        const rapidjson::Value& x = poly["x"];
                        const rapidjson::Value& y = poly["y"];
                        if (x.IsArray() && y.IsArray()){
                            for (int i = 0; i < x.Size(); i++) {
                                part.polygon.push_back(Point_2(x[i].GetDouble(), y[i].GetDouble()));
                            }
                        }
                    }
                }
                if (partV.HasMember("center")) {
                    const rapidjson::Value& c = partV["center"];
                    part.center = Point_2(c["x"].GetDouble(), c["y"].GetDouble());
                }
//                part.print();
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

void ModelNesting::writeFile(std::string output_file) {
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = allocator;
    rapidjson::Value res(rapidjson::kArrayType);

    for (Part part : result_parts) {
        rapidjson::Value p(rapidjson::kObjectType);
        // model id
        p.AddMember("id", part.id, allocator);
        // polygon
        std::vector<Point_2> points(part.rotate_polygon.vertices_begin(), part.rotate_polygon.vertices_end());
        rapidjson::Value poly(rapidjson::kArrayType);
        for (Point_2 p : points) {
            rapidjson::Value point(rapidjson::kObjectType);
            point.AddMember("x", p.x(), allocator);
            point.AddMember("y", p.y(), allocator);
            poly.PushBack(point, allocator);
        }
        p.AddMember("polygon", poly, allocator);
        // position
        rapidjson::Value position(rapidjson::kObjectType);
        position.AddMember("x", part.position.x(), allocator);
        position.AddMember("y", part.position.y(), allocator);
        p.AddMember("position", position, allocator);
        // center
        rapidjson::Value center(rapidjson::kObjectType);
        center.AddMember("x", part.center.x(), allocator);
        center.AddMember("y", part.center.y(), allocator);
        p.AddMember("center", center, allocator);
        // in place
        p.AddMember("in_place", part.in_place, allocator);

        res.PushBack(p, allocator);
    }

    std::ofstream ofs(output_file);
    rapidjson::OStreamWrapper osw(ofs);

    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
    doc.Accept(writer);
}

Point_2 ModelNesting::roundAndMulPoint(Point_2 point, double limit = 1) {
    return Point_2(std::round(point.x() * limit), std::round(point.y() * limit));
}

Polygon_2 ModelNesting::roundAndMulPolygons(Polygon_2 polygon, double limit = 1) {
    Polygon_2 res;
    for (Point_2 point : polygon) {
        res.push_back(roundAndMulPoint(point, limit));
    }
    return res;
}

void ModelNesting::updatePolygonPosition(Polygon_2& polygon, Point_2 pos) {

}

void printPolygon(Polygon_2 polygon) {
    std::vector<Point_2> points(polygon.vertices_begin(), polygon.vertices_end());
    for (Point_2 point : points) {
        std::cout << "(" << point.x() << ", " << point.y() << ")" << std::endl;
    }
}

void print_polygon (const CGAL::Polygon_2<K>& P)
{
    typename CGAL::Polygon_2<K>::Vertex_const_iterator vit;

    std::cout << "[ " << P.size() << " vertices:";
    for (vit = P.vertices_begin(); vit != P.vertices_end(); ++vit) {
        std::cout << " (" << *vit << ')';
    }
    std::cout << " ]" << std::endl;
    return;
}

void print_polygon_with_holes(const CGAL::Polygon_with_holes_2<K>& pwh) {
    if (! pwh.is_unbounded()) {
        std::cout << "{ Outer boundary = ";
        print_polygon (pwh.outer_boundary());
    }
    else {
        std::cout << "{ Unbounded polygon." << std::endl;
    }

    typename CGAL::Polygon_with_holes_2<K>::Hole_const_iterator  hit;
    unsigned int k = 1;

    std::cout << "  " << pwh.number_of_holes() << " holes:" << std::endl;
    for (hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit, ++k) {
        std::cout << "    Hole #" << k << " = ";
        print_polygon (*hit);
    }
    std::cout << " }" << std::endl;
    return;
}

void ModelNesting::getRotatePolygons(Polygon_with_holes_2& polygon, int i, Point_2& center) {

}

void ModelNesting::generateTraceLine(Polygon_2 plate, Polygon_2 part, Point_2 center, std::vector<TraceLine>& trace_lines) {

}

void ModelNesting::mergeTraceLines2Polygon(Polygon_2 plate, Point_2 center, std::vector<TraceLine>& trace_lines, std::vector<std::vector<TraceLine>>& nfp_rings) {

}
Point_2 ModelNesting::searchLowerPosition(std::vector<TraceLine> nfpLines) {

}

bool ModelNesting::generateNFP(Plate plate, Part part, Part& result_part) {

    for (int i = 0; i < 360; i += rotate) {
        Polygon_with_holes_2 rotated_poly;
        Point_2 rotated_center;
        getRotatePolygons(rotated_poly, i, rotated_center);

        std::vector<TraceLine> trace_lines;
        generateTraceLine(plate.polygon, rotated_poly.outer_boundary(), rotated_center, trace_lines);

        std::vector<std::vector<TraceLine>> nfp_rings;
        mergeTraceLines2Polygon(plate.polygon, rotated_center, trace_lines, nfp_rings);

        if (nfp_rings.empty()) {
            continue;
        }

        Point_2 lower_point;
        for (std::vector<TraceLine> nfp_lines : nfp_rings) {
            Point_2 lowerPointTmp = searchLowerPosition(nfp_lines);
            Point_2 pos(lowerPointTmp.x()-rotated_center.x(), lowerPointTmp.y()-rotated_center.y());

            Polygon_2 movePolygon;
            updatePolygonPosition(rotated_poly.outer_boundary(), pos); // 后续替换为带孔移动
            Polygon_with_holes_2 diff_polygons = differencePolygon(movePolygon, plate.polygon);
            // to

        }
    }

    return true;
}

std::list<Polygon_with_holes_2> ModelNesting::differencePolygon(Polygon_2 sub, Polygon_2 clip) {
    std::list<Polygon_with_holes_2> pwhs;
    CGAL::difference(sub, clip, std::back_inserter(pwhs), CGAL::Tag_true());

//    std::list<Polygon_with_holes_2>::const_iterator it;
//    int i = 0;
//    for (it = pwhs.begin(); it != pwhs.end(); ++it) {
//        std::cout << "i: " << i++ << std::endl;
//        Polygon_with_holes_2 pwh = *it;
////        if (pwh.is_unbounded()) {
////            std::cout << "{ Unbounded polygon." << std::endl;
////        }
////        Polygon_2 outer = pwh.outer_boundary();
//        print_polygon_with_holes(pwh);
//    }
    return pwhs;
}
std::list<Polygon_with_holes_2> ModelNesting::offsetPolygon(Polygon_with_holes_2& sub, double offset, std::string type) {

}

// 没写完
void ModelNesting::updateCurrentPlate(Plate plate, std::list<Polygon_with_holes_2> pwhs) {

    std::list<Polygon_with_holes_2>::const_iterator it;

    for (it = pwhs.begin(); it != pwhs.end(); ++it) {
        Polygon_with_holes_2 pwh = *it;

        if (pwh.is_unbounded()) {
            std::cout << "{ Unbounded polygon." << std::endl;
            //            return false;
        }

        offsetPolygon(pwh, -1 * plate_offset, "jtMiter"); // to
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

bool ModelNesting::partPlacement(Plate plate, Part part, Part& result_part) {
    if (generateNFP(plate, part, result_part)) {
        result_part.position = roundAndMulPoint(result_part.position);

        Point_2 pos(result_part.position.x() - result_part.center.x(), result_part.position.y() - result_part.center.y());
        updatePolygonPosition(result_part.rotate_polygon, pos);

        std::list<Polygon_with_holes_2> diff_polygons = differencePolygon(plate.polygon, result_part.polygon);

        updateCurrentPlate(plate, diff_polygons);

        // with hole model needs
//        if (result_part.rotate_polygon.size() > 1) {
//            for (int i = 1; i < result_part.rotate_polygon.size(); i++) {
//                plates.emplace_back(xx);
//            }
//        }
        return true;
    }
    return false;
}

bool cmp (Plate a, Plate b) {
    return a.absArea > b.absArea;
}
void ModelNesting::sortPlates(std::vector<Plate>& plates) {
    for (int i = 0; i < plates.size(); i++) {
        plates[i].id = i;
    }

    std::sort(plates.begin(), plates.end(), cmp);
}

void ModelNesting::startNFP() {
    log("start nfp\n");
    for (Part part : parts) {
        if (part.in_place) {
            continue;
        }

        sortPlates(plates);

        for (Plate plate : plates) {
            if (plate.absArea < part.absArea) {
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
            part.absArea /= accuracy;
        }
    }
}

void ModelNesting::modelNesting(std::string input_file, std::string output_file, DataGroup& data_group) {
    // get data_group
    log("read file\n");
    readFile(input_file);
    differencePolygon(plates[0].polygon, parts[0].polygon);
//    startNFP();
//    writeFile(output_file);
}

}
