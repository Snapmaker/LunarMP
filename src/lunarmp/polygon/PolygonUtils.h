//
// Created by Cyril on 2022/7/29.
//
#pragma once

#ifndef LUNARMP_SRC_LUNARMP_UTILS_POLYGONUTILS_H_
#define LUNARMP_SRC_LUNARMP_UTILS_POLYGONUTILS_H_

#include "PolygonBase.h"

namespace lunarmp {

std::vector<Point_2> polygonToVertices(Polygon_2& p) {
    std::vector<Point_2> res;
    for (Polygon_2::Vertex_const_iterator vit = p.vertices_begin(); vit != p.vertices_end(); ++vit) {
        res.emplace_back(*vit);
    }
    return res;
}

std::vector<Point_2> pwhToVertices(Polygon_with_holes_2& pwh) {
    if (pwh.is_unbounded()) {
        return std::vector<Point_2>();
    }
    std::vector<Point_2> res = polygonToVertices(pwh.outer_boundary());
    std::vector<Point_2> tmp;
    if (pwh.number_of_holes() > 0) {
        for (Polygon_2 p : pwh.holes()) {
            tmp = polygonToVertices(p);
            res.insert(res.end(), tmp.begin(), tmp.end());
            tmp.clear();
        }
    }
    return res;
}

Polygon_with_holes_2 verticesToPwh(std::vector<Point_2>& points) {
    Polygon_2 outer;
    for (Point_2 point : points) {
        outer.push_back(point);
    }
    return Polygon_with_holes_2(outer);
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
    std::cout << "[ " << P.size() << " vertices:";
    for (Polygon_2::Vertex_const_iterator vit = P.vertices_begin(); vit != P.vertices_end(); ++vit) {
        std::cout << " (" << (*vit).x() << "," << (*vit).y() << ')';
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

    unsigned int k = 1;

    std::cout << "  " << pwh.number_of_holes() << " holes:" << std::endl;
    for (Polygon_2 hit : pwh.holes()) {
        std::cout << "    Hole #" << k++ << " = ";
        printPolygon (hit);
    }
    std::cout << " }" << std::endl;
    return;
}

void printPolygons(std::vector<Polygon_with_holes_2>& pwhs) {
    int i = 0;
    for (Polygon_with_holes_2 pwh : pwhs) {
        std::cout << "i: " << i++ << std::endl;
        printPolygonWithHoles(pwh);
    }
}

double approximate(double a, int bit = 0) {
    if (bit == 0) {
        return round(a);
    }
    if (bit == 2) {
        return round(a * 1e2) * 1e-2;
    }
    if (bit == 4) {
        return round(a * 1e4) * 1e-4;
    }
    if (bit == 6) {
        return round(a * 1e6) * 1e-6;
    }
    return a;
}

bool isEqual(double a, double b) {
    if (approximate(a - b, 6) == 0) {
        return true;
    }
    return false;
}

bool isEqualPoint(Point_2 a, Point_2 b) {
    return isEqual(a.x(), b.x()) && isEqual(a.y(), b.y());
}

bool getDirection(Point_2 a, Point_2 b) {
    if (isEqual(a.x(), b.x())) {
        return approximate(a.y() - b.y(), 6) < 0;
    }
    else {
        return approximate(a.x() - b.x(), 6) < 0;
    }
}

Point_2 getCenter(Polygon_with_holes_2& pwh) {
    double x = 0.0;
    double y = 0.0;
    std::vector<Point_2> tmp = polygonToVertices(pwh.outer_boundary());
    for (Point_2 p : tmp) {
        x += p.x();
        y += p.y();
    }
    int L = tmp.size();
    return Point_2(x/L, y/L);
}

Point_2 roundAndMulPoint(Point_2& point, double limit = 1) {
    return Point_2(approximate(point.x() * limit), approximate(point.y() * limit));
}

Polygon_2 roundAndMulPolygon(Polygon_2& polygon, double limit = 1) {
    std::vector<Point_2> tmp = polygonToVertices(polygon);
    Polygon_2 res;
    for (Point_2& p : tmp) {
        res.push_back(roundAndMulPoint(p, limit));
    }
    return res;
}

Polygon_with_holes_2 roundAndMulPolygons(Polygon_with_holes_2& polygon, double limit = 1) {
    Polygon_with_holes_2 res(roundAndMulPolygon(polygon.outer_boundary()));
    if (polygon.number_of_holes() > 0) {
        std::vector<Polygon_2> holes;
        for (Polygon_2& hole : polygon.holes()) {
//            holes.emplace_back(roundAndMulPolygon(hole, limit));
            res.add_hole(roundAndMulPolygon(hole));
        }
    }
    return res;
}

double angleToPi(double angle) {
    return approximate(angle / 180 * CGAL_PI, 4);
}

double piToAngle(double pi) {
    return approximate(pi / CGAL_PI * 180, 4);
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
    double x = (p.x() - center.x()) * cos(pi) - (p.y() - center.y()) * sin(pi) + center.x();
    double y = (p.x() - center.x()) * sin(pi) + (p.y() - center.y()) * cos(pi) + center.y();

    return Point_2(approximate(x, 6), approximate(y, 6));
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
        std::vector<Polygon_2> inner;
        for (Polygon_2 hole : polygon.holes()) {
            inner.push_back(rotatePolygon(hole, i, center));
        }
        polygon = Polygon_with_holes_2(outer, inner.begin(), inner.end());
    }
    else {
        polygon = Polygon_with_holes_2(outer);
    }
}

Point_2 getBBoxMinn(Polygon_2 polygon) {
    std::vector<Point_2> tmp = polygonToVertices(polygon);
    double x = polygon[0].x();
    double y = polygon[0].y();
    for (int i = 1; i < polygon.size(); i++) {
        x = std::min(x, polygon[i].x());
        y = std::min(y, polygon[i].y());
    }
    return Point_2(x, y);
}

Point_2 add(Point_2 a, Point_2 b) {
    return Point_2(approximate(a.x() + b.x()), approximate(a.y() + b.y()));
}

Point_2 sub(Point_2 a, Point_2 b) {
    return Point_2(approximate(a.x() - b.x()), approximate(a.y() - b.y()));
}

Point_2 findLowerPointInPolygon(Polygon_with_holes_2 polygon) {
    std::vector<Point_2> tmp = polygonToVertices(polygon.outer_boundary());
    double x = tmp[0].x();
    double y = tmp[0].y();
    for (Point_2 p : tmp) {
        x = std::min(x, p.x());
        y = std::min(y, p.y());
    }
    return Point_2(x, y);
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
        std::vector<Polygon_2> inner;
        for (Polygon_2 hole : polygon.holes()) {
            inner.push_back(movePolygon(hole, offset));
        }
        polygon = Polygon_with_holes_2(outer, inner.begin(), inner.end());
    }
    else {
        polygon = Polygon_with_holes_2(outer);
    }
}

