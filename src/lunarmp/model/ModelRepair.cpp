//
// Created by Cyril on 2022/3/24.
//

#ifndef LUNARMP_POLYGONMESHREPAIR_H
#define LUNARMP_POLYGONMESHREPAIR_H

#include "ModelRepair.h"

namespace lunarmp {

void statusCode(int type, char* message) {
    log("Step:%d;Message:%s\n", type, message);
}

bool ModelRepair::readPolygonSoup(std::string file_name, std::vector<Point_3>& points, std::vector<std::vector<std::size_t> >& polygons) {

    if (!CGAL::IO::read_polygon_soup(file_name, points, polygons) || points.empty()) {
        logError("Cannot open file.\n");
        return false;
    }
    return true;
}

void ModelRepair::writePolygon(std::string file_name, Mesh& mesh) {
    log("Writing file.\n");
    CGAL::IO::write_polygon_mesh(file_name+".stl", mesh, NP::stream_precision(17));
//    CGAL::IO::write_polygon_mesh(file_name+".ply", mesh, NP::stream_precision(17));
}

void ModelRepair::isOutwardMesh(Mesh& mesh) {
    if (PMP::is_outward_oriented(mesh)) {
        log("- Mesh is outward.\n");
    } else {
        log("- Mesh is not outward, start repairing.\n");
        PMP::orient(mesh);
        log("-Repairing mesh orient.\n");
    }
}

bool ModelRepair::orientPolygon(std::vector<Point_3>& points, std::vector<std::vector<std::size_t> >& polygons) {
    Visitor vis(non_manifold_edge, non_manifold_vertex, duplicated_vertex, vertex_id_in_polygon_replaced, polygon_orientation_reversed);
    bool is_producing_self_intersecting = PMP::orient_polygon_soup(points, polygons, NP::visitor(vis));

    log("- Mesh has non_manifold_edge: %d\n", non_manifold_edge);
    log("- Mesh has non_manifold_vertex: %d\n", non_manifold_vertex);
    log("- Mesh has duplicated_vertex: %d\n", duplicated_vertex);
    log("- Mesh has vertex_id_in_polygon_replaced: %d\n", vertex_id_in_polygon_replaced);
    log("- Mesh has polygon_orientation_reversed: %d\n", polygon_orientation_reversed);

    return is_producing_self_intersecting;
}

void ModelRepair::repairPolygon(std::vector<Point_3>& points, std::vector<std::vector<std::size_t> >& polygons, Mesh& mesh) {
    statusCode(1, (char*)"Merge duplicate points and faces, remove invalid polygons and remove isolated points.");
    log("- Before reparation, the soup has %d vertices and %d faces.\n", points.size(), polygons.size());
    PMP::repair_polygon_soup(points, polygons, NP::erase_all_duplicates(false).require_same_orientation(false));

    log("- After reparation, the soup has %d vertices and %d faces.\n", points.size(), polygons.size());

    statusCode(2, (char*)"Orient polygon soup.");
    orientPolygon(points, polygons);

    log("- Building a polygon mesh from a soup of polygons.\n");
    PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh, NP::outward_orientation(true));

