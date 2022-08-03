//
// Created by Cyril on 2022/7/26.
//

#ifndef LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
#define LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_

#include <CGAL/Boolean_set_operations_2.h>

//#include "../../rapidjson/rapidjson/ostreamwrapper.h"
#include "../../rapidjson/rapidjson/document.h"
#include "../../rapidjson/rapidjson/writer.h"
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

    void sort();
};

class Part {
  public:
    Part() {};
    Part(Point_2 p, int angle, Point_2 c, Polygon_with_holes_2 poly) : position(p), angle_step(angle), center(c), rotate_polygon(poly) {};

    ~Part() {};

    Polygon_with_holes_2 polygon;
    Polygon_with_holes_2 rotate_polygon;
    Point_2 position;
    Point_2 center;
    double area;
    double abs_area;
    int angle_step;
    bool in_place;
    bool is_rotated = false;
    int id;

    void sort();
};

class ModelNesting {
  private:
    double accuracy = 10;
    double min_part_area = 0;
    int plate_offset = 10;

  public:
    std::vector<Part> parts;
    std::vector<Plate> plates;
    std::vector<Part> result_parts;
    int rotate = 360;
    int offset = 0;
    int interval = 2;

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

    bool sortPolygon(Polygon_2& polygon, bool clock_wise);

    void sortParts(std::vector<Part>& parts);

    void sortPlates(std::vector<Plate>& plates);

    void updatePolygonPosition(Polygon_with_holes_2& polygon, Point_2& pos);

    void polygon2Vectors(Polygon_2& polygon, std::vector<TraceLine>& vectors);

    void calculateTraceLines(Polygon_2& anglePolygon, Polygon_2& linesPolygon, std::vector<TraceLine>& trace_lines);

    void generateTraceLine(Polygon_2& plate, Polygon_2& part, Point_2& center, std::vector<Segment_2>& trace_lines);

    void processCollinear(std::vector<Segment_2>& trace_lines);

    void deleteOutTraceLine(std::vector<Segment_2>& trace_lines, Polygon_2& platePolygon, Point_2& center);

    void mergeTraceLines2Polygon(Polygon_2& plate, Point_2& center, std::vector<Segment_2>& trace_lines, std::vector<std::vector<Segment_2>>& nfp_rings);

    void traverTraceLines(std::vector<Segment_2>& trace_lines, std::vector<Segment_2>& new_trace_lines);

    void deleteNoRingSegments(std::vector<Segment_2>& trace_lines);

    void setMinPoint(Point_2& lowerPoint, Point_2& point);

    int searchLowerStartPointIndex(std::vector<Segment_2>& nfpLines);

    Point_2 searchLowerPosition(std::vector<Segment_2>& nfp_lines);

    void standardizedPolygons(std::vector<Polygon_2>& rotate_polygons);

    void getRotatePolygons(Polygon_with_holes_2& polygon, int i, Point_2& rotateCenter, Point_2& center);

    void generateNFP(Plate& plate, Part& part, Part& result_part);

    void updateCurrentPlate(Plate& plate, Polygon_with_holes_2& pwhs);

    bool partPlacement(Plate& plate, Part& part, Part& result_part);

    void startNFP();

    bool readFile(std::string input_file);

    void writeFile(std::string output_file);

    void modelNesting(std::string input_file, std::string output_file, DataGroup& data_group);
};


}
#endif  // LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
