/**
 * @file main.cpp
 * @author Andrew McDaniel
 * @brief This is the main entry point for the CLI of Git Sync'd
 * @version 0.1
 * @date 2024-01-11
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <iostream>
#include <stdio.h>
#include <string>
#include <tchar.h>

#ifdef _WIN32
#include "src/windows/ipc.h"
#endif

#if defined(__linux__) || defined(__APPLE__)
#include "src/unix/ipc.h"
#endif

#include "src/common/cli.h"
#include "src/include/lyra/lyra.hpp"

int main(int argc, char** argv)
{
    bool cli_enabled = false;
    bool show_help = false;
    auto cli_parser = lyra::cli()
        | lyra::opt(cli_enabled)["-C"]["--cli"]("Starts the CLI")
        | lyra::opt(show_help)["-h"]["--help"]("Show help");
    auto result = cli_parser.parse({ argc, argv });
    if (!result) {
        // print usage
        std::cerr << "Error in command line: " << result.message() << std::endl;
        return 1;
    }else if (show_help) {
        std::cout << cli_parser << std::endl;
        return 0;
    }

    std::cout << "GitSyncd - CLI" << std::endl;
    // determine if argv[1] is "cli"
    if (argc == 1) {
        IPC ipc;
        // Sleep(20000);
    } else if (cli_enabled) {
        // TODO: start CLI
        CLI cli;
    }
    return 0;
}
