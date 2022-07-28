//
// Created by Cyril on 2022/7/26.
//

#ifndef LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
#define LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/offset_polygon_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Timer.h>

#include <CGAL/Boolean_set_operations_2.h>

#include "../../rapidjson/rapidjson/ostreamwrapper.h"
#include "../../rapidjson/rapidjson/document.h"
#include "../../rapidjson/rapidjson/writer.h"
#include "../utils/logoutput.h"
#include "../data/DataGroup.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <list>

typedef CGAL::Exact_predicates_inexact_constructions_kernel                 K;
typedef CGAL::Polygon_2<K>                                                  Polygon_2;
typedef CGAL::Polygon_with_holes_2<K>                                       Polygon_with_holes_2;
typedef K::Segment_2                                                        Segment_2;
typedef K::Point_2                                                          Point_2;
typedef K::Vector_2                                                         Vector_2;
typedef K::FT                                                               FT;


namespace lunarmp {

struct TraceLine {
    Vector_2 line;

};

class Plate {
  public:
    Polygon_2 polygon;
    double area;
    double absArea;
    int id;

    Plate() {};
    Plate(Polygon_2 poly) : polygon(poly){};

    void sort();
    void print() {
        std::cout << "plate:" << std::endl;
        for (Point_2 p : polygon) {
            std::cout << "(" << p.x() << ", " << p.y() << ")" << std::endl;
        }
        std::cout << "area: " << area << std::endl;
    }
};

class Part {
  public:
    Part() {};
    ~Part() {};

    Polygon_2 polygon;
    Polygon_2 rotate_polygon;
    Point_2 position;
    Point_2 center;
    double area;
    double absArea;
    int angle;
    bool in_place;
    bool is_rotated = false;
    int id;

    void sort();
    void print() {
        std::cout << "id: " << id << std::endl;
        std::cout << "polygon:" << std::endl;
        for (Point_2 p : polygon) {
            std::cout << "(" << p.x() << ", " << p.y() << ")" << std::endl;
        }
        std::cout << "rotate_polygon:" << std::endl;
        for (Point_2 p : rotate_polygon) {
            std::cout << "(" << p.x() << ", " << p.y() << ")" << std::endl;
        }
        std::cout << "position: (" << position.x() << ", " << position.y() << ")" << std::endl;
        std::cout << "center: (" << center.x() << ", " << center.y() << ")" << std::endl;
        std::cout << "angle: " << angle << std::endl;
        std::cout << "in_place: " << in_place << std::endl;
    }
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



    bool sortPolygon(Polygon_2 polygon, bool clock_wise);

    void sortParts(std::vector<Part>& parts);

    void sortPlates(std::vector<Plate>& plates);

    Polygon_2 roundAndMulPolygons(Polygon_2 polygon, double limit);

    Point_2 roundAndMulPoint(Point_2 point, double limit);

    void updatePolygonPosition(Polygon_2& polygon, Point_2 pos);

    std::list<Polygon_with_holes_2> differencePolygon(Polygon_2 sub, Polygon_2 clip);
    std::list<Polygon_with_holes_2> offsetPolygon(Polygon_with_holes_2& sub, double offset, std::string type);

    void polygon2Vector(Polygon_2 polygon, std::vector<Point_2>& vectors);

    void calculateTraceLine(std::vector<Polygon_2> anglePolygon, std::vector<Polygon_2> linesPolygon, std::vector<Segment_2>& trace_lines);

    void generateTraceLine(Polygon_2 plate, Polygon_2 part, Point_2 center, std::vector<TraceLine>& trace_lines);

    void processCollinear(std::vector<Segment_2>& trace_lines);

    void processCollinearOnlyHVLines(std::vector<Segment_2>& trace_lines);

    void deleteOutTraceLine(std::vector<Segment_2>& trace_lines, std::vector<Segment_2>& newTraceLines, Plate platePolygon, Point_2 center);

    void mergeTraceLines2Polygon(Polygon_2 plate, Point_2 center, std::vector<TraceLine>& trace_lines, std::vector<std::vector<TraceLine>>& nfp_rings);

    void deleteNoRingSegments(std::vector<Segment_2>& trace_lines);

    void setMinPoint(Point_2 lowerPoint, Point_2 point);

    int searchLowerStartPointIndex(std::vector<Segment_2>& nfpLines);

    Point_2 searchLowerPosition(std::vector<TraceLine> nfpLines);

    void standardizedPolygons(std::vector<Polygon_2>& rotatePolygons);

    void getRotatePolygons(Polygon_with_holes_2& polygon, int i, Point_2& center);

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
