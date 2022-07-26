//
// Created by Cyril on 2022/7/26.
//

#include "ModelNesting.h"

namespace lunarmp {

void getFiles(std::string folder_path, std::vector<std::string> folder) {

}

bool ModelNesting::readFileList(std::string folder_path) {

}

void ModelNesting::modelNesting(std::string input_file, std::string output_file, DataGroup& data_group) {
    // get data_group


    readFileList(input_file);
    startNFP();
    writeFile(output_file);
}

}
