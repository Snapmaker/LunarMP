//
// Created by Cyril on 2022/4/8.
//

#include "ModelCheck.h"

namespace lunarmp {

void ModelCheck::writeMesh(std::string output_file, Mesh& mesh) {
    CGAL::IO::write_polygon_mesh(output_file+".stl", mesh, NP::stream_precision(17));
    CGAL::IO::write_polygon_mesh(output_file+".ply", mesh, NP::stream_precision(17));
}

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

bool ModelCheck::checkBorder(Mesh mesh) {
    std::unordered_set<halfedge_descriptor> hedge_handled;
    for(halfedge_descriptor h : halfedges(mesh)) {
        if (is_border(h, mesh)) {
            return true;
        }
    }
    return false;
}


void ModelCheck::checkIntersect(Mesh mesh) {
    is_intersecting = PMP::does_self_intersect<CGAL::Parallel_if_available_tag>(mesh, NP::vertex_point_map(get(CGAL::vertex_point, mesh)));
}

void ModelCheck::checkModel(std::string input_file, std::string output_file) {
    t.start();
    std::vector<Point_3> points;
    std::vector<std::vector<std::size_t> > polygons;
    Mesh mesh;
    const std::string filename = CGAL::data_file_path(input_file);
    if (!CGAL::IO::read_polygon_soup(filename, points, polygons) || points.empty()) {
        logError("Cannot open file.\n");
        return;
    }
    PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh, NP::outward_orientation(true));
    read_time = t.time();
    log("read file time: %.3f\n", read_time);
    t.reset();

    is_outward_mesh = PMP::is_outward_oriented(mesh);
    if (!is_outward_mesh) {
        check_time = t.time();
        log("check file time: %.3f", check_time);
        writeMesh(output_file, mesh);
        exit(static_cast<int>(ExitType::BROKEN));
    }
    log("normal check: %.3f\n", t.time());

    if (checkBorder(mesh)) {
        check_time = t.time();
        log("check file time: %.3f", check_time);
        writeMesh(output_file, mesh);
        exit(static_cast<int>(ExitType::BROKEN));
    }
    log("Border check: %.3f\n", t.time());

    checkIntersect(mesh);
    if (is_intersecting) {
        check_time = t.time();
        log("check file time: %.3f", check_time);
        writeMesh(output_file, mesh);
        exit(static_cast<int>(ExitType::BROKEN));
    }
    log("is_intersecting check: %.3f\n", t.time());

    check_time = t.time();
    log("check file time: %.3f", check_time);

    writeMesh(output_file, mesh);
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
