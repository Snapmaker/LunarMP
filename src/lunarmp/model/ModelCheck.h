//
// Created by Cyril on 2022/4/28.
//

#ifndef LUNARMP_SRC_LUNARMP_MODEL_MODELCHECK_H_
#define LUNARMP_SRC_LUNARMP_MODEL_MODELCHECK_H_

#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/repair_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/stitch_borders.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <CGAL/IO/polygon_soup_io.h>

#include "ModelBase.h"

#include <iostream>
#include <fstream>

namespace lunarmp {

class ModelCheck {
  public:
    CGAL::Timer t;

    int non_manifold_edge = 0;               //! Numbers of non-manifold_edge
    int non_manifold_vertex = 0;             //! non-manifold vertex
    int duplicated_vertex = 0;               //! Numbers of duplicated vertex
    int vertex_id_in_polygon_replaced = 0;   //! Numbers of vertex_id in polygon replaced
    int polygon_orientation_reversed = 0;    //! Numbers of polygon orientation reversed
    int number_of_connected_components = 0;  //! Number of connected components
    int number_of_holes = 0;                 //! Number of holes
    int number_of_intersections = 0;         //! Number of intersections

    bool is_outward_mesh = false; //! A closed triangle mesh has a positive orientation.
    bool is_intersecting = false; //! A triangulated surface mesh self-intersects.
    bool is_producing_self_intersecting = false; //! Number of intersections between a subset of faces of a triangulated surface mesh.

    double check_time = 0;                      //! The time spent inspecting the model.

    /*!
     * \brief Check the number of components in the model.
     */
    void checkConnectedComponents(Mesh mesh);

    /*!
     * \brief Determine whether there are holes in the mesh.
     */
    void checkHoles(Mesh mesh);

    /*!
     * \brief Determine whether the mesh is self-intersecting.
     */
    void checkIntersect(Mesh mesh);

    /*!
     * \brief Check the model for errors.
     */
    void checkModel(std::string input_file);

    /*!
     * \brief Test interface.
     */
    void checkTest1();

    bool checkTest(std::string file_name);
};

}

#endif  // LUNARMP_SRC_LUNARMP_MODEL_MODELCHECK_H_
