//
// Created by Cyril on 2022/7/26.
//
#pragma once

#ifndef LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
#define LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_

#include "../../rapidjson/rapidjson/filewritestream.h"
#include "../../rapidjson/rapidjson/document.h"
#include "../../rapidjson/rapidjson/prettywriter.h"
#include "../utils/logoutput.h"
#include "../utils/Enums.h"
#include "../data/DataGroup.h"
#include "../polygon/PolygonBase.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>

namespace lunarmp {

class TraceLine {
  public:
    Segment_2 l;
    Point_2 p;
    Vector_2 v;

    TraceLine() {};
    TraceLine(Segment_2 line) : l(line) {};
    TraceLine(Segment_2 line, Point_2 pos) : l(line), p(pos) {};
};

class Plate {
  public:
    Polygon_with_holes_2 polygon;
    double area;
    double abs_area;
    int id;

    Plate() {};
    Plate(Polygon_2 poly) : polygon(Polygon_with_holes_2(poly)) {};
    void updateArea() {
        area = polygon.outer_boundary().area();
        abs_area = std::fabs(area);
    }
    void printPlate();
};

class Part {
  public:
    Part(){};
    Part(Point_2 p, Point_2 c, int id, int g, Polygon_with_holes_2 poly) : position(p), center(c), id(id), group_number(g), polygon(poly) {};

    ~Part(){};

    Polygon_with_holes_2 polygon;
    Polygon_with_holes_2 rotated_polygon;
    Point_2 position = Point_2(-1, -1);
    Point_2 center = Point_2(-1, -1);
    double area = 0;
    double abs_area = 0;
    int in_plate = -1;
    int id = -1;
    int group_number = -1;
    int rotation_degree = 0;

    void printPart();
    void initializeArea() {
        area = polygon.outer_boundary().area();
        abs_area = std::fabs(area);
    }
};

class PartGroup {
  public:
    PartGroup() {};
    ~PartGroup() {};

    std::vector<Part> models;
    Part convex_hull;
    int id = -1;
    void initialize();
};

class ModelNesting {
  private:
    double accuracy = 1;
    int plate_offset = 10;

  public:
    std::vector<Part> parts;
    std::map<int, PartGroup> part_groups;
    std::vector<Plate> plates;
    std::vector<Part> result_parts;
    int rotate = 360;
    int offset = 0;
    int limit_edge = 2;
    bool is_rotation = false;
    Point_2 move_vector;
    int plate_number = 1;

    void initialize(std::vector<Plate>& plate, std::vector<Part>& part);

    void printTraceLines(std::vector<TraceLine> tls, bool hasPos) {
        std::cout << "Size: " << tls.size() << std::endl;
        for (TraceLine tl: tls) {
            std::cout << "l: (" << tl.l << ")" << std::endl;
            if (hasPos) {
                std::cout << "pos: (" << tl.p << ")" << std::endl;
            }
//            std::cout << "v: (" << tl.v.x() << ", " << tl.v.y() << std::endl;
        }
    }

    void polygon2Vectors(Polygon_2& polygon, std::vector<TraceLine>& vectors);

    void calculateTraceLines(Polygon_2& anglePolygon, Polygon_2& linesPolygon, std::vector<TraceLine>& trace_lines);

    void generateTraceLine(Polygon_2& plate, Polygon_2& part, Point_2& center, std::vector<Segment_2>& trace_lines);

    void processCollinear(std::vector<Segment_2>& trace_lines);

    void deleteOutTraceLine(std::vector<Segment_2>& trace_lines, Polygon_2& platePolygon, Point_2& center);

    void mergeTraceLines2Polygon(Polygon_2& plate, Point_2& center, std::vector<Segment_2>& trace_lines, std::vector<std::vector<Segment_2>>& nfp_rings);

    void traverTraceLines(std::vector<Segment_2>& trace_lines, std::vector<Segment_2>& new_trace_lines);

    int searchLowerStartPointIndex(std::vector<Segment_2>& nfpLines);

    Point_2 searchLowerPosition(std::vector<Segment_2>& nfp_lines);

    void getRotatePolygons(Polygon_with_holes_2& polygon, int i, Point_2& rotateCenter, Point_2& center);

    void generateNFP(Plate& plate, Part& part, Part& result_part);

    void updateCurrentPlate(Plate& plate, Polygon_with_holes_2& pwhs);

    bool partPlacement(Plate& plate, Part& part, Part& result_part);

    void startNFP();

    void modelNesting(std::string input_file, std::string output_file, DataGroup& data_group);

    bool readFile(std::string input_file);

    void readGroup(const rapidjson::Value& itemV);

    Part readPart(const rapidjson::Value& itemV);

    void readPlate(const rapidjson::Value& itemV);

    Polygon_with_holes_2 readPolygons(const rapidjson::Value& polysV);

    void checkParams(std::string type_name, const rapidjson::Value& item);

    bool writeFile(std::string output_file);

    void createJson(rapidjson::Document& doc);


};


}
#endif  // LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
