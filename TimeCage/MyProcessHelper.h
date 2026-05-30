#pragma once
#include <afxwin.h>
#include <afxtempl.h>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

// 进程信息结构体
struct ProcessInfo
{
    DWORD dwPID;          // 进程ID
    CString strName;      // 进程名
    double dblCPU;        // CPU 占用率
    HWND hWnd;            // 进程窗口句柄
    CString strCaption;   // 窗口标题
};

// 进程帮助类：获取CPU TOP10
class CMyProcessHelper
{
public:
    CMyProcessHelper(void);
    ~CMyProcessHelper(void);

    // 根据PID获取进程主窗口句柄
    HWND GetMainWindowByPID(DWORD dwPID);

    // 根据PID获取进程主窗口句柄列表
    void GetMainWindowListByPID(DWORD dwPID, CList<HWND, HWND> &lstHWnd);

    // 根据窗口句柄获取窗口标题
    CString GetWindowTitle(HWND hWnd);

    // 挂起进程
    BOOL SuspendProcess(DWORD nPID, BOOL bHideWindow, BOOL bHideWindowExMode);

    // 恢复进程
    BOOL ResumeProcess(DWORD nPID, BOOL bHideWindow, BOOL bHideWindowExMode);

    // 根据进程名取得进程ID
    DWORD GetProcessIdByName(const CString& strProcessName);

private:
    // 获取进程名
    CString GetProcessName(DWORD dwPID);

    // 窗口枚举回调函数（用于获取主窗口/所有窗口）
    static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);

    // 窗口列表枚举回调函数（用于获取主窗口/所有窗口）
    static BOOL CALLBACK EnumMultiMainWindowsProc(HWND hWnd, LPARAM lParam);
};