std::vector<Point_2> compressLine(std::vector<Point_2>& polygon, int start, int end, double limit) {
    std::vector<Point_2> res;
    if (start < end) {
        double max_dist = 0;
        double idx = 0;
        Point_2 start_point = polygon[start];
        Point_2 end_point = polygon[end];
        Segment_2 s(start_point, end_point);
        for (int i = start+1; i < end; i++) {
            double current_dist = std::sqrt(CGAL::squared_distance(polygon[i], s));
            if (current_dist > max_dist) {
                max_dist = current_dist;
                idx = i;
            }
        }
        if (max_dist >= limit) {
            std::vector<Point_2> res1 = compressLine(polygon, start, idx, limit);
            std::vector<Point_2> res2 = compressLine(polygon, idx, end, limit);
            for (int i = 0; i < res1.size() - 1; i++) {
                res.emplace_back(res1[i]);
            }
            for (int i = 0; i < res2.size(); i++) {
                res.emplace_back(res2[i]);
            }
        }
        else {
            res.emplace_back(polygon[start]);
            res.emplace_back(polygon[end]);
        }
    }
    return res;
}

void simplifyPolygon(Polygon_2& poly, int limit_edge) {
    std::vector<Point_2> tmp = polygonToVertices(poly);
    std::vector<Point_2> points = compressLine(tmp, 0, poly.size()-1, limit_edge);
    poly = Polygon_2(points.begin(), points.end());
}

