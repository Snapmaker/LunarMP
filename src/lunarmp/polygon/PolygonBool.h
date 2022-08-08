//
// Created by Cyril on 2022/8/3.
//
#pragma once

#ifndef LUNARMP_SRC_LUNARMP_POLYGON_POLYGONBOOL_H_
#define LUNARMP_SRC_LUNARMP_POLYGON_POLYGONBOOL_H_

#include <CGAL/create_offset_polygons_from_polygon_with_holes_2.h>
#include <CGAL/Boolean_set_operations_2.h>

#include "PolygonBase.h"
typedef boost::shared_ptr<Polygon_2>                                         P_ptr;
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

template <class Type1, class Type2>
Polygon_with_holes_2 polygonIntersection(Type1& sub, Type2& clip) {
    std::list<Polygon_with_holes_2> pwhs;
    CGAL::intersection (sub, clip, std::back_inserter(pwhs));

    return pwhs.front();
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

template <class Type>
Polygon_with_holes_2 polygonOffset(Type& poly, double offset) {
    std::vector<Pwh_ptr> offset_poly_with_holes_ptr = CGAL::create_interior_skeleton_and_offset_polygons_with_holes_2(offset, poly);
    if (offset_poly_with_holes_ptr.size() == 0) {
        return Polygon_with_holes_2();
    }
    typename std::vector<Pwh_ptr>::const_iterator pi = offset_poly_with_holes_ptr.begin();
//    for (; pi < offset_poly_with_holes_ptr.end(); ++pi) {
//        printPolygonWithHoles1(**pi);
//    }
    return (**pi);
}

}


#endif  // LUNARMP_SRC_LUNARMP_POLYGON_POLYGONBOOL_H_
