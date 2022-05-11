//
// Created by zjiefee on 2022/3/22.
//

#ifndef LUBANENGINE_SRC_LUBAN_CONTROLLER_TASKWORKER_H_
#define LUBANENGINE_SRC_LUBAN_CONTROLLER_TASKWORKER_H_

#include <deque>
#include <map>

#include "../utils/TimeKeeper.h"
#include "./ModelController.h"
#include "Task.h"

namespace lunarmp {

class TaskWorker {
  private:
    static TaskWorker task_worker;

    ModelController m_model_controller;

    std::deque<Task> tasks;

  public:
    TimeKeeper time_keeper;

    static TaskWorker* getInstance() { return &task_worker; }

    void addTask(std::string cmd);

    Task& getTaskBack();

    bool hasNext();

    void runNext();
};

}  // namespace lunarmp

#endif  // LUBANENGINE_SRC_LUBAN_CONTROLLER_TASKWORKER_H_