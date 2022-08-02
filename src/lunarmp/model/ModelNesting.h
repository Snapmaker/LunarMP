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
    TraceLine(Vector_2 vec, Point_2 pos) : v(vec), p(pos) {};
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
    ~Part() {};

    Polygon_with_holes_2 polygon;
    Polygon_with_holes_2 rotate_polygon;
    Point_2 position;
    Point_2 center;
    double area;
    double abs_area;
    int angle;
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
            std::cout << "st: (" << tl.l.source().x() << ", " << tl.l.source().y() << ")\tend: (" << tl.l.target().x() << "," << tl.l.target().y() << ")" << std::endl;
            if (hasPos) {
                std::cout << "pos: (" << tl.p.x() << ", " << tl.p.y() << std::endl;
            }
//            std::cout << "v: (" << tl.v.x() << ", " << tl.v.y() << std::endl;
        }
    }

    bool sortPolygon(Polygon_2 polygon, bool clock_wise);

    void sortParts(std::vector<Part>& parts);

    void sortPlates(std::vector<Plate>& plates);

    void updatePolygonPosition(Polygon_with_holes_2& polygon, Point_2 pos);

    void polygon2Vectors(Polygon_2 polygon, std::vector<TraceLine>& vectors);

    void calculateTraceLines(Polygon_2 anglePolygon, Polygon_2 linesPolygon, std::vector<TraceLine>& trace_lines);

    void generateTraceLine(Polygon_2 plate, Polygon_2 part, Point_2 center, std::vector<TraceLine>& trace_lines);

    void processCollinear(std::vector<TraceLine>& trace_lines);

    void deleteOutTraceLine(std::vector<TraceLine>& trace_lines, Plate platePolygon, Point_2 center);

    void mergeTraceLines2Polygon(Polygon_2 plate, Point_2 center, std::vector<TraceLine>& trace_lines, std::vector<std::vector<TraceLine>>& nfp_rings);

    void traverTraceLines(std::vector<TraceLine> trace_lines, std::vector<TraceLine>& new_trace_lines);

    void deleteNoRingSegments(std::vector<Segment_2>& trace_lines);

    void setMinPoint(Point_2 lowerPoint, Point_2 point);

    int searchLowerStartPointIndex(std::vector<Segment_2>& nfpLines);

    Point_2 searchLowerPosition(std::vector<Segment_2> nfpLines);

    void standardizedPolygons(std::vector<Polygon_2>& rotatePolygons);

    void getRotatePolygons(Polygon_with_holes_2& polygon, int i, Point_2& rotateCenter, Point_2 center);

    bool generateNFP(Plate plate, Part part, Part& result_part);

    void updateCurrentPlate(Plate plate, std::list<Polygon_with_holes_2> pwhs);

    bool partPlacement(Plate plate, Part part, Part& result_part);

    void startNFP();

    bool readFile(std::string input_file);

    void writeFile(std::string output_file);

    void modelNesting(std::string input_file, std::string output_file, DataGroup& data_group);
};


}
#endif  // LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
