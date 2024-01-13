#include "log.h"

namespace LOGGER {
std::mutex Log::log_mutex;
std::vector<std::string> Log::log;
bool Log::printToStdout = false;
Log::Log()
{
    if(!readLog()){
        std::cout << "Failed to read log file. Continuing." << std::endl;
    }
    if(log.size() > 25000){
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
    writeLog();
}

void Log::addLog(std::string log)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    log = getCurrentDateTime() + "\t" + log;
    if (Log::printToStdout) {
        std::cout << log << std::endl;
    }
    Log::log.push_back(log);
}

std::string Log::getLast()
{
    std::lock_guard<std::mutex> lock(log_mutex);
    return Log::log.back();
}

bool Log::writeLog()
{
    std::lock_guard<std::mutex> lock(log_mutex);
    std::ofstream logFile;
    logFile.open("log.txt", std::ios::out);
    if (logFile.is_open()) {
        for (auto& line : Log::log) {
            logFile << line << std::endl;
        }
        logFile.close();
        return true;
    } else {
        return false;
    }
}

bool Log::readLog()
{
    std::lock_guard<std::mutex> lock(log_mutex);
    std::ifstream logFile;
    logFile.open("log.txt", std::ios::in);
    if (logFile.is_open()) {
        std::string line;
        while (std::getline(logFile, line)) {
            Log::log.push_back(line);
        }
        logFile.close();
        return true;
    } else {
        return false;
    }
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