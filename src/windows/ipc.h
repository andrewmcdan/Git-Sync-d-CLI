#pragma once
#ifndef _IPC_H_
#define _IPC_H_
#include <iostream>
#include <string>
#include <tchar.h>
#include <stdio.h>
#include <thread>
#include <vector>
#include <utility>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <WinSock2.h>
#include <boost/asio.hpp>
#include <windows.h>
#include <Aclapi.h>
#include <Sddl.h>
#include <errors.h>
#include <filesystem>
#include <tlhelp32.h>

#include "../common/log.h"

#define USE_BOOST_ASIO
#include <boost/asio/windows/stream_handle.hpp>

#define START_PATTERN_STRING "zL`93O5d"
#define END_PATTERN_STRING "oY>U093Z"

using namespace boost::asio;

enum COMMAND_CODE {
    COMMAND_ADD_FILE,
    COMMAND_REMOVE_SYNC,
    COMMAND_ADD_CREDENTIALS,
    COMMAND_REMOVE_CREDENTIALS,
    COMMAND_ADD_REMOTE_REPO,
    COMMAND_REMOVE_REMOTE_REPO,
    COMMAND_ADD_DIRECTORY,
    COMMAND_ADD_REMOTE_REPO_CREDENTIALS,
    COMMAND_REMOVE_REMOTE_REPO_CREDENTIALS,
    COMMAND_TRIGGER_SYNC,
    COMMAND_TRIGGER_SYNC_ALL,
    COMMAND_TRIGGER_SYNC_ALL_REPOS,
    COMMAND_TRIGGER_SYNC_ALL_CREDENTIALS,
    COMMAND_TRIGGER_SYNC_ALL_DIRECTORIES,
    COMMAND_TRIGGER_SYNC_ALL_FILES,
    COMMAND_TRIGGER_SYNC_ALL_FILES_AND_DIRECTORIES,
    COMMAND_TRIGGER_SYNC_ALL_FILES_AND_DIRECTORIES_FOR_REPO,
    COMMAND_TRIGGER_SYNC_ALL_FILES_AND_DIRECTORIES_FOR_CREDENTIALS,
    COMMAND_TRIGGER_SYNC_ALL_FILES_AND_DIRECTORIES_FOR_REPO_AND_CREDENTIALS,
    COMMAND_TRIGGER_SYNC_ALL_FILES_AND_DIRECTORIES_FOR_REPO_AND_CREDENTIALS_AND_DIRECTORY,
    COMMAND_KILL_GIT_SYNC_D
};

enum RESPONSE_CODE {
    RESP_SUCCESS,
    RESP_ERROR,
    RESP_UNKNOWN,
    RESP_NOT_IMPLEMENTED,
    RESP_INVALID_COMMAND,
    RESP_INVALID_DATA
};

enum SYNC_TYPE {
    SYNC_TYPE_ALL = 0,
    SYNC_TYPE_REPO = 1,
    SYNC_TYPE_TIME_FRAME = 2,
    SYNC_TYPE_DIRECTORY = 4,
    SYNC_TYPE_FILE = 8,
    SYNC_TYPE_UNDEFINED = 256
};

struct PipeMessage
{
    COMMAND_CODE command;
    std::string data;
};

class IPC
{
public:
    IPC();
    ~IPC();

private:
    bool launchGitSyncd();
    std::thread io_service_thread;
    std::thread reader_thread;
    std::thread writer_thread;
    bool IPC::IsProcessRunning(const char* processName);
    bool stop_services;
};


#endif // !_IPC_H_