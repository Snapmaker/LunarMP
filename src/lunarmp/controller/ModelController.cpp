#include "ModelController.h"

#include <string>

#include "../model/ModelCheck.h"
#include "../model/ModelRepair.h"
#include "../model/ModelCompare.h"
#include "../model/ModelSimplification.h"

namespace lunarmp {

void ModelController::repair(DataGroup& data_group) {
    auto input_path = data_group.settings.get<std::string>("input_path");
    auto output_path = data_group.settings.get<std::string>("output_path");

    ModelRepair model_repair;
    model_repair.repairModel(input_path, output_path);
}

void ModelController::check(DataGroup& data_group) {
    auto input_path = data_group.settings.get<std::string>("input_path");

    ModelCheck model_check;
    model_check.checkModel(input_path);
}

void ModelController::simplification(DataGroup& data_group) {
    auto input_path = data_group.settings.get<std::string>("input_path");
    auto output_path = data_group.settings.get<std::string>("output_path");
    auto simplify_type = data_group.settings.get<int>("simplify_type");
    auto simplify_stop_predicate_threshold = data_group.settings.get<double>("simplify_stop_predicate_threshold");

    ModelSimplification model_simplification;
    model_simplification.modelSimplification(input_path, output_path, simplify_type, simplify_stop_predicate_threshold);
}


void ModelController::compare(DataGroup& data_group) {
    auto input_path = data_group.settings.get<std::string>("input_path");
    auto output_path = data_group.settings.get<std::string>("output_path");

    ModelCompare model_compare;
    model_compare.modelCompare(input_path, output_path, 0.01);
}

}  // namespace lunarmp
