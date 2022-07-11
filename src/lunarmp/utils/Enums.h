//
// Created by zjiefee on 2021/3/26.
//

#ifndef LUBANENGINE_SRC_SETTINGS_ENUMSETTINGS_H_
#define LUBANENGINE_SRC_SETTINGS_ENUMSETTINGS_H_

namespace lunarmp {
enum class HeadType {
    PRINT = 0,
    LASER,
    CNC
};

enum class ToolpathType {
    IMAGE = 0,
    VECTOR = 1,
    SCULPT = 2
};

enum class ProcessMode {
    BW,
    GREYSCALE,
    VECTOR,
    HALF_TONE
};

enum class MovementMode {
    POINT,
    LINE
};

enum class SliceMode {
    ROTATION,
    LINKAGE
};

enum class EnumCommand {
    COMMENT,
    COMMAND,
    G0,
    G1,
    G4
};

enum class TraverseType {
    VERTICAL,
    HORIZONTAL,
    DIAGONAL,
    DIAGONAL2
};

enum class SerpentineAlgorithm {
    ATKINSON,
    BURKES,
    FLOYDSTEINBURG,
    JARVISJUDICENINKE,
    SIERRA2,
    SIERRA3,
    SIERRALITE,
    STUCKI
};

enum class FaceDirection {
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

enum class ToolpathMode {
    NORMAL,
    XTOB
};

enum class HalftoneType {
    LINE,
    ROUND,
    DIAMOND
};

enum class SVGPathType {
    PATH,
    OUTLINE,
    FILL
};

enum class TaskType {
    TOOLPATH = 0,
    PROCESS,
    SUPPORT
};

enum class SimplifyType
{
    EDGE_LENGTH_STOP,
    EDGE_COUNT_STOP,
    EDGE_RATIO_STOP
};
} // namespace lunarmp

#endif // LUBANENGINE_SRC_SETTINGS_ENUMSETTINGS_H_
