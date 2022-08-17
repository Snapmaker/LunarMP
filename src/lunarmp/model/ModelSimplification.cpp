//
// Created by Cyril on 2022/4/22.
//

#include "ModelSimplification.h"
#include <fstream>

namespace lunarmp {

void ModelSimplification::writeMesh(std::string output_file, Mesh& mesh) {
    CGAL::IO::write_polygon_mesh(output_file+".stl", mesh, NP::stream_precision(17));
//    CGAL::IO::write_polygon_mesh(output_file+".ply", mesh, NP::stream_precision(17));

}

bool ModelSimplification::readFile(std::string input_file, Mesh& mesh) {
    const std::string filename = CGAL::data_file_path(input_file);

    std::vector<Point_3> points;
    std::vector<std::vector<std::size_t> > polygons;
    if (!CGAL::IO::read_polygon_soup(filename, points, polygons) || points.empty()) {
        logError("Cannot open file.\n");
        return false;
    }
    PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh, NP::outward_orientation(true));
    if (!CGAL::is_triangle_mesh(mesh)) {
        logError("Input geometry is not triangulated.\n");
        return false;
    }
    return true;
}

typedef typename SMS::GarlandHeckbert_policies<Mesh, K> GH_policies;
typedef typename GH_policies::Get_cost GH_cost;
typedef typename GH_policies::Get_placement GH_placement;
typedef SMS::Bounded_normal_change_placement<GH_placement> Bounded_GH_placement;
void ModelSimplification::edgeCollapseGarlandHeckbert(Mesh& mesh, double count_ratio_threshold) {
    SMS::Count_ratio_stop_predicate<Mesh> stop(count_ratio_threshold);
    GH_policies gh_policies(mesh);
    const GH_cost& gh_cost = gh_policies.get_cost();
    const GH_placement& gh_placement = gh_policies.get_placement();
    Bounded_GH_placement placement(gh_placement);

    log("Collapsing edges of mesh, aiming for %.2lf of the input edges...\n", count_ratio_threshold);
    remove_edge = SMS::edge_collapse(mesh, stop, NP::get_cost(gh_cost).get_placement(placement));

    log("Finished.\n Removed edges: %d, final edges: %d.\n", remove_edge, mesh.number_of_edges());
}

void ModelSimplification::edgeCollapseAllShortEdges(Mesh& mesh, double edge_length_threshold) {
    remove_edge = SMS::edge_collapse(mesh, SMS::Edge_length_stop_predicate<double>(edge_length_threshold),
                                     NP::get_cost(SMS::Edge_length_cost<Mesh>()).get_placement(SMS::Midpoint_placement<Mesh>()));

    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    log("Finished.\n Removed edges: %d, final edges: %d.\n", remove_edge, mesh.number_of_edges());
}

void ModelSimplification::edgeCollapseBoundedNormalChange(Mesh& mesh, double edge_count_threshold) {
    SMS::Count_stop_predicate<Mesh> stop(edge_count_threshold);
    typedef SMS::LindstromTurk_placement<Mesh> Placement;
    SMS::Bounded_normal_change_filter<> filter;
    remove_edge = SMS::edge_collapse(mesh, stop, NP::get_cost(SMS::LindstromTurk_cost<Mesh>()).filter(filter).get_placement(Placement()));

    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    log("Finished.\n Removed edges: %d, final edges: %d.\n", remove_edge, mesh.number_of_edges());
}

void ModelSimplification::modelSimplification(std::string input_file, std::string output_file, DataGroup& data_group) {
    Mesh mesh;
    CGAL::Timer t;
    t.start();
    if (!readFile(input_file, mesh)) {
        exit(2);
    }
    read_time = t.time();
    log("read Time: %.3f\n", read_time);
    t.reset();

    SimplifyType type = data_group.settings.get<SimplifyType>("simplify_type");

    switch (type) {
        case SimplifyType::EDGE_LENGTH_STOP: {
            auto edge_length_threshold = data_group.settings.get<double>("edge_length_threshold");
            edgeCollapseAllShortEdges(mesh, edge_length_threshold);
            simplification_time = t.time();
            break;
        }
        case SimplifyType::EDGE_COUNT_STOP: {
            auto edge_count_threshold = data_group.settings.get<double>("edge_count_threshold");
            edgeCollapseBoundedNormalChange(mesh, edge_count_threshold);
            simplification_time = t.time();
            break;
        }
        case SimplifyType::EDGE_RATIO_STOP: {
            auto edge_ratio_threshold = data_group.settings.get<double>("edge_ratio_threshold");
//            edgeCollapseGarlandHeckbert(mesh, edge_ratio_threshold);
            edgeCollapseBoundedNormalChange(mesh, mesh.edges().size() * edge_ratio_threshold);
            simplification_time = t.time();
            break;
        }
        default:
            break;
    }
    log("Time elapsed: %f4lf s.\n", simplification_time);
    writeMesh(output_file, mesh);
}

}  // namespace lunarmp
