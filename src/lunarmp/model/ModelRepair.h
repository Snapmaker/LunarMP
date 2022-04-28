//
// Created by Cyril on 2022/3/28.
//

#ifndef LUNARMP_MODELREPAIR_H
#define LUNARMP_MODELREPAIR_H

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

class ModelRepair {

  private:
    // fill holes params
    double max_hole_diam = -1.0;    //! Lower diameter of filling hole

    int max_num_hole_edges = -1;    //! Lower limit of the number of edges for filling holes

    bool use_delaunay_triangulation = true;     //! If true, use the Delaunay triangulation facet search space.
                                                //! If no valid triangulation can be found in this search space,
                                                //! the algorithm falls back to the non-Delaunay triangulations
                                                //! search space to find a solution.
                                                //!  Default: true

    bool use_2d_constrained_delaunay_triangulation = true;      //! If true, the points of the boundary of the hole are
                                                                //! used to estimate a fitting plane and a 2D constrained
                                                                //! Delaunay triangulation is then used to fill the hole
                                                                //! projected in the fitting plane.
                                                                //!  Default: true

    K::FT threshold_distance = K::FT(-1);       //! The maximum distance between the vertices of the hole boundary and
                                                //! the least squares plane fitted to this boundary.
                                                //! This parameter is used only in conjunction with use_2d_constrained_delaunay_triangulation.

    double density_control_factor = K::FT(sqrt(2));      //! factor to control density of the output mesh, where larger values
                                                         //! cause denser refinements

    int fairing_continuity = 1;     //! A value control the tangential continuity of the output surface patch.
                                    //! The possible values are 0, 1 and 2, refer to the C0, C1 and C2 continuity.

    int least_faces_per_component = 2; //! A component need to have at least faces.


  public:

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

    CGAL::Timer t;
    double check_time = 0;                      //! The time spent inspecting the model.
    double repair_time = 0;                     //! Total model repair time taken
    double repair_basic_time = 0;               //! Time spent on base repairs
    double repair_manifoldness_time = 0;        //! Time spent on manifold vertices & edges repairs
    double repair_borders_time = 0;             //! Time spent on borders repairs
    double repair_holes_time = 0;               //! Time spent on holes repairs
    double repair_self_intersect_time = 0;      //! Time spent on self intersecting triangular plane repairs
    double read_file_time = 0;                  //! Time spent on file to read

    /*!
     * \brief Read model file as polygon soup, support off/obj/stl/ply/ts/vtp file
     *
     * \param file_name the name of the file.
     * \param points points of the soup of polygons.
     * \param polygons each element in the range describes a polygon using the indices of the vertices.
     * \param mesh model mesh
     */
    bool readPolygonSoup(std::string file_name, std::vector<K::Point_3>& points, std::vector<std::vector<std::size_t> >& polygons);

    /*!
     * \brief Writes a polygon mesh in a file.
     *
     * \param file_name the name of the file.
     * \param mesh the mesh to be output.
     */
    void writePolygon(std::string file_name, Mesh& mesh);

    /*!
     * \brief Repair polygon soup
     * step1: "repair_polygon_soup" include:
     *     - merge_duplicate_points_in_polygon_soup(points, polygons, np);         合并 polygon soup 中的重复点
     *     - simplify_polygons_in_polygon_soup(points, polygons, traits);          去除 polygon soup 中的重复点(几何临近点)
     *     - split_pinched_polygons_in_polygon_soup(points, polygons, traits);     拆分 polygon soup 中的被挤压面(顶点多次出现的面)
     *     - remove_invalid_polygons_in_polygon_soup(points, polygons);            去除 polygon soup 中的退化面
     *     - merge_duplicate_polygons_in_polygon_soup(points, polygons, np);       合并 polygon soup 中的重复面
     *     - remove_isolated_points_in_polygon_soup(points, polygons);             去除 polygon soup 中的孤立点
     * step2: "orient_polygon_soup"
     * step3: "polygon_soup_to_polygon_mesh"
     *
     * \param points points of the soup of polygons.
     * \param polygons each element in the range describes a polygon using the indices of the vertices.
     * \param mesh model mesh
     */
    void repairPolygon(std::vector<K::Point_3>& points, std::vector<std::vector<std::size_t> >& polygons, Mesh& mesh);

    /*!
     * \brief Orient polygon soup.
     */
    bool orientPolygon(std::vector<K::Point_3>& points, std::vector<std::vector<std::size_t> >& polygons);

    /*!
     * \brief Judge whether the normal vectors of the mesh face outwards uniformly.
     */
    void isOutwardMesh(Mesh& mesh);

    /*!
     * \brief Stitch borders for mesh.
     */
    void repairBorders(Mesh& mesh);

    /*!
     * \brief Check the mesh for non-streaming vertices.
     */
    void is_manifoldness(Mesh& mesh);

    /*!
     * \brief Fixed mesh non-manifold problems.
     */
    void repair_manifoldness(Mesh& mesh);

    /*!
     * \brief Determine whether there are holes in the mesh.
     */
    bool isHoleMesh(Mesh mesh);

    /*!
     * \brief Repair each hole step by step.
     */
    void repairHoleStepByStep(Mesh& mesh);

    /*!
     * \brief Repair holes larger than specified diameter.
     */
    void repairHoleOfDiameter(Mesh& mesh);

    /*!
     * \brief Get all pairs of self-intersecting triangle faces.
     */
    void getIntersectedTris(Mesh& mesh);

    /*!
     * \brief Determine whether the mesh is self-intersecting.
     */
    bool isSelfIntersect(Mesh& mesh);

    /*!
     * \brief Repair self-intersecting.
     */
    void repairSelfIntersect(Mesh& mesh);

    /*!
     * \brief Delete degenerate edges and degenerate surfaces.
     */
    void removeDegenerate(Mesh& mesh);

    /*!
     * \brief Rebuild the grid using the topology.
     */
    void isotropic_remeshing(Mesh& mesh);

    /*!
     * \brief Comprehensive repair interface.
     */
    void repairModel(std::vector<K::Point_3>& points, std::vector<std::vector<std::size_t> >& polygons, Mesh& mesh);

    /*!
     * \brief Model repair interface.
     */
    void repairModel(std::string input_file, std::string output_file);

    /*!
     * \brief Test interface.
     */
    void test();

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

#endif // LUNARMP_MODELREPAIR_H


