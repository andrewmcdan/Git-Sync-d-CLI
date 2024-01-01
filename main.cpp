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

#ifdef _WIN32
#include <WinSock2.h>
#include <boost/asio.hpp>
#include <windows.h>
#include <Aclapi.h>
#include <Sddl.h>
#include <errors.h>
#endif

#include <boost/asio.hpp>

#define USE_BOOST_ASIO
#if defined(BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE)
#include <boost/asio/windows/stream_handle.hpp>
#elif defined(BOOST_ASIO_HAS_LOCAL_STREAM_PROTOCOL)
#include <boost/asio/local/stream_protocol.hpp>
#endif

using namespace boost::asio;

int main(int argc, char **argv)
{
    std::cout << "GitSyncd - CLI" << std::endl;
    // determine if argv[1] is "cli"
    if (argc == 1)
    {

        std::string pipeName = "\\\\.\\pipe\\git-sync-d";

#ifdef BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE
        HANDLE pipe_handle = INVALID_HANDLE_VALUE;
        do{
            std::cout << "setting up boost asio" << std::endl;
            SECURITY_ATTRIBUTES sa;
            SECURITY_DESCRIPTOR *pSD;
            PSECURITY_DESCRIPTOR pSDDL;
            // Define the SDDL for the security descriptor
            // This SDDL string specifies that the pipe is open to Everyone
            // D: DACL, A: Allow, GA: Generic All, S-1-1-0: SID string for "Everyone"
            LPCSTR szSDDL = "D:(A;;GA;;;S-1-1-0)";
            if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
                    szSDDL, SDDL_REVISION_1, &pSDDL, NULL))
            {
                std::cout << "Failed to convert SDDL: " << GetLastError() << std::endl;
            }
            pSD = (SECURITY_DESCRIPTOR *)pSDDL;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = pSD;
            sa.bInheritHandle = FALSE;
            pipe_handle = CreateFileA(
                pipeName.c_str(),             // name of the pipe
                GENERIC_READ | GENERIC_WRITE, // two way communication
                0,
                &sa,                  // default security attributes
                OPEN_EXISTING,        // opens existing pipe
                FILE_FLAG_OVERLAPPED, // default attributes
                NULL                  // no template file
            );
            if (pipe_handle == INVALID_HANDLE_VALUE)
            {
                std::cout << "Failed to open pipe: " << GetLastError() << std::endl;
                Sleep(1000);
            }
        }
        while(pipe_handle == INVALID_HANDLE_VALUE);
        std::cout << "Opened pipe handle" << std::endl;
        boost::system::error_code ec;
        boost::asio::io_service io_service;
        boost::asio::windows::stream_handle pipe(io_service, pipe_handle);
        std::cout << "Created pipe" << std::endl;
        bool stop_io_service = false;
        std::thread t([&]()
            {
                // std::cout << "Running io_service" << std::endl;
                while (!stop_io_service)
                {
                    io_service.run(ec);
                    if (ec)
                    {
                        std::cout << "Failed to run io_service: " << ec.message() << std::endl;
                        Sleep(10);
                    }

                } });

        std::vector<char> buf(1024);
        std::cout << "Writing to pipe" << std::endl;
        for (size_t i = 0; i < 1000; i++)
        {
            pipe.async_write_some(boost::asio::buffer("Hello from client", 17), [&](const boost::system::error_code &error, std::size_t bytes_transferred)
                { 
                    // std::cout << "Wrote " << bytes_transferred << " bytes" << std::endl; 
                    if(ec){
                        std::cout << "Failed to write to pipe: " << ec.message() << std::endl;
                    }
                });
        }
        for (int i = 0; i < 1000; i++)
        {
            pipe.write_some(boost::asio::buffer("Hello from client. non-async.", 30), ec);
            if (ec)
            {
                std::cout << "Failed to write to pipe: " << ec.message() << std::endl;
            }
        }
        std::cout << "Wrote to pipe" << std::endl;
        std::cout << "Sleeping for 1 seconds" << std::endl;
        Sleep(1000);
#endif // BOOST_ASIO_HAS_WINDOWS_STREAM_HANDLE
        // std::cout << "Closed pipe" << std::endl;
        pipe.close(ec);
        if(ec){
            std::cout << "Failed to close pipe: " << ec.message() << std::endl;
        }
        CloseHandle(pipe.native_handle());
        
        std::cout << "Sleeping for 2 seconds" << std::endl;
        stop_io_service = true;
        t.join();
        // Sleep(20000);
    }
    else if (argc == 2 && (strcmp(argv[1], "cli") == 0) || (strcmp(argv[1], "--cli") == 0))
    {
        // TODO: start CLI
        while (true)
        {
            std::cout << "GitSyncd CLI: ";
            std::string someData = "";
            std::cin >> someData;
            if (someData == "exit")
            {
                break;
            }
        }
    }
    // std::cout << "Exiting" << std::endl;
    // Sleep(2000);
    typedef std::pair<int, std::string> command;
    std::vector<command> commands(10);
    commands.push_back(std::make_pair(1, "Hello"));
    commands.push_back(std::make_pair(2, "World"));
    for (auto &cmd : commands)
    {
        std::cout << cmd.first << ": " << cmd.second << std::endl;
    }
    std::cout << "Done" << std::endl;

    return 0;
}
