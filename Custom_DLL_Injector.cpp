#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <iostream>

#define IDC_DLL_BUTTON    101
#define IDC_PROC_COMBO   102
#define IDC_INJECT_BTN   103

char selectedDllPath[MAX_PATH] = "";

DWORD GetProcessIdByName(const char* processName) {
    DWORD processId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe)) {
            do {
                if (_stricmp(pe.szExeFile, processName) == 0) {
                    processId = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        CloseHandle(hSnapshot);
    }
    return processId;
}

bool InjectDLL(DWORD processId, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) return false;

    LPVOID allocMem = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!allocMem) {
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, allocMem, dllPath, strlen(dllPath) + 1, NULL)) {
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, allocMem, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}

void SelectDLL(HWND hwnd) {
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "DLL Files\0*.dll\0";
    ofn.lpstrFile = selectedDllPath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;

    GetOpenFileName(&ofn);
}

void PopulateProcessList(HWND hwndCombo) {
    SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnap, &pe)) {
            do {
                SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)pe.szExeFile);
            } while (Process32Next(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hDllBtn, hProcCombo, hInjectBtn;

    switch (msg) {
    case WM_CREATE:
        hDllBtn = CreateWindow("BUTTON", "Select DLL", WS_VISIBLE | WS_CHILD, 20, 20, 100, 30, hwnd, (HMENU)IDC_DLL_BUTTON, NULL, NULL);
        hProcCombo = CreateWindow("COMBOBOX", NULL, CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD | WS_VSCROLL, 140, 20, 250, 300, hwnd, (HMENU)IDC_PROC_COMBO, NULL, NULL);
        hInjectBtn = CreateWindow("BUTTON", "Inject", WS_VISIBLE | WS_CHILD, 400, 20, 100, 30, hwnd, (HMENU)IDC_INJECT_BTN, NULL, NULL);
        PopulateProcessList(hProcCombo);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_DLL_BUTTON:
            SelectDLL(hwnd);
            break;
        case IDC_INJECT_BTN: {
            char selectedProc[260];
            GetWindowText(hProcCombo, selectedProc, sizeof(selectedProc));
            DWORD pid = GetProcessIdByName(selectedProc);

            if (pid && strlen(selectedDllPath) > 0) {
                if (InjectDLL(pid, selectedDllPath)) {
                    MessageBox(hwnd, "DLL injected successfully!", "Success", MB_OK);
                }
                else {
                    MessageBox(hwnd, "Injection failed.", "Error", MB_OK);
                }
            }
            else {
                MessageBox(hwnd, "Select a DLL and a process first.", "Warning", MB_OK);
            }
            break;
        }
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "DLLInjectorWindow";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow("DLLInjectorWindow", "DLL Injector", WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 550, 120, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
