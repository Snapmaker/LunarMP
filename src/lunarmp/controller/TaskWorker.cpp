//
// Created by zjiefee on 2021/12/21.
//

#include "TaskWorker.h"

#include <utility>

#include "../utils/StringUtils.h"

namespace lunarmp {

TaskWorker TaskWorker::task_worker;

void TaskWorker::addTask(std::string cmd) { tasks.emplace_back(Task(cmd)); }

Task &TaskWorker::getTaskBack() { return tasks.back(); }

bool TaskWorker::hasNext() { return !tasks.empty(); }

void TaskWorker::runNext() {
    Task task = tasks.front();
    tasks.pop_front();

    std::string lower_cmd = sToLower(task.cmd);

    switch (str2int(lower_cmd.c_str())) {
        case str2int("modelrepair"):
            m_model_controller.repair(task.data_group);
            break;
        case str2int("modelsimplify"):
            m_model_controller.simplification(task.data_group);
            break;
        case str2int("modelcheck"):
            m_model_controller.check(task.data_group);
            break;
        case str2int("modelcompare"):
            m_model_controller.compare(task.data_group);
            break;
        case str2int("modelnesting"):
            m_model_controller.nesting(task.data_group);
            break;
    }
}

}  // namespace lunarmp
