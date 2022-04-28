//
// Created by Cyril on 2022/4/28.
//

#ifndef LUNARMP_SRC_LUNARMP_MODEL_MODELTEST_H_
#define LUNARMP_SRC_LUNARMP_MODEL_MODELTEST_H_

#include <io.h>

#include "ModelCheck.h"
#include "ModelCompare.h"
#include "ModelRepair.h"
#include "ModelSimplification.h"

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
