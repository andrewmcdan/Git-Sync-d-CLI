#include <iostream>
#include <string>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <thread>
#include <boost/interprocess/managed_shared_memory.hpp>

#define SERVICE_CONTROL_USER 128
#define SERVICE_CONTROL_START_CLI (SERVICE_CONTROL_USER + 0)

using namespace boost;

bool IsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID administratorsGroup;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                 &administratorsGroup))
    {
        CheckTokenMembership(NULL, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    }
    return isAdmin == TRUE;
}

void RestartAsAdmin(int argc, char *argv[])
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    SHELLEXECUTEINFOA shExInfo = {};
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_DEFAULT;
    shExInfo.hwnd = 0;
    shExInfo.lpVerb = "runas"; // Run as admin
    shExInfo.lpFile = path;    // Path to current executable
    // fill lpParameters with the command line arguments
    std::string params = "";
    for (int i = 0; i < argc; i++)
    {
        params += argv[i];
        params += " ";
    }
    shExInfo.lpParameters = params.c_str(); // Any parameters
    shExInfo.lpDirectory = 0;
    shExInfo.nShow = SW_NORMAL;

    if (!ShellExecuteExA(&shExInfo))
    {
        DWORD error = GetLastError();
        // Handle error TODO:
    }
}

int main(int argc, char **argv)
{
    std::cout << "GitSyncd - CLI" << std::endl;

    if (!IsAdmin())
    {
        RestartAsAdmin(argc, argv);
        return 0;
    }
    std::cout << "Running as admin" << std::endl;
    LPCSTR serviceName = "GitSyncdService";
    // determine if argv[1] is "cli"
    if (argc > 1 && (std::string(argv[1]) == "cli") || (std::string(argv[1]) == "--cli"))
    {
        // TODO:
        
        
        while (true)
        {
            std::cout << "GitSyncd CLI: ";
            std::string someData = "";
            std::cin >> someData;
            if (someData == "exit")
            {
                break;
            }
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory( &si, sizeof(si) );
            si.cb = sizeof(si);
            ZeroMemory( &pi, sizeof(pi) );

            // Start the child process. 
            if( !CreateProcess( NULL,   // No module name (use command line)
                (LPSTR)someData.c_str(),        // Command line
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                0,              // No creation flags
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory 
                &si,            // Pointer to STARTUPINFO structure
                &pi )           // Pointer to PROCESS_INFORMATION structure
            ) 
            {
                printf( "CreateProcess failed (%d).\n", GetLastError() );
            }

            // Wait until child process exits.
            WaitForSingleObject( pi.hProcess, INFINITE );

            // Close process and thread handles. 
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
        }
    }
    else
    {
        // send signal to service to start CLI as child process
        std::cout << "Sending GitSyncd service command" << std::endl;
        // sleep for a bit
        std::cout << "a Line of text" << std::endl;

        SC_HANDLE scmHandle = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if (!scmHandle)
        {
            std::cout << "OpenSCManager failed" << std::endl;
            return 1;
        }
        std::cout << "OpenSCManager success" << std::endl;
        Sleep(1000);
        SC_HANDLE serviceHandle;
        try
        {
            serviceHandle = OpenServiceA(scmHandle, serviceName, SERVICE_ALL_ACCESS);
        }
        catch (std::error_code e)
        {
            std::cout << "OpenService failed. error: " << e << std::endl;
            Sleep(10000);
            return 1;
        }
        catch (...)
        {
            std::cout << "OpenService failed. error: unknown" << GetLastError() << std::endl;
            Sleep(10000);
            return 1;
        }
        if (!serviceHandle)
        {
            CloseServiceHandle(scmHandle);
            std::cout << "OpenService failed" << std::endl;
            return 1;
        }
        SERVICE_STATUS_PROCESS serviceStatus;
        DWORD bytesNeeded;
        if (!QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&serviceStatus, sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded))
        {
            CloseServiceHandle(serviceHandle);
            CloseServiceHandle(scmHandle);
            std::cout << "QueryServiceStatusEx failed" << std::endl;
            return 1;
        }
        if (serviceStatus.dwCurrentState != SERVICE_RUNNING)
        {
            std::cout << "Service is not running" << std::endl;
            CloseServiceHandle(serviceHandle);
            CloseServiceHandle(scmHandle);
            return 1;
        }
        ControlService(serviceHandle, SERVICE_CONTROL_START_CLI, (LPSERVICE_STATUS)&serviceStatus);
        std::cout << "GitSyncd service command sent" << std::endl;
        CloseServiceHandle(serviceHandle);
        CloseServiceHandle(scmHandle);
        std::cout << "Sleeping for 10 seconds" << std::endl;
        Sleep(10000);
    }
    typedef std::pair<int, std::string> command;
    std::vector<command> commands;
    return 0;
}
