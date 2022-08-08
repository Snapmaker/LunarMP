//
// Created by Cyril on 2022/8/5.
//
#pragma once

#ifndef LUNARMP_SRC_LUNARMP_JSONPROCESSOR_H_
#define LUNARMP_SRC_LUNARMP_JSONPROCESSOR_H_

#include "../model/ModelNesting.h"

#include "../../rapidjson/rapidjson/filewritestream.h"
#include "../../rapidjson/rapidjson/document.h"
#include "../../rapidjson/rapidjson/prettywriter.h"
//#include "../../rapidjson/rapidjson/writer.h"

namespace lunarmp {
class PolygonJsonProcessor {
  public:
    Point_2 readPoint(const rapidjson::Value& point);

    Polygon_2 readPolygon(const rapidjson::Value& polyV);

    Polygon_with_holes_2 readPolygonWithHoles(const rapidjson::Value& polysV);

    rapidjson::Value writePoint(Point_2 p, rapidjson::Document::AllocatorType& allocator);

    rapidjson::Value writePolygon(Polygon_2 polygon, rapidjson::Document::AllocatorType& allocator);

    void writePolygonWithHoles(rapidjson::Value& polygons, Polygon_with_holes_2 pwh, rapidjson::Document::AllocatorType& allocator);

    bool readFile(std::string input_file, std::vector<Plate>& plates, std::vector<Part>& parts);

    void createJson(rapidjson::Document& doc);

    bool writeFile(std::string output_file);

};

}  // namespace lunarmp

#endif  // LUNARMP_SRC_LUNARMP_JSONPROCESSOR_H_
