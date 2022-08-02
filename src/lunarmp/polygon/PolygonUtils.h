//
// Created by Cyril on 2022/7/29.
//

#ifndef LUNARMP_SRC_LUNARMP_UTILS_POLYGONUTILS_H_
#define LUNARMP_SRC_LUNARMP_UTILS_POLYGONUTILS_H_


#include "PolygonBase.h"

#define EPSILON 1e-6

namespace lunarmp {

std::vector<Point_2> getVertices(Polygon_2 p) {
    std::vector<Point_2> res;
    Polygon_2::Vertex_const_iterator vit;
    for (vit = p.vertices_begin(); vit != p.vertices_end(); ++vit) {
        res.emplace_back(*vit);
    }
    return res;
}

void printPoints(std::vector<Point_2> P) {
    std::cout << "[ " << P.size() << " vertices:";
    for (Point_2 v : P) {
        std::cout << " (" << v.x() << ", " << v.y() << ')';
    }
    std::cout << " ]" << std::endl;
}

void printPolygon(const Polygon_2& P)
{
    Polygon_2::Vertex_const_iterator vit;

    std::cout << "[ " << P.size() << " vertices:";
    for (vit = P.vertices_begin(); vit != P.vertices_end(); ++vit) {
        std::cout << " (" << *vit << ')';
    }
    std::cout << " ]" << std::endl;
    return;
}

void printPolygonWithHoles(const Polygon_with_holes_2& pwh) {
    if (! pwh.is_unbounded()) {
        std::cout << "{ Outer boundary = ";
        printPolygon (pwh.outer_boundary());
    }
    else {
        std::cout << "{ Unbounded polygon." << std::endl;
    }

    Polygon_with_holes_2::Hole_const_iterator  hit;
    unsigned int k = 1;

    std::cout << "  " << pwh.number_of_holes() << " holes:" << std::endl;
    for (hit = pwh.holes_begin(); hit != pwh.holes_end(); ++hit, ++k) {
        std::cout << "    Hole #" << k << " = ";
        printPolygon (*hit);
    }
    std::cout << " }" << std::endl;
    return;
}

void printPolygons(std::list<Polygon_with_holes_2>& pwhs) {
    std::list<Polygon_with_holes_2>::const_iterator it;
    int i = 0;
    for (it = pwhs.begin(); it != pwhs.end(); ++it) {
        std::cout << "i: " << i++ << std::endl;
        Polygon_with_holes_2 pwh = *it;
        printPolygonWithHoles(pwh);
    }
}

double approximate(double a, int bit = 6) {
    if (bit == 0) {
        return round(a);
    }

    return round(a * 1e6) * 1e-6;
}

bool getDirection(Point_2 a, Point_2 b) {
    if (approximate(a.x() - b.x()) == 0) {
        return approximate(a.y() - b.y()) < 0;
    }
    else {
        return approximate(a.x() - b.x()) < 0;
    }
}

bool isParallel(Segment_2 a, Segment_2 b) {
    return a.is_degenerate() || b.is_degenerate() ||
           a.direction() == b.direction() ||
           a.direction() == -b.direction();
}

Point_2 roundAndMulPoint(Point_2 point, double limit = 1) {
    return Point_2(approximate(point.x() * limit), approximate(point.y() * limit));
}

Polygon_2 roundAndMulPolygon(Polygon_2 polygon, double limit = 1) {
    Polygon_2 res;
    for (VertexIterator vi = polygon.vertices_begin(); vi != polygon.vertices_end(); ++vi) {
        res.push_back(roundAndMulPoint(*vi, limit));
    }
    return res;
}

void roundAndMulPolygons(Polygon_with_holes_2& polygon, double limit = 1) {
    Polygon_2 outer;
    for (Point_2 point : polygon.outer_boundary()) {
        outer.push_back(roundAndMulPoint(point, limit));
    }
    if (polygon.number_of_holes() > 0) {
        std::vector<Polygon_2> holes;
        for (Polygon_2 hole : polygon.holes()) {
            holes.emplace_back(roundAndMulPolygon(hole, limit));
        }
        polygon = Polygon_with_holes_2(outer, holes.begin(), holes.end());
    }
    else {
        polygon = Polygon_with_holes_2(outer);
    }
}

double angleToPi(double angle) {
    return approximate(angle / 180 * CGAL_PI);
}

double piToAngle(double pi) {
    return approximate(pi / CGAL_PI * 180);
}

int angle(Vector_2 v) {
    double angle = piToAngle(std::atan2(v.y(), v.x()));
    if (angle <= 0) {
        angle += 360;
    }
    return (int)angle;
}

Point_2 rotate(Point_2 p, int angle, Point_2 center = Point_2(0.0, 0.0)) {
    double pi = angleToPi((double)angle);
    FT x = (p.x() - center.x()) * cos(pi) - (p.y() - center.y()) * sin(pi) + center.x();
    FT y = (p.x() - center.x()) * sin(pi) + (p.y() - center.y()) * cos(pi) + center.y();
    return Point_2(approximate(x), approximate(y));
}

Polygon_2 rotatePolygon(Polygon_2 polygon, int i, Point_2 center = Point_2(0.0, 0.0)) {
    Polygon_2 res;
    for (VertexIterator vi = polygon.vertices_begin(); vi != polygon.vertices_end(); ++vi) {
        res.push_back(rotate(*vi, i, center));
    }
    return res;
}

void rotatePolygons(Polygon_with_holes_2& polygon, int i, Point_2 center) {
    Polygon_2 outer = rotatePolygon(polygon.outer_boundary(), i, center);

    if (polygon.number_of_holes() > 0) {
        std::vector<Polygon_2> inner(polygon.number_of_holes());
        Polygon_with_holes_2::Hole_const_iterator hit;
        for (hit = polygon.holes_begin(); hit != polygon.holes_end(); ++hit) {
            inner.push_back(rotatePolygon(*hit, i, center));
        }
        polygon = Polygon_with_holes_2(outer, inner.begin(), inner.end());
    }
    else {
        polygon = Polygon_with_holes_2(outer);
    }
}

Point_2 add(Point_2 a, Point_2 b) {
    return Point_2(approximate(a.x() + b.x()), approximate(a.y() + b.y()));
}

Point_2 sub(Point_2 a, Point_2 b) {
    return Point_2(approximate(a.x() - b.x()), approximate(a.y() - b.y()));
}

Polygon_2 movePolygon(Polygon_2 polygon, Point_2 offset) {
    Polygon_2 res;
    for (VertexIterator vi = polygon.vertices_begin(); vi != polygon.vertices_end(); ++vi) {
        res.push_back(add(*vi, offset));
    }
    return res;
}

void movePolygons(Polygon_with_holes_2& polygon, Point_2 offset) {
    Polygon_2 outer = movePolygon(polygon.outer_boundary(), offset);

    if (polygon.number_of_holes() > 0) {
        std::vector<Polygon_2> inner(polygon.number_of_holes());
        Polygon_with_holes_2::Hole_const_iterator hit;
        for (hit = polygon.holes_begin(); hit != polygon.holes_end(); ++hit) {
            inner.push_back(movePolygon(*hit, offset));
        }
        polygon = Polygon_with_holes_2(outer, inner.begin(), inner.end());
    }
    else {
        polygon = Polygon_with_holes_2(outer);
    }
}

std::list<Polygon_with_holes_2> differencePolygon(Polygon_with_holes_2 sub, Polygon_with_holes_2 clip) {
    std::list<Polygon_with_holes_2> pwhs;
    CGAL::difference(sub, clip, std::back_inserter(pwhs), CGAL::Tag_true());


    return pwhs;
}

//std::list<Polygon_with_holes_2> offsetPolygon(Polygon_with_holes_2& sub, double offset, std::string type) {
//
//}


}

#endif  // LUNARMP_SRC_LUNARMP_UTILS_POLYGONUTILS_H_
