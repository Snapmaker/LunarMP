#ifndef LUNARMP_SRC_LUNARMP_CONTROLLER_MODELCONTROLLER_H_
#define LUNARMP_SRC_LUNARMP_CONTROLLER_MODELCONTROLLER_H_

#include "../data/DataGroup.h"

namespace lunarmp {

class ModelController {
  public:
    void repair(DataGroup& data_group);
    void check(DataGroup& data_group);
    void simplification(DataGroup& data_group);
    void compare(DataGroup& data_group);
};

}  // namespace lunarmp

#endif  // LUNARMP_SRC_LUNARMP_CONTROLLER_MODELCONTROLLER_H_
