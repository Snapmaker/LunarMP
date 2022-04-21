//
// Created by Cyril on 2022/4/8.
//

#ifndef LUNARMP_MODELCHECK_H
#define LUNARMP_MODELCHECK_H

#include <CGAL/IO/polygon_soup_io.h>
#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>

#include <fstream>
#include <iostream>

#include "../utils/logoutput.h"
#include "ModelBase.h"

namespace lunarmp {
class ModelCheck {
  private:
    int least_faces_per_component = 2;  //! A component need to have at least faces.

  public:
    int non_manifold_edge = 0;              //! Numbers of non-manifold_edge
    int non_manifold_vertex = 0;            //! non-manifold vertex
    int duplicated_vertex = 0;              //! Numbers of duplicated vertex
    int vertex_id_in_polygon_replaced = 0;  //! Numbers of vertex_id in polygon_ replaced
    int polygon_orientation_reversed = 0;   //! Numbers of polygon orientation reversed

    int number_of_connected_components = 0;  //! Number of connected components
    int number_of_holes = 0;                 //! Number of holes
    int number_of_intersections = 0;         //! Number of intersections

    bool is_outward_mesh = false;                 //! A closed triangle mesh has a positive orientation.
    bool is_intersecting = false;                 //! A triangulated surface mesh self-intersects.
    bool is_producing_self_intersecting = false;  //! Number of intersections between a subset of faces of a triangulated surface mesh.

    /*!
     * \brief Check the number of components in the model.
     */
    void checkConnectedComponents(triangular_Mesh mesh);

    /*!
     * \brief Determine whether there are holes in the mesh.
     */
    void checkHoles(triangular_Mesh mesh);

    /*!
     * \brief Determine whether the mesh is self-intersecting.
     */
    void checkIntersect(triangular_Mesh mesh);

    /*!
     * \brief Check the model for errors.
     */
    void checkModel(std::string input_file);

    /*!
     * \brief Test interface.
     */
    void test1();
};
}  // namespace lunarmp

#endif  // LUNARMP_MODELCHECK_H