void simplifyPolygons(Polygon_with_holes_2& pwh, int limit_edge) {
    if (pwh.is_unbounded()) {
        return;
    }
    simplifyPolygon(pwh.outer_boundary(), limit_edge);
    if (pwh.number_of_holes() > 0) {
        for (Polygon_2& hole : pwh.holes()) {
            simplifyPolygon(hole, limit_edge);
        }
    }
}

double getPwhArea(Polygon_with_holes_2 pwh) {
    double area = 0.0;
    area += pwh.outer_boundary().area();

    if (pwh.number_of_holes() > 0) {
        for (Polygon_2 hole : pwh.holes()) {
            area -= hole.area();
        }
    }
    return area;
}

void reversePolygon(Polygon_with_holes_2& pwh) {
    pwh.outer_boundary().reverse_orientation();
    for (Polygon_2& hole : pwh.holes()) {
        hole.reverse_orientation();
    }
}

void coutPoints(std::vector<double>& points) {
    for (int i = 0; i < points.size(); i++) {
        if (i == points.size() - 1) {
            std::cout << points[i];
        }
        else {
            std::cout << points[i] << ",";
        }
    }
}

void coutLines(std::vector<Segment_2> tLines) {
    std::vector<double> st_x;
    std::vector<double> st_y;
    std::vector<double> ed_x;
    std::vector<double> ed_y;
    for (Segment_2 tl : tLines) {
        Point_2 st = tl.source();
        Point_2 ed = tl.target();
        st_x.emplace_back(approximate(st.x()));
        st_y.emplace_back(approximate(st.y()));
        ed_x.emplace_back(approximate(ed.x()));
        ed_y.emplace_back(approximate(ed.y()));
    }
    std::cout <<"len: " << tLines.size() << std::endl;
    std::cout << "ST_X = ["; coutPoints(st_x); std::cout << "]\n";
    std::cout << "ST_Y = ["; coutPoints(st_y); std::cout << "]\n";
    std::cout << "ED_X = ["; coutPoints(ed_x); std::cout << "]\n";
    std::cout << "ED_Y = ["; coutPoints(ed_y); std::cout << "]\n";
}

void coutPoly(Polygon_2 p) {
    std::cout << "[";
    std::vector<Point_2> tmp = polygonToVertices(p);
    for (int i = 0 ; i < tmp.size(); i++) {
        std::cout << tmp[i].x() << "," << tmp[i].y() << ",";
    }
    std::cout << tmp[0].x() << "," << tmp[0].y() << "]";
}

void coutPwh(Polygon_with_holes_2& pwh) {
    std::cout << "[";
    if (! pwh.is_unbounded()) {
        coutPoly(pwh.outer_boundary());
    }
    else {
        std::cout << "{ Unbounded polygon." << std::endl;
    }
    for (Polygon_2 hit : pwh.holes()) {
        std::cout << ",";
        coutPoly(hit);
    }
    std::cout << "]" << std::endl;
    return;
}

void coutPoly1(Polygon_2 p) {
    std::cout << "[";
    std::vector<Point_2> tmp = polygonToVertices(p);
    for (int i = 0 ; i < tmp.size(); i++) {
        std::cout << "{x:" << tmp[i].x() << ",y:" << tmp[i].y() << "},";
    }
    std::cout<< "{x:"  << tmp[0].x() << ",y:" << tmp[0].y() << "}]";
}

void coutPwh1(Polygon_with_holes_2& pwh) {
    std::cout << "[";
    if (! pwh.is_unbounded()) {
        coutPoly1(pwh.outer_boundary());
    }
    else {
        std::cout << "{ Unbounded polygon." << std::endl;
    }
    for (Polygon_2 hit : pwh.holes()) {
        std::cout << ",";
        coutPoly1(hit);
    }
    std::cout << "]" << std::endl;
    return;
}

}

#endif  // LUNARMP_SRC_LUNARMP_UTILS_POLYGONUTILS_H_
