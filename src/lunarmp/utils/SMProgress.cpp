// Copyright (c) 2018 Ultimaker B.V.
// LunarTPP is released under the terms of the AGPLv3 or higher.

#include "SMProgress.h"

#include <cassert>

#include "../Application.h"                  //To get the communication channel to send progress through.
#include "../communication/Communication.h"  //To send progress through the communication channel.
#include "TimeKeeper.h"
#include "logoutput.h"

namespace lunarmp {
std::vector<double> SMProgress::sm_times = std::vector<double>();

std::vector<double> SMProgress::sm_accumulated_times = std::vector<double>();

std::string SMProgress::sm_names[] = {"start", "model", "toolpath", "export", "finish"};
float SMProgress::stage_progress_starts[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
float SMProgress::stage_progress_ends[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

double SMProgress::sm_last_progress = 0;
double SMProgress::sm_total_timing = -1;

float SMProgress::calcOverallProgress(Stage stage, float stage_progress) {
    if (sm_times.empty()) {
        return stage_progress;
    }
    assert(stage_progress <= 1.0);
    assert(stage_progress >= 0.0);
    return (sm_accumulated_times.back() + stage_progress * sm_times.back()) / sm_total_timing;
}

void SMProgress::init() {
    sm_times.clear();
    sm_accumulated_times.clear();
}

// void SMProgress::messageProgress(SMProgress::Stage stage, int
// progress_in_stage, int progress_in_stage_max) {
//  float percentage = calcOverallProgress(stage, float(progress_in_stage) /
//  float(progress_in_stage_max));
//
//  Application::getInstance().communication->sendProgress(percentage);
//
//  logProgress(sm_names[(int) stage].c_str(), progress_in_stage,
//  progress_in_stage_max, percentage);
//}

void SMProgress::messageProgress(SMProgress::Stage stage, int progress_in_stage, int progress_in_stage_max) {
    float stage_progress_start = stage_progress_starts[(int)stage];
    float stage_progress_end = stage_progress_ends[(int)stage];

    float percentage = (stage_progress_start + (float)progress_in_stage / (float)progress_in_stage_max * (stage_progress_end - stage_progress_start)) / 100;

    if (percentage <= 1 && percentage - sm_last_progress > 0.005) {
        sm_last_progress = percentage;
//        Application::getInstance().communication->sendProgress(percentage);

        logProgress(sm_names[(int)stage].c_str(), progress_in_stage, progress_in_stage_max, percentage);
    }
}

// void SMProgress::messageProgressStage(SMProgress::Stage stage, TimeKeeper
// *time_keeper, float time) {
//  if (time_keeper) {
//    if ((int) stage > 0) {
//      if (sm_accumulated_times.empty()) {
//        sm_accumulated_times.emplace_back(0);
//      } else {
//        sm_accumulated_times.emplace_back(sm_accumulated_times.back() +
//        sm_times.back());
//      }
//      sm_times.emplace_back(time);
//
//      log("Progress: %s accomplished in %5.3fs\n", sm_names[(int) stage -
//      1].c_str(), time_keeper->restart());
//    } else {
//      sm_total_timing = time;
//      time_keeper->restart();
//    }
//
//    if ((int) stage < (int) Stage::FINISH) {
//      log("Starting %s...\n", sm_names[(int) stage].c_str());
//    } else {
//      SMProgress::init();
//    }
//  }
//}

void SMProgress::messageProgressStage(SMProgress::Stage stage, TimeKeeper* time_keeper, float stage_progress_start, float stage_progress_end) {
    stage_progress_starts[(int)stage] = stage_progress_start;
    stage_progress_ends[(int)stage] = stage_progress_end;
    sm_last_progress = 0;
    if (time_keeper) {
        if ((int)stage > 0) {
            log("Progress: %s accomplished in %5.3fs\n", sm_names[(int)stage - 1].c_str(), time_keeper->restart());
        } else {
            time_keeper->restart();
        }

        if ((int)stage < (int)Stage::FINISH) {
            log("Starting %s...\n", sm_names[(int)stage].c_str());
        } else {
            logProgress(sm_names[(int)stage].c_str(), 100, 100, 1);
        }
    }
}

}  // namespace lunarmp