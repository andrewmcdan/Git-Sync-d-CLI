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
};


#endif // !_IPC_H_