//
// Created by Cyril on 2022/4/8.
//

#include "ModelCheck.h"

namespace lunarmp {

void ModelCheck::checkConnectedComponents(Mesh mesh) {
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
    number_of_connected_components = (int)num;
}

void ModelCheck::checkHoles(Mesh mesh) {
    number_of_holes = 0;
    std::vector<halfedge_descriptor> border_cycles;
    PMP::extract_boundary_cycles(mesh, std::back_inserter(border_cycles));
    number_of_holes = border_cycles.size();
}

void ModelCheck::checkIntersect(Mesh mesh) {
    is_intersecting = PMP::does_self_intersect<CGAL::Parallel_if_available_tag>(mesh, NP::vertex_point_map(get(CGAL::vertex_point, mesh)));
//    std::vector<std::pair<face_descriptor, face_descriptor>> intersected_tris;
//    PMP::self_intersections<CGAL::Parallel_if_available_tag>(faces(mesh), mesh, std::back_inserter(intersected_tris));
//    number_of_intersections = intersected_tris.size();
}

void ModelCheck::checkModel(std::string input_file, std::string output_file) {
    t.start();

    std::vector<Point_3> points;
    std::vector<std::vector<std::size_t>> polygons;
    Mesh mesh;

    if (!CGAL::IO::read_polygon_soup(input_file, points, polygons) || points.empty()) {
        logError("Cannot open file.\n");
        return;
    }
    read_time = t.time();
    t.reset();

    Visitor vis(non_manifold_edge, non_manifold_vertex, duplicated_vertex, vertex_id_in_polygon_replaced, polygon_orientation_reversed);

    is_producing_self_intersecting = PMP::orient_polygon_soup(points, polygons, NP::visitor(vis));

    if (non_manifold_edge || non_manifold_vertex || duplicated_vertex || polygon_orientation_reversed) {
        check_time = t.time();
        exit(static_cast<int>(ExitType::BROKEN));
        return;
    }

    PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh, NP::outward_orientation(true));

    is_outward_mesh = PMP::is_outward_oriented(mesh);
    if (is_outward_mesh) {
        check_time = t.time();
        exit(static_cast<int>(ExitType::BROKEN));
        return;
    }

    checkConnectedComponents(mesh);
    if (number_of_connected_components) {
        check_time = t.time();
        exit(static_cast<int>(ExitType::BROKEN));
        return;
    }

    checkHoles(mesh);
    if (number_of_holes) {
        check_time = t.time();
        exit(static_cast<int>(ExitType::BROKEN));
        return;
    }

    checkIntersect(mesh);
    if (is_intersecting) {
        check_time = t.time();
        exit(static_cast<int>(ExitType::BROKEN));
        return;
    }
    check_time = t.time();

    CGAL::IO::write_polygon_mesh(output_file, mesh, NP::stream_precision(17));
    exit(static_cast<int>(ExitType::WATER));
}

void ModelCheck::checkTest1() {
    const std::string file_name = CGAL::data_file_path("E:/Datasets/ModelCheck/bun_zipper_res4.stl");
    std::string output_file = "";
    checkModel(file_name, output_file);

    std::cout << "non_manifold_edge: " << non_manifold_edge << std::endl;
    std::cout << "non_manifold_vertex: " << non_manifold_vertex << std::endl;
    std::cout << "duplicated_vertex: " << duplicated_vertex << std::endl;
    std::cout << "vertex_id_in_polygon_replaced: " << vertex_id_in_polygon_replaced << std::endl;
    std::cout << "polygon_orientation_reversed: " << polygon_orientation_reversed << std::endl;
    std::cout << "number_of_connected_components: " << number_of_connected_components << std::endl;
    std::cout << "number_of_holes: " << number_of_holes << std::endl;
    std::cout << "number_of_intersections: " << number_of_intersections << std::endl;
    std::cout << "is_outward_mesh: " << is_outward_mesh << std::endl;
    std::cout << "is_intersecting: " << is_intersecting << std::endl;
    std::cout << "is_producing_self_intersecting: " << is_producing_self_intersecting << std::endl;
}

bool ModelCheck::checkTest(std::string file_name, std::string output_file) {
    CGAL::Timer t1;
    std::cout << "\nstart: " << std::endl;

    t1.start();
    checkModel(file_name, output_file);
    check_time = t1.time();

    std::cout << "non_manifold_edge: " << non_manifold_edge << std::endl;
    std::cout << "non_manifold_vertex: " << non_manifold_vertex << std::endl;
    std::cout << "duplicated_vertex: " << duplicated_vertex << std::endl;
    std::cout << "vertex_id_in_polygon_replaced: " << vertex_id_in_polygon_replaced << std::endl;
    std::cout << "polygon_orientation_reversed: " << polygon_orientation_reversed << std::endl;
    std::cout << "number_of_connected_components: " << number_of_connected_components << std::endl;
    std::cout << "number_of_holes: " << number_of_holes << std::endl;
    std::cout << "number_of_intersections: " << number_of_intersections << std::endl;
    std::cout << "is_outward_mesh: " << is_outward_mesh << std::endl;
    std::cout << "is_intersecting: " << is_intersecting << std::endl;
    std::cout << "is_producing_self_intersecting: " << is_producing_self_intersecting << std::endl;

    if (non_manifold_edge || non_manifold_vertex || number_of_holes || !is_outward_mesh || is_intersecting || number_of_connected_components || number_of_holes) {
        return false;
    } else {
        return true;
    }
}

}  // namespace lunarmp
