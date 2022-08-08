//
// Created by Cyril on 2022/8/5.
//

#include "PolygonJsonProcessor.h"

namespace lunarmp {


Point_2 PolygonJsonProcessor::readPoint(const rapidjson::Value& point) {

}

Polygon_2 PolygonJsonProcessor::readPolygon(const rapidjson::Value& polyV) {
    log(" - readPolygo   s\n");
    Polygon_2 polygon;
    for (int i = 0; i < polyV.Size(); i++) {
        const rapidjson::Value& point = polyV[i];
        polygon.push_back(Point_2(point[0].GetDouble(), point[1].GetDouble()));
    }

    log(" - End - readPolygon\n");
    return polygon;
}

Polygon_with_holes_2 PolygonJsonProcessor::readPolygonWithHoles(const rapidjson::Value& polysV) {
    log("readPolygonWithHoles\n");
    Polygon_2 outer = readPolygon(polysV[0]);

    if (polysV.Size() > 1) {
        std::vector<Polygon_2> holes;
        for (int i = 1; i < polysV.Size(); i++) {
            const rapidjson::Value& hols = polysV[i];
            holes.emplace_back(readPolygon(hols));
        }
        return Polygon_with_holes_2(outer, holes.begin(), holes.end());
    }
    log("End - readPolygonWithHoles\n");

    return Polygon_with_holes_2(outer);
}

bool PolygonJsonProcessor::readFile(std::string input_file, std::vector<Plate>& plates, std::vector<Part>& parts) {
    log("read file\n");

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
            //            const rapidjson::Value& polyV = plateV["polygon"];
            //            if (polyV.HasMember("x") && polyV.HasMember("y")) {
            //                const rapidjson::Value& x = polyV["x"];
            //                const rapidjson::Value& y = polyV["y"];
            //                if (x.IsArray() && y.IsArray()){
            //                    Polygon_2 outer;
            //                    for (int i = 0; i < x.Size(); i++) {
            //                        outer.push_back(Point_2(x[i].GetDouble(), y[i].GetDouble()));
            //                    }
            //                    plate.polygon = Polygon_with_holes_2(outer);
            //                    plate.init();
            //                }
            //            }
            plate.polygon = readPolygonWithHoles(plateV["polygon"]);
//            printPolygonWithHoles(plate.polygon);

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
                part.init();
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

rapidjson::Value PolygonJsonProcessor::writePoint(Point_2 p, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value point(rapidjson::kObjectType);
    point.AddMember("x", p.x(), allocator);
    point.AddMember("y", p.y(), allocator);
    return point;
}

rapidjson::Value PolygonJsonProcessor::writePolygon(Polygon_2 polygon, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value point_array(rapidjson::kArrayType);
    for (Polygon_2::Vertex_const_iterator vit = polygon.vertices_begin(); vit != polygon.vertices_end(); ++vit) {
        point_array.PushBack(writePoint(*vit, allocator), allocator);
    }
    return point_array;
}

void PolygonJsonProcessor::writePolygonWithHoles(rapidjson::Value &polygons, Polygon_with_holes_2 pwh, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value polys(rapidjson::kArrayType);
    polygons.PushBack(writePolygon(pwh.outer_boundary(), allocator), allocator);

    if (pwh.number_of_holes() > 0) {
        for (Polygon_2 poly : pwh.holes()) {
            polygons.PushBack(writePolygon(poly, allocator), allocator);
        }
    }
    std::cout << "---- polys" << std::endl;
    std::cout << polygons.Size() << std::endl;
}

void PolygonJsonProcessor::createJson(rapidjson::Document& doc) {
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value part_array(rapidjson::kArrayType);

    for (Part part : result_parts) {
        rapidjson::Value part_obj(rapidjson::kObjectType);
        part_obj.SetObject();
        part_obj.AddMember("id", part.id, allocator);
        part_obj.AddMember("position", writePoint(part.position, allocator), allocator);
        part_obj.AddMember("center", writePoint(part.center, allocator), allocator);

        std::cout << "--before cout" << std::endl;
        printPolygonWithHoles(part.polygon);
        printPolygonWithHoles(part.rotate_polygon);

        rapidjson::Value polygons(rapidjson::kArrayType);
        writePolygonWithHoles(polygons, part.rotate_polygon, allocator);
        std::cout << "---- polys2" << std::endl;
        std::cout << polygons.Size() << std::endl;

        part_obj.AddMember("polygon", polygons, allocator);
        part_array.PushBack(part_obj, allocator);
    }

    doc.AddMember("final_parts", part_array, allocator);
}

bool PolygonJsonProcessor::writeFile(std::string output_file) {
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


}
