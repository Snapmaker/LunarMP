// Copyright (c) 2020 Ultimaker B.V.
// LunarTPP is released under the terms of the AGPLv3 or higher.

#include <cerrno>   // error number when trying to read file
#include <cstring>  //For strtok and strcopy.
#include <fstream>  //To check if files exist.
#include <numeric>  //For std::accumulate.
#ifdef _OPENMP
#include <omp.h>  //To change the number of threads to slice with.
#endif            //_OPENMP
#include <rapidjson/filereadstream.h>
#include <rapidjson/rapidjson.h>

#include "../controller/TaskWorker.h"
#include "../utils/Constant.h"
#include "../utils/StringUtils.h"
#include "../utils/logoutput.h"
#include "CommandLine.h"

namespace lunarmp {
CommandLine::CommandLine(const std::vector<std::string>& arguments) : arguments(arguments), last_shown_progress(0) {}

void CommandLine::beginGCode() {}
void CommandLine::flushGCode() {}
bool CommandLine::hasSlice() const { return !arguments.empty(); }

bool CommandLine::isSequential() const {
    return true;  // We have to receive the g-code in sequential order. Start
    // g-code before the rest and so on.
}

void CommandLine::sliceNext() {
    auto* task_worker = TaskWorker::getInstance();
    task_worker->time_keeper.restart();

    task_worker->addTask(arguments[1]);
    Task& task = task_worker->getTaskBack();

    for (size_t argument_index = 2; argument_index < arguments.size(); argument_index++) {
        std::string argument = arguments[argument_index];
        if (argument[0] == '-')  // Starts with "-".
        {
            if (argument[1] == '-')  // Starts with "--".
            {
                if (argument.find("--next") == 0)  // Starts with "--next".
                {
                    try {
                        log("Loaded from disk in %5.3fs\n", task_worker->time_keeper.restart());

                        task_worker->addTask(arguments[1]);
                        task = task_worker->getTaskBack();
                    } catch (...) {
                        // Catch all exceptions.
                        // This prevents the "something went wrong" dialogue on
                        // Windows to pop up on a thrown exception. Only
                        // ClipperLib currently throws exceptions. And only in
                        // the case that it makes an internal error.
                        logError("Unknown exception!\n");
                        exit(1);
                    }
                } else {
                    logError("Unknown option: %s\n", argument.c_str());
                }
            } else  // Starts with "-" but not with "--".
            {
                argument = arguments[argument_index];
                switch (argument[1]) {
                    case 'v': {
                        increaseVerboseLevel();
                        break;
                    }
#ifdef _OPENMP
                    case 'm': {
                        int threads = stoi(argument.substr(2));
                        threads = std::max(1, threads);
                        omp_set_num_threads(threads);
                        break;
                    }
#endif  //_OPENMP
                    case 'p': {
                        enableProgressLogging();
                        break;
                    }
                    case 'j': {
                        argument_index++;
                        if (argument_index >= arguments.size()) {
                            logError("Missing JSON file with -j argument.");
                            exit(1);
                        }
                        argument = arguments[argument_index];
                        if (loadJSONToDataGroup(argument, task.data_group) == FAIL) {
                            logError("Failed to load JSON file: %s\n", argument.c_str());
                            exit(1);
                        }
                        break;
                    }
                    case 'l': {
                        argument_index++;
                        if (argument_index >= arguments.size()) {
                            logError("Missing model file with -l argument.");
                            exit(1);
                        }
                        argument = arguments[argument_index];
                        task.data_group.settings.add("input_path", argument);
                        break;
                    }
                    case 'o': {
                        argument_index++;
                        if (argument_index >= arguments.size()) {
                            logError("Missing output file with -o argument.");
                            exit(1);
                        }
                        argument = arguments[argument_index];
                        task.data_group.settings.add("output_path", argument);
                        break;
                    }
                    default: {
                        logError("Unknown option: -%c\n", argument[1]);
                        //                        Application::getInstance().printCall();
                        //                        Application::getInstance().printHelp();
                        exit(1);
                    }
                }
            }
        } else {
            logError("Unknown option: %s\n", argument.c_str());
            //            Application::getInstance().printCall();
            //            Application::getInstance().printHelp();
            exit(1);
        }
    }

    arguments.clear();  // We've processed all arguments now.

#ifndef DEBUG
    try {
#endif  // DEBUG
        log("Loaded from disk in %5.3fs\n", task_worker->time_keeper.restart());

        while (task_worker->hasNext()) {
            task_worker->runNext();
        }

#ifndef DEBUG
    } catch (...) {
        // Catch all exceptions.
        // This prevents the "something went wrong" dialogue on Windows to pop
        // up on a thrown exception. Only ClipperLib currently throws
        // exceptions. And only in the case that it makes an internal error.
        logError("Unknown exception.\n");
        exit(1);
    }
#endif  // DEBUG

    // Finalize the processor. This adds the end g-code and reports statistics.
    //  FffProcessor::getInstance()->finalize();
}

int CommandLine::loadJSONToDataGroup(const std::string& json_filename, DataGroup& data_group) {
    FILE* file = fopen(json_filename.c_str(), "rb");
    if (!file) {
        logError("Couldn't open JSON file: %s\n", json_filename.c_str());
        return FAIL;
    }

    rapidjson::Document json_document;
    char read_buffer[4096];

    rapidjson::FileReadStream reader_stream(file, read_buffer, sizeof(read_buffer));
    json_document.ParseStream(reader_stream);

    fclose(file);

    if (json_document.HasParseError()) {
        logError("Error parsing settings: 'objects' is null");
        return FAIL;
    }

    Settings& settings = data_group.settings;
    loadJSONToSettings(json_document, settings);

    if (json_document.HasMember("objects")) {
        loadJSONToBaseDatas(json_document["objects"], data_group);
    } else if (json_document.HasMember("data")) {
        loadJSONToBaseDatas(json_document["data"], data_group);
    } else {
        logError("Error parsing JSON (offset %u): %s\n");
        return FAIL;
    }

    return SUCCESS;
}

int CommandLine::loadJSONToBaseDatas(const rapidjson::Value& value, DataGroup& data_group) {
    if (value.IsObject()) {
        loadJSONToBaseData(value, data_group);
    } else {
        const rapidjson::Value::ConstArray& array = value.GetArray();

        for (rapidjson::SizeType kI = 0; kI < array.Size(); ++kI) {
            loadJSONToBaseData(array[kI], data_group);
        }
    }
    return SUCCESS;
}

int CommandLine::loadJSONToBaseData(const rapidjson::Value& value, DataGroup& data_group) {
    Settings settings;
    loadJSONToSettings(value, settings);

    data_group.data.emplace_back(Data(settings, data_group.settings));
    return SUCCESS;
}

int CommandLine::loadJSONToSettings(const rapidjson::Value& object, Settings& settings) {
    for (rapidjson::Value::ConstMemberIterator iterator = object.MemberBegin(); iterator != object.MemberEnd(); iterator++) {
        if (iterator->value.IsObject()) {
            loadJSONToSettings(iterator->value, settings);
        } else if (iterator->value.IsArray()) {
            continue;
        } else {
            std::string key = iterator->name.GetString();
            std::string snake_key = camel2snake(key);
            std::string value = getJSONToString(object, key);
            if (settings.has(key)) {
                logError("Duplicate parameter naming");
            } else {
                settings.add(key, value);
                settings.add(snake_key, value);
            }
        }
    }

    return SUCCESS;
}

std::string CommandLine::getJSONToString(const rapidjson::Value& value, const std::string& key) {
    if (!value.HasMember(key.c_str())) {
        return "";
    }

    rapidjson::Type type = value[key.c_str()].GetType();

    switch (type) {
        case rapidjson::Type::kNumberType:
            return std::to_string(value[key.c_str()].GetDouble());
        case rapidjson::Type::kTrueType:
        case rapidjson::Type::kFalseType:
            return std::to_string(value[key.c_str()].GetBool());
        case rapidjson::kArrayType:
        case rapidjson::kObjectType:
        case rapidjson::kNullType:
            return "";
        default:
            std::string s = value[key.c_str()].GetString();
            return sToLower(s);
    }
}

}  // namespace lunarmp