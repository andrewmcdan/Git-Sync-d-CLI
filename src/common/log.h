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
    static void addLog(std::string log);
    static std::string getLast();
private:
    static bool printToStdout;
    static std::mutex log_mutex;
    static std::vector<std::string> log;
    bool writeLog();
    bool readLog();
};
std::string getCurrentDateTime();
}
#endif // !_LOG_H_