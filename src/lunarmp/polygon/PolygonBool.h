//
// Created by Cyril on 2022/8/3.
//
#pragma once

#ifndef LUNARMP_SRC_LUNARMP_POLYGON_POLYGONBOOL_H_
#define LUNARMP_SRC_LUNARMP_POLYGON_POLYGONBOOL_H_

#include <CGAL/Boolean_set_operations_2.h>

#include "PolygonBase.h"

#include "../clipper/clipper.hpp"

//typedef boost::shared_ptr<Polygon_2>                                         P_ptr;
typedef boost::shared_ptr<Polygon_with_holes_2>                             Pwh_ptr;

namespace lunarmp {

std::vector<Polygon_with_holes_2> polygonDifference(Polygon_2 sub, Polygon_2& clip) {
    Polygon_2 sub_tmp = sub;
    sub_tmp.reverse_orientation();
    std::list<Polygon_with_holes_2> pwhs;
    CGAL::difference(sub_tmp, clip, std::back_inserter(pwhs), CGAL::Tag_true());
    std::vector<Polygon_with_holes_2> res{
        std::make_move_iterator(std::begin(pwhs)),
        std::make_move_iterator(std::end(pwhs))
    };
    return res;
}


void printPolygon1(const Polygon_2& P)
{
    std::cout << "[ " << P.size() << " vertices:";
    for (Polygon_2::Vertex_const_iterator vit = P.vertices_begin(); vit != P.vertices_end(); ++vit) {
        std::cout << " (" << (*vit).x() << "," << (*vit).y() << ')';
    }
    std::cout << " ]" << std::endl;
    return;
}

void printPolygonWithHoles1(const Polygon_with_holes_2& pwh) {
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

void polygonToPoints(Polygon_2& polygon, ClipperLib::Path & path) {
    for (VertexIterator vi = polygon.vertices_begin(); vi != polygon.vertices_end(); ++vi) {
//        path.emplace_back(round((*vi).x()), round((*vi).y()));
        path.emplace_back(round((*vi).x()*1e6), round((*vi).y()*1e6));
    }
    path.emplace_back(path[0]);
}

void pwhToPoints(Polygon_with_holes_2& pwh, ClipperLib::Paths& paths) {
    if (pwh.is_unbounded()) {
        log("pwhToPoints: Unbounded pwh\n");
        return;
    }

    ClipperLib::Path poly;
    polygonToPoints(pwh.outer_boundary(), poly);
    paths.emplace_back(poly);
    if (pwh.number_of_holes() > 0) {
        for (Polygon_2 hole : pwh.holes()) {
            ClipperLib::Path poly_h;
            polygonToPoints(hole, poly_h);
            paths.emplace_back(poly_h);
        }
    }
//
//    for(int i = 0; i < paths.size(); i++) {
//        ClipperLib::Path poly1 = paths[i];
//        for (int j = 0; j < poly1.size(); j++) {
//            std::cout << poly1[j].X << " " << poly1[j].Y << std::endl;
//        }
//    }
}

Polygon_with_holes_2 pointTpPwh(ClipperLib::Paths paths) {
    Polygon_2 outer;
    for (int i = 0; i < paths[0].size(); i++) {
//        outer.push_back(Point_2(paths[0][i].X, paths[0][i].Y));
        outer.push_back(Point_2(paths[0][i].X*1e-6, paths[0][i].Y*1e-6));
    }

    Polygon_with_holes_2 res(outer);
    if (paths.size() > 1) {
        for (int i = 1; i < paths.size(); i++) {
            ClipperLib::Path path = paths[i];
            Polygon_2 hole;
            for (int j = 0; j < path.size(); j++) {
                hole.push_back(Point_2(path[j].X*1e-6, path[j].Y*1e-6));
            }
            res.add_hole(hole);
        }
    }
//    coutPwh(res);
    return res;
}

Polygon_with_holes_2 polygonOffset(Polygon_with_holes_2& pwh, int distance, ClipperLib::JoinType join_type = ClipperLib::jtMiter, double miter_limit = 3) {
    ClipperLib::Paths paths;
    pwhToPoints(pwh, paths);

    if (distance == 0) {
        return pwh;
    }

    ClipperLib::Paths ret;
    ClipperLib::ClipperOffset clipper(miter_limit*1e6, 0.25*1e6);
    clipper.AddPaths(paths, join_type,
                     ClipperLib::etClosedPolygon);
    clipper.MiterLimit = miter_limit;
    clipper.Execute(ret, distance*1e6);

    return pointTpPwh(ret);
}

Polygon_with_holes_2 polygonsIntersection(Polygon_with_holes_2& sub ,Polygon_2& clip) {
    if (sub.is_unbounded() || clip.is_empty()) {
        return Polygon_with_holes_2();
    }
    ClipperLib::Paths ret;
    ClipperLib::Paths sub_paths;
    pwhToPoints(sub, sub_paths);

    ClipperLib::Path clip_path;
    polygonToPoints(clip, clip_path);

    ClipperLib::Clipper clipper(0);
    clipper.AddPaths(sub_paths, ClipperLib::ptSubject, true);
    clipper.AddPath(clip_path, ClipperLib::ptClip, true);
    clipper.Execute(ClipperLib::ctIntersection, ret);

    return pointTpPwh(ret);
}

}


#endif  // LUNARMP_SRC_LUNARMP_POLYGON_POLYGONBOOL_H_
