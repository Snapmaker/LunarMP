//
// Created by Cyril on 2022/8/3.
//
#pragma once

#ifndef LUNARMP_SRC_LUNARMP_POLYGON_POLYGONBOOL_H_
#define LUNARMP_SRC_LUNARMP_POLYGON_POLYGONBOOL_H_

#include <CGAL/Boolean_set_operations_2.h>
#include "PolygonBase.h"
#include "../clipper/clipper.hpp"


typedef boost::shared_ptr<Polygon_with_holes_2>                             Pwh_ptr;

namespace lunarmp {

void coutPath(ClipperLib::Path& path) {
    std::cout << "[";
    for (int i = 0; i < path.size(); i++) {
        if (i == 0) {
            std::cout << path[i].X << "," << path[i].Y;
        }
        else {
            std::cout << "," << path[i].X << "," << path[i].Y;
        }
    }
    std::cout << "],";

}

void coutPaths(ClipperLib::Paths& paths) {
    if (paths.size() == 0) {
        std::cout << "None paths";
        return ;
    }

    std::cout << "[";
    for (ClipperLib::Path& path : paths) {
        coutPath(path);
    }
    std::cout << "]\n";
}

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

void polygonToPoints(Polygon_2& polygon, ClipperLib::Path& path) {
    for (VertexIterator vi = polygon.vertices_begin(); vi != polygon.vertices_end(); ++vi) {
        path.emplace_back(round((*vi).x()), round((*vi).y()));
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
}

Polygon_2 pointsToPolygon(ClipperLib::Path& path) {
    Polygon_2 poly;
    for (int i = 0; i < path.size(); i++) {
        poly.push_back(Point_2(path[i].X, path[i].Y));
    }
    return poly;
}

Polygon_with_holes_2 pointTpPwh(ClipperLib::Paths paths) {
    Polygon_2 outer;
    for (int i = 0; i < paths[0].size(); i++) {
        outer.push_back(Point_2(paths[0][i].X, paths[0][i].Y));
    }

    Polygon_with_holes_2 res(outer);
    if (paths.size() > 1) {
        for (int i = 1; i < paths.size(); i++) {
            ClipperLib::Path path = paths[i];
            Polygon_2 hole;
            for (int j = 0; j < path.size(); j++) {
                hole.push_back(Point_2(path[j].X, path[j].Y));
            }
            res.add_hole(hole);
        }
    }
    return res;
}

void splitExecutePath(ClipperLib::Paths& paths, std::vector<Polygon_with_holes_2>& pwhs) {
    if (paths.size() == 0) {
        return ;
    }
    std::vector<Polygon_2> polys;
    for (ClipperLib::Path path : paths) {
        polys.emplace_back(pointsToPolygon(path));
    }
    for (int i = 0; i < polys.size(); i++) {
        Polygon_with_holes_2 pwh(polys[i]);
        int j = i + 1;
        for (; j < polys.size(); j++) {
            if ( polys[j].area() > 0) {
                break;
            }
            pwh.add_hole(polys[j]);
        }
        if (j != i+1) {
            i = j + 1;
        }
        pwhs.emplace_back(pwh);
    }
}

void polygonsIntersection(ClipperLib::Path& result, Polygon_with_holes_2& sub, ClipperLib::Path& clip_path) {
    if (sub.is_unbounded() || clip_path.size() == 0) {
        return ;
    }
    ClipperLib::Paths ret;
    ClipperLib::Paths sub_paths;
    pwhToPoints(sub, sub_paths);

    ClipperLib::Clipper clipper(0);
    clipper.AddPaths(sub_paths, ClipperLib::ptSubject, true);
    clipper.AddPath(clip_path, ClipperLib::ptClip, true);
    clipper.Execute(ClipperLib::ctIntersection, ret);
    result = ret[0];
}

Polygon_with_holes_2 polygonDifferenceClipper(Polygon_with_holes_2& sub, Polygon_with_holes_2& clip) {
    if (sub.is_unbounded() || clip.is_unbounded()) {
        return Polygon_with_holes_2();
    }
    ClipperLib::Paths ret;
    ClipperLib::Paths sub_paths;
    pwhToPoints(sub, sub_paths);
    ClipperLib::Paths clip_paths;
    pwhToPoints(clip, clip_paths);

    ClipperLib::Clipper clipper(0);
    clipper.AddPaths(sub_paths, ClipperLib::ptSubject, true);
    clipper.AddPaths(clip_paths, ClipperLib::ptClip, true);
    clipper.Execute(ClipperLib::ctDifference, ret);

    return pointTpPwh(ret);
}

void polygonOffset(std::vector<Polygon_with_holes_2>& res, Polygon_with_holes_2& pwh, int distance, ClipperLib::JoinType join_type = ClipperLib::jtMiter, double miter_limit = 3, double roundPrecision = 0.25) {
    ClipperLib::Paths paths;
    pwhToPoints(pwh, paths);

    if (distance == 0) {
        return ;
    }

    ClipperLib::Paths ret;
    ClipperLib::ClipperOffset clipper(miter_limit, roundPrecision);
    clipper.AddPaths(paths, join_type,ClipperLib::etClosedPolygon);
    clipper.MiterLimit = miter_limit;
    clipper.Execute(ret, distance);

    splitExecutePath(ret, res);
    return ;
}

void polygonOffset(ClipperLib::Paths& ret, Polygon_with_holes_2& pwh, int distance, ClipperLib::JoinType join_type = ClipperLib::jtMiter, double miter_limit = 3.0, double roundPrecision = 0.25) {
    ClipperLib::Paths paths;
    pwhToPoints(pwh, paths);

    if (distance == 0) {
        return ;
    }

    ClipperLib::ClipperOffset clipper(miter_limit, roundPrecision);
    clipper.AddPaths(paths, join_type,ClipperLib::etClosedPolygon);
    clipper.MiterLimit = miter_limit;
    clipper.Execute(ret, distance);

    return ;
}

void polygonOffset(ClipperLib::Path& result, ClipperLib::Path& path, int distance, ClipperLib::JoinType join_type = ClipperLib::jtMiter, double miter_limit = 3, double roundPrecision = 0.25) {
    if (distance == 0) {
        return ;
    }
    ClipperLib::Paths ret;
    ClipperLib::ClipperOffset clipper(miter_limit, roundPrecision);
    clipper.AddPath(path, join_type,ClipperLib::etClosedPolygon);
    clipper.MiterLimit = miter_limit;
    clipper.Execute(ret, distance);

    result = ret[0];
    return ;
}

void roundAndMulPath(ClipperLib::Path& path, double limit = 1) {
    for (int i = 0; i < path.size(); i++) {
        path[i].X = round(path[i].X * limit);
        path[i].Y = round(path[i].Y * limit);
    }
}

void removeSelfIntersect(std::vector<Polygon_with_holes_2>& res, Polygon_with_holes_2& pwh, int distance) {
    ClipperLib::Paths shrink_paths;
    polygonOffset(shrink_paths, pwh, -distance);

    ClipperLib::Paths expand_paths;
    for (ClipperLib::Path s_path : shrink_paths) {
        ClipperLib::Path outer_path;
        polygonOffset(outer_path, s_path, distance);
        roundAndMulPath(outer_path, 2);
        roundAndMulPath(outer_path, 0.5);

        ClipperLib::Path union_path;
        polygonsIntersection(union_path, pwh, outer_path);
        expand_paths.push_back(union_path);
    }
    splitExecutePath(expand_paths, res);
}

}


#endif  // LUNARMP_SRC_LUNARMP_POLYGON_POLYGONBOOL_H_
