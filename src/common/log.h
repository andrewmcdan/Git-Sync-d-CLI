#pragma once
#ifndef _LOG_H_
#define _LOG_H_

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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace LOGGER {
class Log
{
public:
    Log();
    Log(bool printToStdout);
    ~Log();
    static int addLogFunction(std::function<void(std::string)> log_function, int priority);
    static int addLogFunction(std::function<void(std::string)> log_function);
    static void removeLogFunction(int index);
    static void addLogEntry(std::string log);
    static void addLogEntry(std::string log, int priority);
    static std::string getLast();
    static bool writeLog();
private:
    static bool printToStdout;
    static std::mutex log_mutex;
    static std::vector<std::string> log;
    static bool readLog();
    static std::vector<std::function<void(std::string)>> log_functions;
    static std::vector<int> log_function_priorities;
    static bool logWritten;
};

std::string getCurrentDateTime();
}
#endif // !_LOG_H_