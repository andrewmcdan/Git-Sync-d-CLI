#include <filesystem>
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
#include "src/common/log.h"
#include "src/include/lyra/lyra.hpp"

int main(int argc, char** argv)
{
    struct CL_config {
        bool cli_enabled = false;
        bool show_help = false;
        std::string addFile_fileName = "";
        std::string addFile_repoName = "";
        bool addFile = false;
        std::string addRemote_repoName = "";
        bool showRemoteRepos = false;
        bool addRemote = false;
        bool triggerSync_all = false;
        bool printToStdout = false;
    } commandLine_config;
    std::function<bool(std::string const&)> fileExists = [](std::string const& s) {
        return std::filesystem::exists(s);
    };
    auto cli_parser = lyra::help(commandLine_config.show_help).description("Git-Sync'd - Command Line Interface")
        | lyra::opt(commandLine_config.cli_enabled)["-C"]["--cli"]("Starts the CLI")
        | lyra::opt(commandLine_config.printToStdout)["-p"]["--print"]("Prints log entries to stdout")
        | lyra::opt(commandLine_config.showRemoteRepos)["-l"]["--list-remote"]("Lists all remote repos")
        | lyra::opt(commandLine_config.triggerSync_all)["-s"]["--sync"]("Syncs all repos");

    cli_parser.add_argument(lyra::group([&](const lyra::group& g) { commandLine_config.addFile = true; })
                                .add_argument(lyra::opt(commandLine_config.addFile_fileName, "file|folder")["-a"]["--add"]("Add a file or folder to a repo").choices(fileExists)))
        .add_argument(lyra::opt(commandLine_config.addFile_repoName, "repo")["-r"]["--repo"]("Specify a repo to operate on"));

    cli_parser.add_argument(lyra::group([&](const lyra::group& g) { commandLine_config.addRemote = true; })
                                .add_argument(lyra::opt(commandLine_config.addFile_fileName, "repo")["-e"]["--add-remote"]("Add a remote repo as a sync destination. i.e. https://github.com/owner/project  git://host.xyz/path/to/repo.git etc.").choices(fileExists)))
        .add_argument(lyra::opt(commandLine_config.addFile_repoName, "credential")["-c"]["--cred"]("Specify a credential to use for this repo.  Use <service:username> for saved credentials, <service:username:password>|<token>|<ssh-key>|<ssh-key-path> for new credentials."));

    auto result = cli_parser.parse({ argc, argv });
    if (!result) {
        std::cerr << "Error in command line. Use -h or --help." << std::endl;
        return 1;
    } else if (commandLine_config.show_help) {
        // print usage
        std::cout << cli_parser << std::endl;
        return 0;
    }

    if(commandLine_config.printToStdout && commandLine_config.cli_enabled){
        std::cout << "Cannot use -p|--print with -C|--cli" << std::endl;
        return 1;
    }

    LOGGER::Log log(commandLine_config.printToStdout);
    LOGGER::Log::addLogEntry("Git-Sync'd starting");

    {
        INTERPROCESS::IPC ipc;

        if (commandLine_config.cli_enabled) {
            CLI cli = CLI();
            std::cout << "Git-Sync'd - Command Line Interface is shutting down..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        } else {
            std::cout << "Git-Sync'd" << std::endl;
            // here we do the things that the command line options specified
        }
        ipc.shutdown();
        while(!ipc.getIsShutdown()){
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::cout << "About to write log file" << std::endl;
    if(!LOGGER::Log::writeLog()){
        std::cout << "Failed to write log file" << std::endl;
    }
    std::cout << "Git-Sync'd is exiting..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    return 0;
}
