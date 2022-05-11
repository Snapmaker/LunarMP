#include "ModelController.h"

#include <string>

#include "../model/ModelRepair.h"

namespace lunarmp {

void ModelController::repair(DataGroup& data_group) {
    auto input_path = data_group.settings.get<std::string>("input_path");
    auto output_path = data_group.settings.get<std::string>("output_path");

    ModelRepair model_repair;
    model_repair.repairModel(input_path, output_path);
}

}  // namespace lunarmp