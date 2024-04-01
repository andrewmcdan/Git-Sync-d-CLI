#include "log.h"

namespace LOGGER {
std::mutex Log::log_mutex;
std::vector<std::string> Log::log;
bool Log::printToStdout = false;
std::vector<std::function<void(std::string)>> Log::log_functions;
std::vector<int> Log::log_function_priorities;
bool Log::logWritten = false;

Log::Log()
{
    if (!readLog()) {
        std::cout << "Failed to read log file. Continuing." << std::endl;
    }
    if (log.size() > 25000) {
        std::cout << "Log file is too large. Truncating." << std::endl;
        log.erase(log.begin(), log.end() - 25000);
    }
}

Log::Log(bool printToStdout) : Log()
{
    Log::printToStdout = printToStdout;
}

Log::~Log()
{
    if (!Log::logWritten) {
        if (!Log::writeLog()) {
            std::cout << "Failed to write log file. Continuing." << std::endl;
        }
    }
}

void Log::addLogEntry(std::string log, int priority)
{
    std::lock_guard<std::mutex> lock(Log::log_mutex);
    log = getCurrentDateTime() + " \t " + log;
    if (Log::printToStdout) {
        std::cout << log << std::endl;
    }
    Log::log.push_back(log);
    for (size_t i = 0; i < Log::log_functions.size(); i++) {
        if (Log::log_function_priorities[i] <= priority)
            Log::log_functions[i](log);
    }
}

void Log::addLogEntry(std::string entry)
{
    Log::addLogEntry(entry, 0);
}

std::string Log::getLast()
{
    std::lock_guard<std::mutex> lock(Log::log_mutex);
    return Log::log.back();
}

bool Log::writeLog()
{
    std::cout << "Writing log file" << std::endl;
    std::lock_guard<std::mutex> lock(Log::log_mutex);
    std::ofstream logFile;
    try {
        logFile.open("log.txt", std::ios::out);
    } catch (std::exception& e) {
        std::cout << "Failed to open log file: " << e.what() << std::endl;
        return false;
    }
    if (logFile.is_open()) {
        try {
            for (auto& line : Log::log) {
                logFile << line << std::endl;
            }
            logFile.close();
        } catch (std::exception& e) {
            std::cout << "Failed to write log file: " << e.what() << std::endl;
            return false;
        }
        Log::logWritten = true;
        return true;
    } else {
        return false;
    }
}

bool Log::readLog()
{
    std::lock_guard<std::mutex> lock(Log::log_mutex);
    std::ifstream logFile;
    try {
        logFile.open("log.txt", std::ios::in);
    } catch (std::exception& e) {
        std::cout << "Failed to open log file: " << e.what() << std::endl;
        return false;
    }
    if (logFile.is_open()) {
        try {
            std::string line;
            while (std::getline(logFile, line)) {
                Log::log.push_back(line);
            }
            logFile.close();
        } catch (std::exception& e) {
            std::cout << "Failed to read log file: " << e.what() << std::endl;
            return false;
        }
        return true;
    } else {
        return false;
    }
}

int Log::addLogFunction(std::function<void(std::string)> log_function, int priority)
{
    Log::log_functions.push_back(log_function);
    Log::log_function_priorities.push_back(priority);
    return Log::log_functions.size() - 1;
}

int Log::addLogFunction(std::function<void(std::string)> log_function)
{
    return Log::addLogFunction(log_function, 0);
}

void Log::removeLogFunction(int index)
{
    Log::log_functions.erase(Log::log_functions.begin() + index);
    Log::log_function_priorities.erase(Log::log_function_priorities.begin() + index);
}

std::string getCurrentDateTime()
{
    // Get current time as a time_point
    auto now = std::chrono::system_clock::now();

    // Convert to a time_t object
    auto now_c = std::chrono::system_clock::to_time_t(now);

    // Convert to local time
    std::tm localTime;
#ifdef _MSC_VER
    localtime_s(&localTime, &now_c); // Microsoft version
#else
    localtime_r(&now_c, &localTime); // POSIX version
#endif

    // Format the time
    std::stringstream ss;
    ss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace LOGGER