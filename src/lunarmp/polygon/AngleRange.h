//
// Created by Cyril on 2022/7/26.
//

#ifndef LUNARMP_SRC_LUNARMP_UTILS_ANGLERANGE_H_
#define LUNARMP_SRC_LUNARMP_UTILS_ANGLERANGE_H_

namespace lunarmp {
class AngleRange {
  public:
    int start;
    int end;

    AngleRange() {};
    AngleRange(int st, int ed):start(st),end(ed) {};

    void init() {
        start = normalize(start);
        end = normalize(end);
    }

    int normalize(int angle) {
        while (angle < 0) {
            angle += 360;
        }
        while (angle >= 360) {
            angle -= 360;
        }
        return angle;
    }

    int get_range() {
        return normalize(end - start);
    }

    bool between(int angle) {
        if(get_range() >= 180) {
            return false;
        }
        angle = normalize(angle);
        if (start <= end) {
            return angle >= start && angle <= end;
        }
        else {
            return angle >= start || angle <= end;
        }
    }
};

}


#endif  // LUNARMP_SRC_LUNARMP_UTILS_ANGLERANGE_H_
