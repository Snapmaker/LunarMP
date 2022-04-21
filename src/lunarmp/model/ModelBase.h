//
// Created by Cyril on 2022/4/8.
//

#ifndef LUNARMP_MODELBASE_H
#define LUNARMP_MODELBASE_H
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_3 Point_3;
typedef CGAL::Surface_mesh<Point_3> triangular_Mesh;

typedef boost::graph_traits<triangular_Mesh>::face_descriptor face_descriptor;
typedef boost::graph_traits<triangular_Mesh>::edge_descriptor edge_descriptor;
typedef boost::graph_traits<triangular_Mesh>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<triangular_Mesh>::halfedge_descriptor halfedge_descriptor;
typedef K::Compare_dihedral_angle_3 Compare_dihedral_angle_3;

namespace PMP = CGAL::Polygon_mesh_processing;
namespace NP = CGAL::parameters;

struct Visitor : public PMP::Default_orientation_visitor {
    int* non_manifold_edge_num;
    int* non_manifold_vertex_num;
    int* duplicated_vertex_num;
    int* vertex_id_in_polygon_replaced_num;
    int* polygon_orientation_reversed_num;

    Visitor(int& nmE, int& nmV, int& dv, int& vpr, int& por) {
        non_manifold_edge_num = &nmE;
        non_manifold_vertex_num = &nmV;
        duplicated_vertex_num = &dv;
        vertex_id_in_polygon_replaced_num = &vpr;
        polygon_orientation_reversed_num = &por;
    }

    void non_manifold_edge(std::size_t id1, std::size_t id2, std::size_t nb_poly) { (*non_manifold_edge_num)++; }
    void non_manifold_vertex(std::size_t id, std::size_t nb_cycles) { (*non_manifold_vertex_num)++; }
    void duplicated_vertex(std::size_t v1, std::size_t v2) { (*duplicated_vertex_num)++; }
    void vertex_id_in_polygon_replaced(std::size_t p_id, std::size_t i1, std::size_t i2) { (*vertex_id_in_polygon_replaced_num)++; }
    void polygon_orientation_reversed(std::size_t p_id) { (*polygon_orientation_reversed_num)++; }
};

template <typename G>
struct Constraint {
    typedef boost::readable_property_map_tag category;
    typedef bool value_type;
    typedef bool reference;
    typedef edge_descriptor key_type;
    Constraint() : g_(NULL) {}
    Constraint(G& g, double bound) : g_(&g), bound_(bound) {}
    value_type operator[](edge_descriptor e) const {
        const G& g = *g_;
        return compare_(g.point(source(e, g)), g.point(target(e, g)), g.point(target(next(halfedge(e, g), g), g)),
                        g.point(target(next(opposite(halfedge(e, g), g), g), g)), bound_) == CGAL::SMALLER;
    }
    friend inline value_type get(const Constraint& m, const key_type k) { return m[k]; }
    const G* g_;
    Compare_dihedral_angle_3 compare_;
    double bound_;
};

template <typename PM>
struct Put_true {
    Put_true(const PM pm) : pm(pm) {}
    template <typename T>
    void operator()(const T& t) {
        put(pm, t, true);
    }
    PM pm;
};

#endif  // LUNARMP_MODELBASE_H
