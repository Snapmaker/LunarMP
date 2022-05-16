//
// Created by Cyril on 2022/4/22.
//

#ifndef LUNARMP_DEPS_SRC_LUNARMP_MODEL_MODELSIMPLIFICATION_H_
#define LUNARMP_DEPS_SRC_LUNARMP_MODEL_MODELSIMPLIFICATION_H_

#include "ModelBase.h"
#include "../data/DataGroup.h"

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Bbox_3.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>

// edgeCollapseGarlandHeckbert
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Bounded_normal_change_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/GarlandHeckbert_policies.h>
// edge_collapse_all_short_edges
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>
// edge limited
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Bounded_normal_change_filter.h>

#include <iostream>
#include <fstream>
#include <vector>

typedef CGAL::Bbox_3                                               Bbox_3;
namespace SMS = CGAL::Surface_mesh_simplification;

namespace lunarmp {
enum class SIMPLIFYMethod
{
    edge_length_stop,
    edge_count_stop,
    edge_ratio_stop
};

class ModelSimplification {
  public:
    int remove_edge = 0;                  //! The total number of edges removed in the model simplification.
    double simplification_time = 0.0;     //! The time spent simplifying the model.

    /*!
     * \brief Read file
     *
     * \param input_file the name of the output file.
     * \param mesh A triangular mesh that needs to be simplified.
     *
     */
    bool readFile(std::string input_file, Mesh& mesh);

    /*!
     * \brief Model simplification based on "Edge_length_stop_predicate" strategy
     *
     * \param mesh A triangular mesh that needs to be simplified.
     * \param count_ratio_stop
     *
     */
    void edgeCollapseGarlandHeckbert(Mesh& mesh, double count_ratio_stop);

    /*!
     * \brief Model simplification based on "Edge_length_stop_predicate" strategy
     *
     * \param mesh A triangular mesh that needs to be simplified.
     * \param edge_length_limit The limit on the length of the sides of a triangle.
     *
     */
    void edgeCollapseAllShortEdges(Mesh& mesh, double edge_length_limit);

    /*!
     * \brief Model simplification based on "Edge_length_stop_predicate" strategy
     *
     * \param mesh A triangular mesh that needs to be simplified.
     * \param edge_length_length The limit on the length of the sides of a triangle.
     *
     */
    void edgeCollapseBoundedNormalChange(Mesh& mesh, double edge_length_limit);

    /*!
     * \brief Simplify model
     *
     * \param input_file the name of input file.
     * \param output_file the name of the output file.
     * \param type Simplified strategy.
     * \param stop_predicate_threshold The minimum side length of a triangle.
     *
     */
    void modelSimplification(std::string input_file, std::string output_file);

};

}

#endif  // LUNARMP_DEPS_SRC_LUNARMP_MODEL_MODELSIMPLIFICATION_H_
