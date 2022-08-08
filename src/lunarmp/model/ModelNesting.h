//
// Created by Cyril on 2022/7/26.
//
#pragma once

#ifndef LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
#define LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_

#include "../../rapidjson/rapidjson/filewritestream.h"
#include "../../rapidjson/rapidjson/document.h"
#include "../../rapidjson/rapidjson/prettywriter.h"
//#include "../../rapidjson/rapidjson/writer.h"
#include "../utils/logoutput.h"
#include "../data/DataGroup.h"
#include "../polygon/PolygonBase.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <list>

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
    Plate(Polygon_2 poly) : polygon(poly){};

    void init() {
        area = polygon.outer_boundary().area();
        abs_area = std::fabs(area);

//        std::cout << "area: " << area << std::endl;
//        std::cout << "abs area: " << abs_area << std::endl;
    }
    void printPlate();
};

class Part {
  public:
    Part() {};
    Part(Point_2 p, Point_2 c, Polygon_with_holes_2 poly) : position(p), center(c), rotate_polygon(poly) {};

    ~Part() {};

    Polygon_with_holes_2 polygon;
    Polygon_with_holes_2 rotate_polygon;
    Point_2 position = Point_2(-1, -1);
    Point_2 center = Point_2(-1, -1);
    double area = 0;
    double abs_area = 0;
    bool in_place = false;
    int id = -1;
    bool is_group = false;

    void init() {
        area = polygon.outer_boundary().area();
        abs_area = std::fabs(area);
//        std::cout << "area: " << area << std::endl;
//        std::cout << "abs area: " << abs_area << std::endl;
    }
    void printPart();
    void updateArea() {
        area = rotate_polygon.outer_boundary().area();
        abs_area = std::fabs(area);
    }
};

class PartGroup {
  public:
    PartGroup() {};
    PartGroup(std::vector<Part> models) : models(models) {};
    ~PartGroup() {};

    int id_group = -1;
    std::vector<Part> models;
};

class ModelNesting {
  private:
    double accuracy = 1;
    double min_part_area = 0;
    int plate_offset = 10;

  public:
    std::vector<Part> parts;
    std::vector<PartGroup> part_groups;
    std::vector<Plate> plates;
    std::vector<Part> result_parts;
    std::vector<PartGroup> result;
    int rotate = 360;
    int offset = 0;
    int limit_edge = 2;
    bool is_rotation = false;

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

//    bool sortPolygon(Polygon_2& polygon, bool clock_wise);

//    void updatePolygonPosition(Polygon_with_holes_2& polygon, Point_2& pos);

    void polygon2Vectors(Polygon_2& polygon, std::vector<TraceLine>& vectors);

    void calculateTraceLines(Polygon_2& anglePolygon, Polygon_2& linesPolygon, std::vector<TraceLine>& trace_lines);

    void generateTraceLine(Polygon_2& plate, Polygon_2& part, Point_2& center, std::vector<Segment_2>& trace_lines);

    void processCollinear(std::vector<Segment_2>& trace_lines);

    void deleteOutTraceLine(std::vector<Segment_2>& trace_lines, Polygon_2& platePolygon, Point_2& center);

    void mergeTraceLines2Polygon(Polygon_2& plate, Point_2& center, std::vector<Segment_2>& trace_lines, std::vector<std::vector<Segment_2>>& nfp_rings);

    void traverTraceLines(std::vector<Segment_2>& trace_lines, std::vector<Segment_2>& new_trace_lines);

//    void deleteNoRingSegments(std::vector<Segment_2>& trace_lines);

//    void setMinPoint(Point_2& lowerPoint, Point_2& point);

    int searchLowerStartPointIndex(std::vector<Segment_2>& nfpLines);

    Point_2 searchLowerPosition(std::vector<Segment_2>& nfp_lines);

//    void standardizedPolygons(std::vector<Polygon_2>& rotate_polygons);

    void getRotatePolygons(Polygon_with_holes_2& polygon, int i, Point_2& rotateCenter, Point_2& center);

    void generateNFP(Plate& plate, Part& part, Part& result_part);

//    Polygon_2 updateCurrentPolygon(Plate& plate, Polygon_2& poly, Polygon_with_holes_2& pwhs);

    void updateCurrentPlate(Plate& plate, Polygon_with_holes_2& pwhs);

    bool partPlacement(Plate& plate, Part& part, Part& result_part);

    void startNFP();

    bool readFile(std::string input_file);

    bool writeFile(std::string output_file);

    void createJson(rapidjson::Document& doc);

    void modelNesting(std::string input_file, std::string output_file, DataGroup& data_group);
};


}
#endif  // LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
