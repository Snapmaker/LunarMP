#ifndef LUBANENGINE_SRC_LUBAN_CONTROLLER_TASK_H_
#define LUBANENGINE_SRC_LUBAN_CONTROLLER_TASK_H_

#include <utility>

#include "../data/DataGroup.h"
#include "../utils/Enums.h"
#include "../utils/TimeKeeper.h"

namespace lunarmp {

class Task {

public:
    std::string cmd;

    DataGroup data_group;

    TimeKeeper time_keeper;

    explicit Task(std::string cmd) : cmd(cmd) {};

    Task(std::string cmd, DataGroup& data_group) : cmd(std::move(cmd)), data_group(data_group) {};
};

}

#endif //LUBANENGINE_SRC_LUBAN_CONTROLLER_TASK_H_
