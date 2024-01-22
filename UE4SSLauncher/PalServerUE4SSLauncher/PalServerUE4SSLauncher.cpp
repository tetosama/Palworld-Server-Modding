#include <cstdio>
#include <string>
#include <windows.h>
#include <iostream>

int main(int argc, wchar_t* argv[])
{
    std::string ue4ss = "UE4SS.dll";

    STARTUPINFOA* si = (STARTUPINFOA*)calloc(1, sizeof(STARTUPINFOA));
    PROCESS_INFORMATION* pi = (PROCESS_INFORMATION*)calloc(1, sizeof(PROCESS_INFORMATION));
    if (!si || !pi) {
        printf("Failed To Allocate Memory");
        return EXIT_FAILURE;
    }
    si->cb = sizeof(STARTUPINFOA);

    CreateProcessA("PalServer-Win64-Test-Cmd.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, si, pi);
    printf("Process Created, trying to inject dll...\n");

    LPVOID alloc = VirtualAllocEx(pi->hProcess, NULL, ue4ss.size(), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (alloc) {
        int write = WriteProcessMemory(pi->hProcess, alloc, ue4ss.c_str(), ue4ss.size(), NULL);
        if (write != 0) {
            HANDLE thread = CreateRemoteThread(pi->hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, alloc, 0, NULL);
            if (thread) {
                WaitForSingleObject(thread, 5000);
                VirtualFreeEx(pi->hProcess, alloc, 0, MEM_RELEASE);
                printf("DLL Injected");

                ResumeThread(pi->hThread);

                printf("Server Started!");

                WaitForSingleObject(pi->hProcess, INFINITE);
            }
            else {
                printf("Failed To Create Thread");
                VirtualFreeEx(pi->hProcess, alloc, 0, MEM_RELEASE);
            }
        }
        else {
            printf("Failed To Write DLL");
            VirtualFreeEx(pi->hProcess, alloc, 0, MEM_RELEASE);
        }
    }
    else {
        printf("Failed To Allocate Memory");
    }

    CloseHandle(pi->hProcess);
    CloseHandle(pi->hThread);
    free(si);
    free(pi);
    fflush(stdout);
    return EXIT_SUCCESS;
}