    if (CGAL::is_closed(mesh)) {
        log("- mesh is cloesd, orients the connected components of tm to make it bound a volume.\n");
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

bool ModelRepair::checkBorder(Mesh mesh) {
    std::unordered_set<halfedge_descriptor> hedge_handled;
    for(halfedge_descriptor h : halfedges(mesh)) {
        if (is_border(h, mesh)) {
            return true;
        }
    }
    return false;
}


void ModelRepair::repairBorders(Mesh& mesh) {
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

void ModelRepair::calculateFactor(DataGroup& data_group, Mesh mesh) {
    auto machine_width = data_group.settings.get<double>("machine_width");
    auto machine_depth = data_group.settings.get<double>("machine_depth");
    auto machine_height = data_group.settings.get<double>("machine_height");

    CGAL::Bbox_3 box = PMP::bbox(mesh);
    double mesh_width = box.xmax() - box.xmin();
    double mesh_depth = box.ymax() - box.ymin();
    double mesh_height = box.zmax() - box.zmin();

    hole_factor = std::min(machine_width/mesh_width, std::min(machine_height/mesh_height, machine_depth/mesh_depth));
}

double getArea(std::vector<Point_3> points)
{
    if (points.size() < 3) {
        return 0;
    }
    double area = 0;
    Point_3 p1 = points[0];
    Point_3 p2 = points[1];
    Point_3 p3 = points[2];

    if (points.size() < 3) {
        double i = (p1.y() - p2.y()) * (p1.z() - p3.z()) - (p1.y() - p3.y()) * (p1.z() - p2.z());
        double j = (p1.x() - p3.x()) * (p1.z() - p2.z()) - (p1.x() - p2.x()) * (p1.z() - p3.z());
        double k = (p1.x() - p2.x()) * (p1.y() - p3.y()) - (p1.x() - p3.x()) * (p1.y() - p2.y());
        area = std::pow((i + j + k), 1/2);
    }
    else {
        Point_3 p0 = points[points.size() - 1];

        double a = std::pow(((p2.y()-p1.y())*(p3.z()-p1.z())-(p3.y()-p1.y())*(p2.z()-p1.z())),2)
                   + std::pow(((p3.x()-p1.x())*(p2.z()-p1.z())-(p2.x()-p1.x())*(p3.z()-p1.z())),2)
                   + std::pow(((p2.x()-p1.x())*(p3.y()-p1.y())-(p3.x()-p1.x())*(p2.y()-p1.y())),2);

        double cosnx = ((p2.y()-p1.y())*(p3.z()-p1.z())-(p3.y()-p1.y())*(p2.z()-p1.z())) / (std::pow(a,1/2));
        double cosny = ((p3.x()-p1.x())*(p2.z()-p1.z())-(p2.x()-p1.x())*(p3.z()-p1.z())) / (std::pow(a,1/2));
        double cosnz = ((p2.x()-p1.x())*(p3.y()-p1.y())-(p3.x()-p1.x())*(p2.y()-p1.y())) / (std::pow(a,1/2));

        area = cosnz*(p0.x()*p1.y()-p1.x()*p0.y()) + cosnx*(p0.y()*p1.z()-p1.y()*p0.z()) + cosny*(p2.z()*p1.x()-p1.z()*p0.x());

        for (int j = 0; j < points.size()-1; j++) {
            Point_3 pj1 = points[j];
            Point_3 pj2 = points[j+1];
            area += cosnz *((pj1.x())*(pj2.y())-(pj2.x())*(pj1.y()))
                    + cosnx*((pj1.y())*(pj2.z())-(pj2.y())*(pj1.z()))
                    + cosny*((pj1.z())*(pj2.x())-(pj2.z())*(pj1.x()));
        }
    }
    return std::abs(area) * 0.5;
}

bool isSmallHole(halfedge_descriptor h, Mesh& mesh, double max_hole_diam, int max_num_hole_edges) {
    int num_hole_edges = 0;
    CGAL::Bbox_3 hole_bbox;
    std::vector<Point_3> hole_points;
    for (halfedge_descriptor hc : CGAL::halfedges_around_face(h, mesh)) {
        const Point_3& p = mesh.point(target(hc, mesh));
        hole_points.push_back(p);
        hole_bbox += p.bbox();
        ++num_hole_edges;

        if (num_hole_edges < 3) {
            continue;
        }

        double area = getArea(hole_points);
//        std::cout << "area: " << area << "\n";
        if (area > max_hole_diam) {
            return false;
        }
    }
    return true;
}

void ModelRepair::repairHoleOfDiameter(Mesh& mesh) {
    // Both of these must be positive in order to be considered
    unsigned int nb_holes = 0;
    std::vector<halfedge_descriptor> border_cycles;
    // collect one halfedge per boundary cycle
    PMP::extract_boundary_cycles(mesh, std::back_inserter(border_cycles));
    nb_holes = border_cycles.size();
    int success_fill = 0;
    max_hole_diam /= (hole_factor * hole_factor / 2);
    for (halfedge_descriptor h : border_cycles) {
        bool success;
        std::vector<face_descriptor> patch_facets;
        std::vector<vertex_descriptor> patch_vertices;
        if (!isSmallHole(h, mesh, max_hole_diam, max_num_hole_edges))  {
            success = std::get<0>(PMP::triangulate_refine_and_fair_hole(mesh, h,
                                                                             std::back_inserter(patch_facets),
                                                                             std::back_inserter(patch_vertices)));
            if (success) {
                success_fill++;
            }
        }
        else {
            PMP::triangulate_hole(mesh, h, std::back_inserter(patch_facets));
            success_fill++;
        }
    }
    log("- The mesh has %d holes. successfully fill %d holes.\n", nb_holes, success_fill);
}

bool ModelRepair::isSelfIntersect(Mesh& mesh) {
    bool intersecting = PMP::does_self_intersect<CGAL::Parallel_if_available_tag>(mesh, NP::vertex_point_map(get(CGAL::vertex_point, mesh)));
    if (intersecting) {
        log("- There are self-intersections.\n");
        return true;
    } else {
        log("- There is no self-intersection.\n");
        return false;
    }
}

void ModelRepair::repairSelfIntersect(Mesh& mesh) {
    std::vector<std::pair<face_descriptor, face_descriptor> > intersected_tris;
    PMP::self_intersections<CGAL::Parallel_if_available_tag>(faces(mesh), mesh, std::back_inserter(intersected_tris));
    log("- %d pairs of triangles intersect.\n", intersected_tris.size());
}

void ModelRepair::repairModel(std::vector<Point_3>& points, std::vector<std::vector<std::size_t> >& polygons, Mesh& mesh, DataGroup& data_group) {

    statusCode(0, (char*)"Repair polygon soup.");
    if (PMP::is_polygon_soup_a_polygon_mesh(polygons)) {
        log("- The polygon soup is a polygon mesh.\n");
        if (orientPolygon(points, polygons)) {
            log("- The polygon soup is not a polygon mesh.\n");
        }

        PMP::polygon_soup_to_polygon_mesh(points, polygons, mesh, NP::outward_orientation(true));

        if (CGAL::is_closed(mesh)) {
            PMP::orient_to_bound_a_volume(mesh, NP::outward_orientation(true));
            PMP::orient_to_bound_a_volume(mesh, NP::outward_orientation(true));
        }
    } else {
        repairPolygon(points, polygons, mesh);
    }
    repair_basic_time = t.time();
    t.reset();

    if (non_manifold_edge || non_manifold_vertex) {
        statusCode(3, (char*)"Fix manifoldness.");
        t.reset();
        repairManifoldness(mesh);
        repair_manifoldness_time = t.time();
        t.reset();
    }

    if (checkBorder(mesh)) {
        statusCode(4, (char*)"Repair open borders.");
        t.reset();
        repairBorders(mesh);
        repair_borders_time = t.time();
        t.reset();
    }

    if (checkBorder(mesh)) {
        statusCode(5, (char*)"Repair holes.");
        calculateFactor(data_group, mesh);
        repairHoleOfDiameter(mesh);
        if(!PMP::is_outward_oriented(mesh)) {
            PMP::orient(mesh);
        }
        repair_holes_time = t.time();
        t.reset();
    }

    if (isSelfIntersect(mesh)) {
        statusCode(6, (char*)"Repair self_intersect faces.");
        t.reset();
        repairSelfIntersect(mesh);
        repair_self_intersect_time = t.time();
        t.reset();
    }
    repair_time = repair_basic_time + repair_manifoldness_time + repair_borders_time + repair_holes_time + repair_self_intersect_time;
    log("Repair Time: %.3f\n", repair_time);
}

void ModelRepair::repairModel(std::string input_file, std::string output_file, DataGroup& data_group) {
    std::vector<Point_3> points;
    std::vector<std::vector<std::size_t> > polygons;
    Mesh mesh;
    t.start();
    if (!readPolygonSoup(input_file, points, polygons)) {
        exit(2);
    }
    read_file_time = t.time();
    log("Read Time: %.3f\n", read_file_time);
    t.reset();
    repairModel(points, polygons, mesh, data_group);
    repair_time = repair_basic_time + repair_manifoldness_time + repair_borders_time + repair_holes_time + repair_self_intersect_time;

    writePolygon(output_file, mesh);
    statusCode(7, (char*)"End.");
}

void ModelRepair::test() {
    const std::string file_name = CGAL::data_file_path("E:/Datasets/modelrepair/078.stl");

    std::vector<Point_3> points;
    std::vector<std::vector<std::size_t> > polygons;
    Mesh mesh;
    readPolygonSoup(file_name, points, polygons);

//    repairModel(points, polygons, mesh);

    const std::string output_file = "E:/Datasets/modelrepair/out_demo/078.stl";
    writePolygon(output_file, mesh);
}

}  // namespace lunarmp

#endif  // LUNARMP_POLYGONMESHREPAIR_H
