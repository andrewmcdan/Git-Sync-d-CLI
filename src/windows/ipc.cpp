#include "ipc.h"

IPC::IPC()
{
    std::string pipeName = "\\\\.\\pipe\\git-sync-d";

    HANDLE pipe_handle = INVALID_HANDLE_VALUE;
    do {
        std::cout << "setting up boost asio" << std::endl;
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR* pSD;
        PSECURITY_DESCRIPTOR pSDDL;
        // Define the SDDL for the security descriptor
        // This SDDL string specifies that the pipe is open to Everyone
        // D: DACL, A: Allow, GA: Generic All, S-1-1-0: SID string for "Everyone"
        LPCSTR szSDDL = "D:(A;;GA;;;S-1-1-0)";
        if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
                szSDDL, SDDL_REVISION_1, &pSDDL, NULL)) {
            std::cout << "Failed to convert SDDL: " << GetLastError() << std::endl;
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
            std::cout << "Failed to open pipe: " << GetLastError() << std::endl;
            Sleep(1000);
        }
    } while (pipe_handle == INVALID_HANDLE_VALUE);
    std::cout << "Opened pipe handle" << std::endl;
    boost::system::error_code ec;
    boost::asio::io_service io_service;
    boost::asio::windows::stream_handle pipe(io_service, pipe_handle);
    std::cout << "Created pipe" << std::endl;
    bool stop_io_service = false;
    union {
        char c[4];
        int i;
    } slot;
    union {
        char c[4];
        int i;
    } command;
    union {
        char c[4];
        int i;
    } dataLength;
    union {
        char c[4];
        int i;
    } totalLength;
    totalLength.i = 0;
    std::thread t([&]() {
        // std::cout << "Running io_service" << std::endl;
        while (!stop_io_service) {
            io_service.run(ec);
            if (ec)
                std::cout << "Failed to run io_service: " << ec.message() << std::endl;
            Sleep(10);
        }
    });
    std::vector<char> buffer_vector(1024 * 16);
    std::thread reader([&]() {
        while (!stop_io_service) {
            pipe.async_read_some(
                boost::asio::buffer(buffer_vector.data(), buffer_vector.size()),
                [&](const boost::system::error_code& ec,
                    std::size_t bytes_transferred) {
                    if (ec) {
                        std::cout << "Failed to read from pipe: " << ec.message() << std::endl;
                    } else if (bytes_transferred > 0) {
                        std::cout << "Read " << bytes_transferred << " bytes from pipe" << std::endl;
                        std::string s(buffer_vector.begin(), buffer_vector.begin() + bytes_transferred);
                        std::cout << s << std::endl
                                  << std::endl;
                        union {
                            char c[sizeof(size_t)];
                            size_t i;
                        } responseSize;
                        responseSize.i = 0;
                        memcpy(responseSize.c, buffer_vector.data(), sizeof(size_t));
                        std::cout << "Response size: " << responseSize.i << std::endl;
                    }
                });
            Sleep(100);
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

    std::cout << "Writing to pipe" << std::endl;
    std::string stringToSend = START_PATTERN_STRING + std::string(totalLength.c, 4) + std::string(dataLength.c, 4) + std::string(slot.c, 4) + std::string(command.c, 4) + testData + END_PATTERN_STRING;
    std::cout << "Sending: " << stringToSend << std::endl;
    std::cout << "Sending: " << stringToSend.length() << " bytes" << std::endl;
    for (size_t i = 0; i < 1; i++) {
        pipe.write_some(
            boost::asio::buffer(stringToSend.c_str(), stringToSend.length()), ec);
        if (ec) {
            std::cout << "Failed to write to pipe: " << ec.message() << std::endl;
        }
    }
    for (int i = 0; i < 1; i++) {
        pipe.write_some(boost::asio::buffer("Hello from client. non-async. ", 29),
            ec);
        if (ec) {
            std::cout << "Failed to write to pipe: " << ec.message() << std::endl;
        }
    }
    std::cout << "Wrote to pipe" << std::endl;
    std::cout << "Sleeping for 1 seconds" << std::endl;

    std::cout << "Sleeping for 2 seconds" << std::endl;
    Sleep(5000);

    pipe.close(ec);
    if (ec) {
        std::cout << "Failed to close pipe: " << ec.message() << std::endl;
    }
    CloseHandle(pipe.native_handle());
    stop_io_service = true;
    t.join();
    reader.join();
}

IPC::~IPC() { }