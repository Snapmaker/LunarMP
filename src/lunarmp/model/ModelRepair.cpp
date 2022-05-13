//
// Created by Cyril on 2022/3/24.
//

#ifndef LUNARMP_POLYGONMESHREPAIR_H
#define LUNARMP_POLYGONMESHREPAIR_H

#include "ModelRepair.h"

namespace lunarmp {

bool ModelRepair::readPolygonSoup(std::string file_name, std::vector<Point_3>& points, std::vector<std::vector<std::size_t> >& polygons) {
    if (!CGAL::IO::read_polygon_soup(file_name, points, polygons) || points.empty()) {
        logError("Cannot open file.\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void ModelRepair::writePolygon(std::string file_name, Mesh& mesh) {
    log("Writing file.\n");
    CGAL::IO::write_polygon_mesh(file_name, mesh, NP::stream_precision(17));
}

void ModelRepair::isOutwardMesh(Mesh& mesh) {
    if (PMP::is_outward_oriented(mesh)) {
        log("Mesh is outward.\n");
    } else {
        log("Mesh is not outward, start repairing.\n");
        PMP::orient(mesh);
        log("Repairing mesh orient.\n");
    }
}

bool ModelRepair::orientPolygon(std::vector<Point_3>& points, std::vector<std::vector<std::size_t> >& polygons) {
    log("Tring to consistently orient a soup of polygons in 3D space.\n");
    Visitor vis(non_manifold_edge, non_manifold_vertex, duplicated_vertex, vertex_id_in_polygon_replaced, polygon_orientation_reversed);
    bool is_producing_self_intersecting = PMP::orient_polygon_soup(points, polygons, NP::visitor(vis));

    log("Mesh has non_manifold_edge: %d\n", non_manifold_edge);
    log("Mesh has non_manifold_vertex: %d\n", non_manifold_vertex);
    log("Mesh has duplicated_vertex: %d\n", duplicated_vertex);
    log("Mesh has vertex_id_in_polygon_replaced: %d\n", vertex_id_in_polygon_replaced);
    log("Mesh has polygon_orientation_reversed: %d\n", polygon_orientation_reversed);

    return is_producing_self_intersecting;
}

void ModelRepair::repairPolygon(std::vector<Point_3>& points, std::vector<std::vector<std::size_t> >& polygons, Mesh& mesh) {
    log("Start repairing polygon soup.\n");
    log("Before reparation, the soup has %d vertices and %d faces.\n", points.size(), polygons.size());
    PMP::repair_polygon_soup(points, polygons, NP::erase_all_duplicates(false).require_same_orientation(false));
    log("After reparation, the soup has %d vertices and %d faces.\n", points.size(), polygons.size());

    orientPolygon(points, polygons);

    log("Building a polygon mesh from a soup of polygons.\n");
    PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh, NP::outward_orientation(true));

    if (CGAL::is_closed(mesh)) {
        log("mesh is cloesd, orients the connected components of tm to make it bound a volume.\n");
        PMP::orient_to_bound_a_volume(mesh, NP::outward_orientation(true));
    }
}

void ModelRepair::repairManifoldness(Mesh& mesh) {
    const double bound = std::cos(0.75 * CGAL_PI);
    std::vector<face_descriptor> cc;
    face_descriptor fd = *faces(mesh).first;
    PMP::connected_component(fd, mesh, std::back_inserter(cc));

    // Instead of writing the faces into a container, you can set a face property to true
    typedef Mesh::Property_map<face_descriptor, bool> F_select_map;
    F_select_map fselect_map = mesh.add_property_map<face_descriptor, bool>("f:select", false).first;

    PMP::connected_component(fd, mesh, boost::make_function_output_iterator(Put_true<F_select_map>(fselect_map)));

    Mesh::Property_map<face_descriptor, std::size_t> fccmap = mesh.add_property_map<face_descriptor, std::size_t>("f:CC").first;
    std::size_t num = PMP::connected_components(mesh, fccmap, NP::edge_is_constrained_map(Constraint<Mesh>(mesh, bound)));
    log("- The graph has %d connected components (face connectivity).\n", num);

    typedef std::map<std::size_t /*index of CC*/, unsigned int /*nb*/> Components_size;

    Components_size nb_per_cc;
    for (face_descriptor f : faces(mesh)) {
        nb_per_cc[fccmap[f]]++;
    }
    log("- We keep only components which have at least %d faces.\n", least_faces_per_component);
    PMP::keep_large_connected_components(mesh, least_faces_per_component, NP::edge_is_constrained_map(Constraint<Mesh>(mesh, bound)));
}

typedef CGAL::Halfedge_around_face_circulator<Mesh> Halfedge_around_facet_circulator;
void detect_borders(Mesh& poly, std::vector<halfedge_descriptor>& border_reps) {
    border_reps.clear();
    std::set<halfedge_descriptor> border_map;
    for (halfedge_descriptor h : halfedges(poly)) {
        if (face(h, poly) == boost::graph_traits<Mesh>::null_face() && border_map.find(h) == border_map.end()) {
            border_reps.push_back(h);
            Halfedge_around_facet_circulator hf_around_facet(h, poly), done(hf_around_facet);
            do {
                bool insertion_ok = border_map.insert(*hf_around_facet).second;
                assert(insertion_ok);
            } while (++hf_around_facet != done);
        }
    }
}

void ModelRepair::repairBorders(Mesh& mesh) {
    log("Start stitching borders.\n");
    PMP::stitch_borders(mesh, NP::apply_per_connected_component(false));
}

bool ModelRepair::isHoleMesh(Mesh tMesh) {
    unsigned int nb_holes = 0;
    for (halfedge_descriptor h : tMesh.halfedges()) {
        if (CGAL::is_border(h, tMesh)) {
            return true;
        }
    }
    return false;
}

void ModelRepair::repairHoleStepByStep(Mesh& tMesh) {
    unsigned int nb_holes = 0;
    int success_fill = 0;
    for (halfedge_descriptor h : tMesh.halfedges()) {
        if (CGAL::is_border(h, tMesh)) {
            std::vector<face_descriptor> patch_faces;
            std::vector<vertex_descriptor> patch_vertices;
            bool success = std::get<0>(PMP::triangulate_refine_and_fair_hole(tMesh, h, std::back_inserter(patch_faces), std::back_inserter(patch_vertices)));
            if (success) {
                success_fill++;
            }
            ++nb_holes;
        }
    }
}

bool isSmallHole(halfedge_descriptor h, Mesh& mesh, double max_hole_diam, int max_num_hole_edges) {
    int num_hole_edges = 0;
    CGAL::Bbox_3 hole_bbox;
    for (halfedge_descriptor hc : CGAL::halfedges_around_face(h, mesh)) {
        const Point_3& p = mesh.point(target(hc, mesh));
        hole_bbox += p.bbox();
        ++num_hole_edges;
        // Exit early, to avoid unnecessary traversal of large holes
        if (num_hole_edges > max_num_hole_edges) {
            return false;
        }
        if (hole_bbox.xmax() - hole_bbox.xmin() > max_hole_diam) {
            return false;
        }
        if (hole_bbox.ymax() - hole_bbox.ymin() > max_hole_diam) {
            return false;
        }
        if (hole_bbox.zmax() - hole_bbox.zmin() > max_hole_diam) {
            return false;
        }
    }
    return true;
}

void ModelRepair::repairHoleOfDiameter(Mesh& mesh) {
    log("Start repairing hole.\n");
    // Both of these must be positive in order to be considered
    unsigned int nb_holes = 0;
    std::vector<halfedge_descriptor> border_cycles;
    // collect one halfedge per boundary cycle
    PMP::extract_boundary_cycles(mesh, std::back_inserter(border_cycles));
    int success_fill = 0;
    for (halfedge_descriptor h : border_cycles) {
        if (max_hole_diam > 0 && max_num_hole_edges > 0 && !isSmallHole(h, mesh, max_hole_diam, max_num_hole_edges)) {
            continue;
        }
        std::vector<face_descriptor> patch_facets;
        std::vector<vertex_descriptor> patch_vertices;
        bool success = std::get<0>(PMP::triangulate_refine_and_fair_hole(mesh, h, std::back_inserter(patch_facets), std::back_inserter(patch_vertices)));

        if (success) {
            success_fill++;
        }
        ++nb_holes;
    }
    log("The mesh has %d holes. \nsuccessfully fill %d holes.\n", nb_holes, success_fill);
}

bool ModelRepair::isSelfIntersect(Mesh& mesh) {
    bool intersecting = PMP::does_self_intersect<CGAL::Parallel_if_available_tag>(mesh, NP::vertex_point_map(get(CGAL::vertex_point, mesh)));
    if (intersecting) {
        log("There are self-intersections.\n");
        return true;
    } else {
        log("There is no self-intersection.\n");
        return false;
    }
}

void ModelRepair::repairSelfIntersect(Mesh& mesh) {
    log("Start repairing self-intersections.\n");
    std::vector<std::pair<face_descriptor, face_descriptor> > intersected_tris;
    PMP::self_intersections<CGAL::Parallel_if_available_tag>(faces(mesh), mesh, std::back_inserter(intersected_tris));
    log("%d pairs of triangles intersect.\n", intersected_tris.size());
}

void ModelRepair::repairModel(std::vector<Point_3>& points, std::vector<std::vector<std::size_t> >& polygons, Mesh& mesh) {
    t.reset();
    if (PMP::is_polygon_soup_a_polygon_mesh(polygons)) {
        log("The polygon soup is a polygon mesh.\n");
        if (orientPolygon(points, polygons)) {
            log("The polygon soup is not a polygon mesh.\n");
        }

        PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh, NP::outward_orientation(true));

        if (CGAL::is_closed(mesh)) {
            PMP::orient_to_bound_a_volume(mesh, NP::outward_orientation(true));
        }
    } else {
        repairPolygon(points, polygons, mesh);
    }
    repair_basic_time = t.time();
    t.reset();

    if (non_manifold_edge || non_manifold_vertex) {
        t.reset();
        repairManifoldness(mesh);
        repair_manifoldness_time = t.time();
        t.reset();
    }

    if (isHoleMesh(mesh)) {
        t.reset();
        repairBorders(mesh);
        repair_borders_time = t.time();
        t.reset();

        repairHoleOfDiameter(mesh);
        repair_holes_time = t.time();
        t.reset();
    }

    if (isSelfIntersect(mesh)) {
        t.reset();
        repairSelfIntersect(mesh);
        repair_self_intersect_time = t.time();
        t.reset();
    }
}

void ModelRepair::repairModel(std::string input_file, std::string output_file) {
    std::vector<Point_3> points;
    std::vector<std::vector<std::size_t> > polygons;
    Mesh mesh;
    t.start();
    readPolygonSoup(input_file, points, polygons);
    read_file_time = t.time();
    repairModel(points, polygons, mesh);
    repair_time = repair_basic_time + repair_manifoldness_time + repair_borders_time + repair_holes_time + repair_self_intersect_time;

    writePolygon(output_file, mesh);
}

void ModelRepair::test() {
    const std::string file_name = CGAL::data_file_path("E:/Datasets/modelrepair/078.stl");

    std::vector<Point_3> points;
    std::vector<std::vector<std::size_t> > polygons;
    Mesh mesh;
    readPolygonSoup(file_name, points, polygons);

    repairModel(points, polygons, mesh);

    const std::string output_file = "E:/Datasets/modelrepair/out_demo/078.stl";
    writePolygon(output_file, mesh);
}

}  // namespace lunarmp

#endif  // LUNARMP_POLYGONMESHREPAIR_H
