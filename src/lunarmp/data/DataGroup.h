//
// Created by zjiefee on 2022/3/17.
//

#ifndef LUBANENGINE_SRC_LUBAN_DATA_DATAGROUP_H_
#define LUBANENGINE_SRC_LUBAN_DATA_DATAGROUP_H_

#include <string>

#include "../settings/Settings.h"

namespace lunarmp {

class Data {
  public:
    Settings settings;

    Data(Settings& settings) : settings(settings) {}

    Data(Settings& settings, Settings& parent_settings) : settings(settings) { this->settings.setParent(&parent_settings); }

    std::string getInputFile(const std::string& file_name) {
        return this->settings.get<std::string>("input_dir") + "/" + this->settings.get<std::string>(file_name);
    };

    std::string getOutputFile(const std::string& file_name) {
        return this->settings.get<std::string>("output_dir") + "/" + this->settings.get<std::string>(file_name);
    };
};

class DataGroup {
  public:
    Settings settings;
    std::vector<Data> data;

    virtual void pushData(Settings& data_settings) {
        data_settings.setParent(&this->settings);
        this->data.emplace_back(Data(data_settings));
    }
};

}  // namespace lunarmp

#endif  // LUBANENGINE_SRC_LUBAN_DATA_DATAGROUP_H_
