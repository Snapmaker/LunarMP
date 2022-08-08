//
// Created by Cyril on 2022/8/1.
//
#pragma once
#ifndef LUNARMP_SRC_LUNARMP_UTILS_POLYGONBASE_H_
#define LUNARMP_SRC_LUNARMP_UTILS_POLYGONBASE_H_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/Polygon_2.h>

#include <CGAL/Kernel/global_functions.h>
#include <CGAL/intersection_2.h>

#include <cmath>

typedef CGAL::Exact_predicates_inexact_constructions_kernel                 K;
typedef CGAL::Polygon_2<K>                                                  Polygon_2;
typedef CGAL::Polygon_with_holes_2<K>                                       Polygon_with_holes_2;
typedef K::Segment_2                                                        Segment_2;
typedef K::Point_2                                                          Point_2;
typedef K::Vector_2                                                         Vector_2;
typedef K::FT                                                               FT;

typedef Polygon_2::Vertex_iterator                                          VertexIterator;
typedef Polygon_2::Edge_const_iterator                                      EdgeIterator;


#endif  // LUNARMP_SRC_LUNARMP_UTILS_POLYGONBASE_H_
