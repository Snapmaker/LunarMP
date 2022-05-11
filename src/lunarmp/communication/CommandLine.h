// Copyright (c) 2018 Ultimaker B.V.
// LunarTPP is released under the terms of the AGPLv3 or higher.

#ifndef CommandLine_H
#define CommandLine_H

#include <rapidjson/document.h>  //Loading JSON documents to get settings from them.

#include <string>  //To store the command line arguments.
#include <unordered_set>
#include <vector>  //To store the command line arguments.

#include "../data/DataGroup.h"
#include "Communication.h"  //The class we're implementing.

namespace lunarmp {
class Settings;

/*
 * \brief When slicing via the command line, interprets the command line
 * arguments to initiate a slice.
 */
class CommandLine : public Communication {
  public:
    /*
     * \brief Construct a new communicator that interprets the command line to
     * start a slice.
     * \param arguments The command line arguments passed to the application.
     */
    CommandLine(const std::vector<std::string>& arguments);

    /*
     * \brief Indicate that we're beginning to send g-code.
     * This does nothing to the command line.
     */
    void beginGCode() override;

    /*
     * \brief Flush all g-code still in the stream into cout.
     */
    void flushGCode() override;

    /*
     * \brief Indicates that for command line output we need to send the g-code
     * from start to finish.
     *
     * We can't go back and erase some g-code very easily.
     */
    bool isSequential() const override;

    /*
     * \brief Test if there are any more slices to be made.
     */
    bool hasSlice() const override;

    /*
     * \brief Slice the next scene that the command line commands us to slice.
     */
    void sliceNext() override;

  private:
    /*
     * \brief The command line arguments that the application was called with.
     */
    std::vector<std::string> arguments;

    /*
     * The last progress update that we output to stdcerr.
     */
    unsigned int last_shown_progress;

    /*
     * \brief Get the default search directories to search for definition files.
     * \return The default search directories to search for definition files.
     */
    std::unordered_set<std::string> defaultSearchDirectories();

    /*
     * \brief Load a JSON file and store the settings inside it.
     * \param json_filename The location of the JSON file to load settings from.
     * \param settings The settings storage to store the settings in.
     * \return Error code. If it's 0, the file was successfully loaded. If it's
     * 1, the file could not be opened. If it's 2, there was a syntax error in
     * the file.
     */
    int loadJSONToDataGroup(const std::string& json_filename, DataGroup& data_group);

    int loadJSONToBaseDatas(const rapidjson::Value& value, DataGroup& data_group);

    int loadJSONToBaseData(const rapidjson::Value& value, DataGroup& data_group);

    int loadJSONToSettings(const rapidjson::Value& value, Settings& settings);

    std::string getJSONToString(const rapidjson::Value& value, const std::string& key);
};

}  // namespace lunarmp

#endif  // CommandLine_H