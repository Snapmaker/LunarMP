//
// Created by Cyril on 2022/7/26.
//

#ifndef LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
#define LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Timer.h>

#include <CGAL/IO/polygon_soup_io.h>

#include <string>
#include <vector>

#include "../utils/logoutput.h"
#include "../data/DataGroup.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel                 K;
typedef K::Segment_2                                                        Segment_2;
typedef K::Line_2                                                           Line_2;
typedef K::Vector_2                                                         Vector_2;
typedef K::Point_2                                                          Point_2;
typedef CGAL::Polygon_2<K>                                                  Polygon_2;

namespace lunarmp {

class Plate {
  public:
    Polygon_2 polygon;
    double area;

    void sort();
};

class Part {
  public:
    Part() {};
    ~Part() {};

    Polygon_2 polygon;
    Polygon_2 rotate_polygon;
    Point_2 position;
    Point_2 center;
    int angle;
    bool isPlace;
    int model_id;

    void sort();
};

class ModelNesting {
  private:
    double accuracy = 10;
    double min_part_area = 0;

  public:
    std::vector<Part> parts;
    std::vector<Plate> plates;
    std::vector<Part> result_parts;
    int rotate = 360;
    int plate_offset = 10;
    int offset = 0;
    int interval = 2;


    bool sortPolygon(Polygon_2 polygon, bool clock_wise);

    void sortParts(std::vector<Part>& parts);

    void polygon2Vector(Polygon_2 polygon, std::vector<Vector_2>& vectors);

    void calculateTraceLine(std::vector<Polygon_2> anglePolygon, std::vector<Polygon_2> linesPolygon, std::vector<Segment_2>& trace_lines);

    void generateTraceLine(Plate& plate, Part& part, Point_2 center, std::vector<Segment_2>& trace_lines);

    void processCollinear(std::vector<Segment_2>& trace_lines);

    void processCollinearOnlyHVLines(std::vector<Segment_2>& trace_lines);

    void deleteOutTraceLine(std::vector<Segment_2>& trace_lines, std::vector<Segment_2>& newTraceLines, Plate platePolygon, Point_2 center);

    void mergeTraceLines2Polygon(std::vector<Segment_2>& trace_lines, Plate& plate, Point_2 center);

    void deleteNoRingSegments(std::vector<Segment_2>& trace_lines);

    void setMinPoint(Point_2 lowerPoint, Point_2 point);

    int searchLowerStartPointIndex(std::vector<Segment_2>& nfpLines);

    Point_2 searchLowerPosition(std::vector<Segment_2>& nfpLines);

    void standardizedPolygons(std::vector<Polygon_2>& rotatePolygons);

    void getRotatePolygons(std::vector<Polygon_2>&polygons, int i, Point_2 center);

    void generateNFP();

    void updateCurrentPlate(Plate currentPlate, std::vector<Polygon_2> diffPlatePolygons);

    void partPlacement(Plate plate, Part part);

    void startNFP();

    bool readFile();

    bool readFileList(std::string folder_path);

    void writeFile(std::string output_file);

    void modelNesting(std::string input_file, std::string output_file, DataGroup& data_group);
};


}
#endif  // LUNARMP_SRC_LUNARMP_MODEL_MODELNESTING_H_
