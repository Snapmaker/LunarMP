//
// Created by zjiefee on 2021/3/30.
//
#include <limits>

#ifndef LUBANENGINE_SRC_SETTINGS_CONSTANT_H_
#define LUBANENGINE_SRC_SETTINGS_CONSTANT_H_

#define LASER_BLACK -1
#define LASER_WHITE 1

#define TOOL_MAX_ANGLE 60

#define TOOLPATH_P_NULL std::numeric_limits<float>::max()

#define TOOLPATH_MAX_B_SPEED 2400.0f

#define TOOLPATH_MAX_B_VALUE 720000

#define IMAGE_16_RANGE 65535.0f
#define IMAGE_8_RANGE 255.0f

#define SUCCESS 0
#define FAIL -1

#endif // LUBANENGINE_SRC_SETTINGS_CONSTANT_H_
