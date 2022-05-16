//
// Created by Cyril on 2022/4/22.
//

#include "ModelSimplification.h"

namespace lunarmp {

bool ModelSimplification::readFile(std::string input_file, Mesh& mesh) {
    const std::string filename = CGAL::data_file_path(input_file);
    if(!PMP::IO::read_polygon_mesh(filename, mesh))
    {
        std::cerr << "Invalid input." << std::endl;
        return EXIT_FAILURE;
    }
    if(!CGAL::is_triangle_mesh(mesh))
    {
        std::cerr << "Input geometry is not triangulated." << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

typedef typename SMS::GarlandHeckbert_policies<Mesh, K>            GH_policies;
typedef typename GH_policies::Get_cost                             GH_cost;
typedef typename GH_policies::Get_placement                        GH_placement;
typedef SMS::Bounded_normal_change_placement<GH_placement>         Bounded_GH_placement;
void ModelSimplification::edgeCollapseGarlandHeckbert(Mesh& mesh, double count_ratio_stop = 0.2)
{
    CGAL::Timer t;
    t.start();
    SMS::Count_ratio_stop_predicate<Mesh> stop(count_ratio_stop);

    GH_policies gh_policies(mesh);
    const GH_cost& gh_cost = gh_policies.get_cost();
    const GH_placement& gh_placement = gh_policies.get_placement();
    Bounded_GH_placement placement(gh_placement);

    log("Collapsing edges of mesh, aiming for %.2lf % of the input edges...\n", count_ratio_stop);
    remove_edge = SMS::edge_collapse(mesh, stop,
                                     NP::get_cost(gh_cost)
                                         .get_placement(placement));

    simplification_time = t.time();
    log("Time elapsed: %f4lf s.\n", simplification_time);
    log("Finished.\n Removed edges: %d, final edges: %d.\n", remove_edge, mesh.number_of_edges());

}

void ModelSimplification::edgeCollapseAllShortEdges(Mesh& mesh, double threshold = 0.08)
{
    CGAL::Timer t;
    t.start();
    remove_edge = SMS::edge_collapse(mesh,
                                     SMS::Edge_length_stop_predicate<double>(threshold),
                                     NP::get_cost(SMS::Edge_length_cost <Mesh>())
                                         .get_placement(SMS::Midpoint_placement<Mesh>()));

    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    simplification_time = t.time();
    log("Time elapsed: %f s.\n", simplification_time);
    log("Finished.\n Removed edges: %d, final edges: %d.\n", remove_edge, mesh.number_of_edges());
}

void ModelSimplification::edgeCollapseBoundedNormalChange(Mesh& mesh, double Edges_limit = 10000)
{
    CGAL::Timer t;
    t.start();
    SMS::Count_stop_predicate<Mesh> stop(Edges_limit);
    typedef SMS::LindstromTurk_placement<Mesh> Placement;
    SMS::Bounded_normal_change_filter<> filter;
    SMS::edge_collapse(mesh, stop,
                       NP::get_cost(SMS::LindstromTurk_cost<Mesh>())
                           .filter(filter)
                           .get_placement(Placement()));

    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    simplification_time = t.time();
    log("Time elapsed: %f4lf s.\n", simplification_time);
    log("Finished.\n Removed edges: %d, final edges: %d.\n", remove_edge, mesh.number_of_edges());

}

void ModelSimplification::modelSimplification(std::string input_file, std::string output_file){
    Mesh mesh;
    DataGroup data_group;

    readFile(input_file, mesh);

    SIMPLIFYMethod type = data_group.settings.get<SIMPLIFYMethod>("simplify_type");

    switch(type)
    {
        case SIMPLIFYMethod::edge_length_stop:
        {
            auto edge_length_threshold = data_group.settings.get<double>("edge_length_threshold");
            edgeCollapseAllShortEdges(mesh, edge_length_threshold);
            break;
        }
        case SIMPLIFYMethod::edge_count_stop:
        {
            auto edge_count_threshold = data_group.settings.get<double>("edge_count_threshold");
            edgeCollapseBoundedNormalChange(mesh, edge_count_threshold);
            break;
        }
        case SIMPLIFYMethod::edge_ratio_stop:
        {
            auto edge_ratio_threshold = data_group.settings.get<double>("edge_ratio_threshold");
            edgeCollapseGarlandHeckbert(mesh, edge_ratio_threshold);
            break;
        }
        default:
            break;
    }

    CGAL::IO::write_polygon_mesh(output_file, mesh, NP::stream_precision(17));
}

}
