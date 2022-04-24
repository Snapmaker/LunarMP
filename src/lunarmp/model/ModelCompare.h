//
// Created by Cyril on 2022/4/22.
//

#ifndef LUNARMP_DEPS_SRC_LUNARMP_MODEL_MODELCOMPARE_H_
#define LUNARMP_DEPS_SRC_LUNARMP_MODEL_MODELCOMPARE_H_

namespace lunarmp {

class ModelCompare {
  public:
    double hausdorff_distance = 0;               //! the Hausdorff distance from a mesh tm1 to a mesh tm2.
    double compare_time = 0;

    /*!
     * \brief Read file
     *
     * \param fileName
     * \param mesh
     *
     */
    void readFile(std::string fileName, Mesh& mesh);

    /*!
     * \brief Computes the approximate Hausdorff distance from mesh1 to mesh2
     *
     * \param mesh1 The triangle mesh that will be sampled
     * \param mesh2 The triangle mesh to compute the distance to
     * \param unit Precision control
     *
     */
    void hausdorffDistance(Mesh& mesh1, Mesh& mesh2, int unit);

    /*!
     * \brief Computes max approximate Hausdorff distance from mesh1 to mesh2
     *
     * \param mesh1 The triangle mesh that will be sampled
     * \param mesh2 The triangle mesh to compute the distance to
     * \param error_bound a maximum bound by which the Hausdorff distance estimate is allowed to deviate from the actual Hausdorff distance.
     *
     */
    void boundedErrorSymmetricHausdorffDistance(Mesh& mesh1, Mesh& mesh2, double error_bound);

    /*!
     * \brief Compare model
     *
     * \param baseFile The triangle mesh that will be sampled
     * \param mesh2 The triangle mesh to compute the distance to
     * \param error_bound a maximum bound by which the Hausdorff distance estimate is allowed to deviate from the actual Hausdorff distance.
     *
     */
    void modelCompare(std::string baseFile, Mesh& mesh2, double error_bound);

    /*!
     * \brief Compare model
     *
     * \param base The triangle mesh that will be sampled
     * \param mesh2 The triangle mesh to compute the distance to
     *
     */
    void modelCompare(Mesh& base, Mesh& mesh2, double error_bound);

    /*!
     * \brief test
     */
    void test();
};

}

#endif  // LUNARMP_DEPS_SRC_LUNARMP_MODEL_MODELCOMPARE_H_
