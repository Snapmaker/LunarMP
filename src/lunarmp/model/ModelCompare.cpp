//
// Created by Cyril on 2022/4/22.
//

#include "ModelCompare.h"

namespace lunarmp {

bool ModelCompare::readFile(std::string fileName, Mesh& mesh) {
    if (!PMP::IO::read_polygon_mesh(fileName, mesh))
    {
        log("Invalid input.");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void ModelCompare::boundedErrorSymmetricHausdorffDistance(Mesh& mesh1, Mesh& mesh2, double error_bound)
{
    hausdorff_bounded_error_distance =
        PMP::bounded_error_symmetric_Hausdorff_distance<TAG>(mesh1, mesh2, error_bound);

    log("Approximated Hausdorff Bounded Error Distance: %lf.\n", hausdorff_bounded_error_distance);
}

void ModelCompare::modelCompare(std::string baseFile, std::string compareFile, double error_bound)
{
    Mesh base, mesh2;
    readFile(baseFile, base);
    readFile(compareFile, mesh2);

    CGAL::Timer t;
    t.start();
    boundedErrorSymmetricHausdorffDistance(base, mesh2, error_bound);
    compare_time = t.time();
}

void ModelCompare::modelCompare(Mesh& base, Mesh& mesh2, double error_bound)
{
    CGAL::Timer t;
    t.start();
    boundedErrorSymmetricHausdorffDistance(base, mesh2, error_bound);
    compare_time = t.time();
}

void ModelCompare::test()
{
    Mesh base, simp;
    readFile("D:\\source\\repos\\ConsoleApplication1\\bun.off", base);
    readFile("D:\\source\\repos\\ConsoleApplication1\\bunout.off", simp);

    CGAL::Timer t;
    t.start();
    hausdorffDistance(base, simp, 100);
    log("Unit: 100, (Done: %lf).\n", t.time());

    t.reset();
    hausdorffDistance(base, simp, 4000);
    log("Unit: 4000, (Done: %lf).\n", t.time());
    std::cout << "(Done: " << t.time() << ")" << std::endl;

    //t.reset();
    //hausdorffBoundedErrorDistance(base, simp, 0.01);
    //log("Bound error: 0.01, (Done: %lf).\n", t.time());

    //t.reset();
    //hausdorffBoundedErrorDistance(base, simp, 0.001);
    //log("Bound error: 0.001, (Done: %lf).\n", t.time());

    //t.reset();
    //hausdorffBoundedErrorDistance(base, simp, 0.0001);
    //log("Bound error: 0.0001, (Done: %lf s).\n", t.time());


    t.reset();
    boundedErrorSymmetricHausdorffDistance(base, simp, 0.01);
    log("Bound Symmetric error: 0.01, (Done: %lf).\n", t.time());

    t.reset();
    boundedErrorSymmetricHausdorffDistance(base, simp, 0.001);
    log("Bound Symmetric error: 0.001, (Done: %lf).\n", t.time());

    t.reset();
    boundedErrorSymmetricHausdorffDistance(base, simp, 0.0001);
    log("Bound Symmetric error: 0.0001, (Done: %lf s).\n", t.time());

}

}// namespace lunarmp
