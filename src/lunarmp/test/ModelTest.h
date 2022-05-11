//
// Created by Cyril on 2022/4/28.
//

#ifndef LUNARMP_SRC_LUNARMP_MODEL_MODELTEST_H_
#define LUNARMP_SRC_LUNARMP_MODEL_MODELTEST_H_

#include <string>

#include "../model/ModelCheck.h"
#include "../model/ModelCompare.h"
#include "../model/ModelRepair.h"
#include "../model/ModelSimplification.h"

namespace lunarmp {

class ModelTest {
  public:
    void GetSpecialFilesFromDirectory(std::string path, std::string fileType, std::vector<std::string>& files);
    void GetAllFilesIncludeSubfolder(std::string path, std::string fileType, std::vector<std::string>& files);
    int getFileSize(std::string filepath);
    int testOutput();
    int simpfyOutput(double mSize);
    int simpfyOutput();
};

}  // namespace lunarmp

#endif  // LUNARMP_SRC_LUNARMP_MODEL_MODELTEST_H_
