﻿
#include <Windows.h>
#include "Taskbar/CTaskbar.h"
#include "Utils.h"
#include "WinShellFunctionality/ShellHookingFunctionality.h"
#include <CommCtrl.h>
#include <dwmapi.h>
#include "Styling.h"
#include "colors.h"
#include "Services/TaskService.h"
#include <stdlib.h>
#include "Logging/FLogger.h"

#pragma warning(disable : 4996)
#define SC_RESTORE      0xF120
#define SC_MINIMIZE     0xF020


#define WINDOW_CLASS L"LiteBoxDesktopWindoww" 

CTaskbar* taskbar;
HWND hDesktopWnd = NULL;

void setAutostart(int on, int elevate)
{
    // Open key
    HKEY key;
    int error = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, KEY_SET_VALUE, NULL, &key, NULL);
    if (error != ERROR_SUCCESS) {
        MessageBoxA(NULL, ("Error setting autostart regkey " + error), "error", MB_OK);
        return;
    }

    if (on) {
        // Get path
        wchar_t path[MAX_PATH], value[MAX_PATH + 20];
        GetModuleFileName(NULL, path, MAX_PATH);
        swprintf(value, MAX_PATH + 20, L"\"%s\"%s", path, (elevate ? L" -elevate" : L""));
        // Set autostart
        error = RegSetValueEx(key, WINDOW_CLASS, 0, REG_SZ, (LPBYTE)value, (wcslen(value) + 1) * sizeof(value[0]));
        if (error != ERROR_SUCCESS) {
            MessageBoxA(NULL, "Error setting autostart regkey" + error, "error", MB_OK);

            return;
        }
    }
    else {
        // Remove
        error = RegDeleteValue(key, WINDOW_CLASS);
        if (error != ERROR_SUCCESS) {
            MessageBoxA(NULL, "Error deleting autostart regkey " + error, "error", MB_OK);
            return;
        }
    }
    // Close key
    RegCloseKey(key);
}

BOOL isAppRunning()
//The system closes the handle automatically when the process terminates.
{
    if (CreateMutex(NULL, TRUE, L"WINDOW_CLASS") && GetLastError() == ERROR_ALREADY_EXISTS) {
        return TRUE; //application is running
    }

    return FALSE;
}


// the "desktop" window for litebox.  i was override the desktop but decided that i didnt want to at the end.
LRESULT CALLBACK WndProcParent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    FLogger log;
    MSG msg;
    WNDCLASSEX wc;
    LB_Api::main_hinstance = hInstance;

    // Check if the taskbar is already running NULL);
    if (isAppRunning()) // if the HWND is not null then that means that it found the proccess
    {
        MessageBoxA(NULL, "Application is already running!\nClose all of the other instances and try again.", "error!", S_OK);
        return 0;
    }
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProcParent;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WINDOW_CLASS;

    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


    if (!RegisterClassEx(&wc)){
        MessageBoxA(NULL, "Error failed to register window class " + __LINE__, "error", MB_OK);
        return 0;
    }

    HWND hWnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        NULL,
        NULL,
        0, 0, 0, 0,
        HWND_MESSAGE/*GetDesktopWindow()*/,
        NULL,
        NULL,
        NULL
    );

    if(hWnd == NULL){
        MessageBoxA(NULL, "Error failed to create window. " + __LINE__, "error", MB_OK);
        return 0;
    }

    setAutostart(1, 0);
    ShowWindow(hWnd, nCmdShow);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (taskbar != NULL) {
        delete taskbar;
        taskbar = nullptr;
    }

    return msg.wParam;
}

std::wstring getStylePath()
{
    return (LB_Api::getLBExePath() + L"\\defaultStyle.ini");
}

void edit_file(int id)
{

    wchar_t buffer[2 * MAX_PATH];
    swprintf(buffer, 2*MAX_PATH,L"\"%s\" \"%s\"", L"notepad.exe", getStylePath().c_str());
    //LB_Api::executeShell(NULL, NULL, L"notepad.exe", getStylePath().c_str(), LB_Api::getLBExePath().c_str(), 1, 0);
    ShellExecute(NULL, L"edit", getStylePath().c_str(), NULL, LB_Api::getLBExePath().c_str(), SW_SHOW);
}

void runTaskbar()
{
    std::wstring buf = getStylePath();
    std::string str(buf.begin(), buf.end());
    readStyle(str.c_str());


    // create it
    RECT monitorSize = LB_Api::getPrimaryScreenRes();


    LB_Api::hideExplorer();// hides the taskbar

    LB_Api::setWorkArea((m_Style.taskbar_Height + m_Style.border_Width + 2));
    LB_Api::setWorkArea((m_Style.taskbar_Height + m_Style.border_Width + 2));

    taskbar = new CTaskbar();

    if (taskbar->create((monitorSize.right / 2) - (m_Style.taskbar_Width / 2), monitorSize.bottom - (m_Style.taskbar_Height + m_Style.border_Width), hDesktopWnd)
        == false) {

        SendMessage(hDesktopWnd, WM_CLOSE, NULL, NULL); // graceful close?
    }
}

LRESULT CALLBACK WndProcParent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
    {
        hDesktopWnd = hwnd;
        initShellFuncs(hDesktopWnd);

        runTaskbar();
    }
    break;
    case RESTART_TASKBAR: 
    {
        if (taskbar != NULL) {
            delete taskbar;
            taskbar = nullptr;
        }
        runTaskbar();
    }
    break;
    case OPEN_STYLE_FILE:
    {
        edit_file(0);
    }
    break;
    case EXIT_TASKBAR:
    case WM_HOTKEY:
    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    break;
    case WM_DESTROY:
    {
        if (taskbar != NULL) {
            delete taskbar;
            taskbar = NULL;
        }
        exitShellFuncs(hwnd);
        //LB_Api::restartExplorerWindow();
        LB_Api::showExplorer();
        PostQuitMessage(0);
    }
    break;

    default:
    {
        if (msg == WM_ShellHook) {
            if (taskbar != nullptr)
                taskbar->taskService.TaskWndProc(wParam, lParam);
        }
    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
