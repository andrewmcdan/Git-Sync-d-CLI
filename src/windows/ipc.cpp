#include "ipc.h"
namespace INTERPROCESS {
unsigned int IPC::slot_counter = 0;
bool IPC::stop_services = false;
IPC::IPC()
{
    this->isShutdown = false;
    this->runner_thread = std::thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        LOGGER::Log::addLogEntry("Starting IPC services");
        std::string pipeName = GIT_SYNCD_PIPENAME;
        int tryOpenPipeCount = 0;
        pipe_handle = INVALID_HANDLE_VALUE;
        do {
            tryOpenPipeCount++;
            LOGGER::Log::addLogEntry("setting up boost asio");
            SECURITY_ATTRIBUTES sa;
            SECURITY_DESCRIPTOR* pSD;
            PSECURITY_DESCRIPTOR pSDDL;
            // Define the SDDL for the security descriptor
            // This SDDL string specifies that the pipe is open to Everyone
            // D: DACL, A: Allow, GA: Generic All, S-1-1-0: SID string for "Everyone"
            LPCSTR szSDDL = "D:(A;;GA;;;S-1-1-0)";
            if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
                    szSDDL, SDDL_REVISION_1, &pSDDL, NULL)) {
                LOGGER::Log::addLogEntry("Failed to convert SDDL: " + std::to_string(GetLastError()));
            }
            pSD = (SECURITY_DESCRIPTOR*)pSDDL;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = pSD;
            sa.bInheritHandle = FALSE;
            pipe_handle = CreateFileA(pipeName.c_str(), // name of the pipe
                GENERIC_READ | GENERIC_WRITE, // two way communication
                0,
                &sa, // default security attributes
                OPEN_EXISTING, // opens existing pipe
                FILE_FLAG_OVERLAPPED, // default attributes
                NULL // no template file
            );
            if (pipe_handle == INVALID_HANDLE_VALUE) {
                LOGGER::Log::addLogEntry("Failed to open pipe: " + std::to_string(GetLastError()));
                LOGGER::Log::addLogEntry("Attempting to launch git-sync-d");
                if (!launchGitSyncd()) {
                    LOGGER::Log::addLogEntry("Failed to launch git-sync-d");
                }
                Sleep(1000);
            }
        } while (pipe_handle == INVALID_HANDLE_VALUE && tryOpenPipeCount < 10);
        if (pipe_handle == INVALID_HANDLE_VALUE) {
            std::cout << "Unable to open pipe. Check the logs for more info." << std::endl;
            return;
        }
        LOGGER::Log::addLogEntry("Opened pipe handle");
        boost::system::error_code ec;
        boost::asio::windows::stream_handle pipe(this->io_service, pipe_handle);
        LOGGER::Log::addLogEntry("Created pipe");
        union {
            char c[4];
            unsigned int i;
        } slot;
        union {
            char c[4];
            unsigned int i;
        } command;
        union {
            char c[4];
            unsigned int i;
        } dataLength;
        union {
            char c[4];
            unsigned int i;
        } totalLength;
        totalLength.i = 0;
        this->io_service_thread = std::thread([&]() {
            while (!IPC::stop_services) {
                this->io_service.run(ec);
                if (ec)
                    LOGGER::Log::addLogEntry("Failed to run io_service: " + ec.message());
                Sleep(10);
            }
        });
        std::vector<char> buffer_vector(1024 * 16);
        this->reader_thread = std::thread([&]() {
            while (!IPC::stop_services) {
                pipe.async_read_some(
                    boost::asio::buffer(buffer_vector.data(), buffer_vector.size()),
                    [&](const boost::system::error_code& ec,
                        std::size_t bytes_transferred) {
                        if (ec) {
                            LOGGER::Log::addLogEntry("Failed to read from pipe:\n\tec value: " + std::to_string(ec.value()) + "\n\tec message: " + ec.message());
                        } else if (bytes_transferred > 0) {
                            std::string s(buffer_vector.begin(), buffer_vector.begin() + bytes_transferred);
                            LOGGER::Log::addLogEntry("Read " + std::to_string(bytes_transferred) + " bytes from pipe");
                            LOGGER::Log::addLogEntry(s);
                            union {
                                char c[sizeof(size_t)];
                                size_t i;
                            } responseSize;
                            responseSize.i = 0;
                            memcpy(responseSize.c, buffer_vector.data(), sizeof(size_t));
                            LOGGER::Log::addLogEntry("Response size: " + std::to_string(responseSize.i));
                        }
                    });
                Sleep(100);
            }
        });
        this->writer_thread = std::thread([&](){
            while(!IPC::stop_services){
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
        std::string testData = "file:O:/Projects/MidJourneyAutomation/user.json\n";
        testData += "destRepository:Git-Sync-d-TestData\n";
        testData += "directory:test1\n";
        union {
            char c[4];
            unsigned int i;
        } syncType_un;
        syncType_un.i = 2;
        testData += "syncType:" + std::string(syncType_un.c, 4) + "\n";
        testData += "syncTimeFrame:1h\n";

        dataLength.i = (int)testData.length();
        slot.i = 0;
        totalLength.i = (int)testData.length() + 16;
        command.i = 0;

        
        for (size_t i = 0; i < 50; i++) {
            LOGGER::Log::addLogEntry("Writing to pipe");
            std::string stringToSend = START_PATTERN_STRING + std::string(totalLength.c, 4) + std::string(dataLength.c, 4) + std::string(slot.c, 4) + std::string(command.c, 4) + testData + END_PATTERN_STRING;
            LOGGER::Log::addLogEntry("Sending: " + stringToSend);
            LOGGER::Log::addLogEntry("Sending: " + std::to_string(stringToSend.length()) + " bytes");
            pipe.write_some(
                boost::asio::buffer(stringToSend.c_str(), stringToSend.length()), ec);
            if (ec) {
                LOGGER::Log::addLogEntry("Failed to write to pipe:\n\tec value: " + std::to_string(ec.value()) + "\n\tec message: " + ec.message());
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        for (int i = 0; i < 1; i++) {
            pipe.write_some(boost::asio::buffer("Hello from client. non-async. ", 29),
                ec);
            if (ec) {
                LOGGER::Log::addLogEntry("Failed to write to pipe:\n\tec value: " + std::to_string(ec.value()) + "\n\tec message: " + ec.message());
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        LOGGER::Log::addLogEntry("Wrote to pipe");
        while (!IPC::stop_services) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        LOGGER::Log::addLogEntry("Stopping IPC services");
        std::cout << "Stopping IPC services 1" << std::endl;
        this->reader_thread.join();
        std::cout << "Stopping IPC services 2" << std::endl;
        this->writer_thread.join();
        std::cout << "Stopping IPC services 3" << std::endl;
        this->io_service_thread.join();
        std::cout << "Stopping IPC services 4" << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        pipe.close(ec);
        if (ec) {
            LOGGER::Log::addLogEntry("Failed to close pipe:\n\tec value: " + std::to_string(ec.value()) + "\n\tec message: " + ec.message());
        }
        CloseHandle(pipe.native_handle());
        this->isShutdown = true;
    });
}

IPC::~IPC()
{
    std::cout << "IPC destructor" << std::endl;
    IPC::stop_services = true;
    std::cout << "IPC destructor 5" << std::endl;
    if(this->runner_thread.joinable())
        this->runner_thread.join();
    std::cout << "IPC destructor 7" << std::endl;
}

void IPC::shutdown()
{
    IPC::stop_services = true;
}

bool IPC::getIsShutdown()
{
    return this->isShutdown;
}

bool IPC::launchGitSyncd()
{
    // get this executable's path
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string gitSyncdPath = path;
    gitSyncdPath = gitSyncdPath.substr(0, gitSyncdPath.find_last_of("\\/"));
    gitSyncdPath += "\\Git-Sync-d.exe";
    if (!std::filesystem::exists(gitSyncdPath)) {
        LOGGER::Log::addLogEntry("git-sync-d.exe not found at: " + gitSyncdPath);
        return false;
    }
    if (IsProcessRunning("Git-Sync-d.exe") || IsProcessRunning("git-sync-d.exe")) {
        LOGGER::Log::addLogEntry("git-sync-d.exe is already running");
        return false;
    }
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    std::string commandLine = gitSyncdPath + " -C";
    if (!CreateProcessA(NULL, // No module name (use command line)
            (LPSTR)commandLine.c_str(), // Command line
            NULL, // Process handle not inheritable
            NULL, // Thread handle not inheritable
            FALSE, // Set handle inheritance to FALSE
            DETACHED_PROCESS, // No creation flags
            NULL, // Use parent's environment block
            NULL, // Use parent's starting directory
            &si, // Pointer to STARTUPINFO structure
            &pi) // Pointer to PROCESS_INFORMATION structure
    ) {
        LOGGER::Log::addLogEntry("Failed to launch git-sync-d: " + std::to_string(GetLastError()));
        return false;
    }
    return true;
}

bool IPC::IsProcessRunning(const char* processName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (strcmp(pe32.szExeFile, processName) == 0) {
                    CloseHandle(hSnapshot);
                    return true;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return false;
}

bool IPC::hasPendingResponse()
{
    return this->pending_responses.size() > 0;
}

PipeResponse IPC::getResponse()
{
    // testing
    return {
        RESP_SUCCESS,
        COMMAND_ADD_FILE,
        0,
        "test"
    };
}

void IPC::sendCommand(COMMAND_CODE command, std::string data)
{
    this->pending_messages_to_send.push_back({
        command,
        IPC::slot_counter++,
        data
    });
    return;
}
} // namespace INTERPROCESS