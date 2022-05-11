// Copyright (c) 2018 Ultimaker B.V.
// LunarTPP is released under the terms of the AGPLv3 or higher.

#ifdef _OPENMP
#include <omp.h>  // omp_get_num_threads
#endif            // _OPENMP
#include <iostream>
#include <string>

#include "Application.h"
#include "communication/CommandLine.h"
#include "settings/Settings.h"
#include "utils/Constant.h"
#include "utils/SMProgress.h"
#include "utils/StringUtils.h"  //For stringcasecompare.
#include "utils/logoutput.h"

namespace lunarmp {

Application::Application() : communication(nullptr), current_slice(0) {}

Application::~Application() { delete communication; }

Application& Application::getInstance() {
    static Application instance;  // Constructs using the default constructor.
    return instance;
}

void Application::printVersion() const {
    logAlways("\n");
    logAlways("Luban_Engine version %s\n", VERSION);
}

void Application::printParallelTest() const {
#pragma omp parallel for
    for (int kI = 0; kI < 10; ++kI) {
        std::cout << kI << std::endl;
    }
}

void Application::printLicense() const {
    logAlways("\n");
    logAlways(
        "This program is free software: you can redistribute it and/or "
        "modify\n");
    logAlways(
        "it under the terms of the GNU Affero General Public License as "
        "published by\n");
    logAlways("the Free Software Foundation, either version 3 of the License, or\n");
    logAlways("(at your option) any later version.\n");
    logAlways("\n");
    logAlways("This program is distributed in the hope that it will be useful,\n");
    logAlways("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    logAlways("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    logAlways("GNU Affero General Public License for more details.\n");
    logAlways("\n");
    logAlways(
        "You should have received a copy of the GNU Affero General Public "
        "License\n");
    logAlways(
        "along with this program.  If not, see "
        "<http://www.gnu.org/licenses/>.\n");
}

void Application::modelRepair() {
    std::vector<std::string> arguments;
    for (size_t argument_index = 0; argument_index < argc; argument_index++) {
        arguments.emplace_back(argv[argument_index]);
    }

    communication = new CommandLine(arguments);
}

void Application::run(const size_t argc, char** argv) {
    this->argc = argc;
    this->argv = argv;

    //    printLicense();
    SMProgress::init();

    if (argc < 2) {
        //        printHelp();
        exit(1);
    }

#pragma omp parallel
    {
#pragma omp master
        {
#ifdef _OPENMP
            log("OpenMP multithreading enabled, likely number of threads to be "
                "used: %u\n",
                omp_get_num_threads());
#else
            log("OpenMP multithreading disabled\n");
#endif
        }
    }
    if (stringcasecompare(argv[1], "modelrepair") == SUCCESS) {
        modelRepair();
    } else if (stringcasecompare(argv[1], "help") == 0) {
        //        printHelp();
    } else if (stringcasecompare(argv[1], "version") == 0) {
        printVersion();
    } else if (stringcasecompare(argv[1], "ptest") == 0) {
        printParallelTest();
    } else {
        logError("Unknown command: %s\n", argv[1]);
        //        printCall();
        //        printHelp();
        exit(1);
    }

    if (!communication) {
        // No communication channel is open any more, so either:
        //- communication failed to connect, or
        //- the engine was called with an unknown command or a command that
        // doesn't connect (like "help"). In either case, we don't want to slice.
        exit(0);
    }
    while (communication->hasSlice()) {
        communication->sliceNext();
    }
}

}  // namespace lunarmp