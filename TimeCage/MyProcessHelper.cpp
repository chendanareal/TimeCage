#include "pch.h"
#include "MyProcessHelper.h"

#define MAX_PROCESS 2048

// 用于枚举窗口的上下文结构体
typedef struct
{
    DWORD dwTargetPID;    // 目标PID
    HWND hMainWnd;        // 找到的主窗口
    CList<HWND, HWND>* pWndList; // 存储所有窗口的列表
} EnumWndContext;


CMyProcessHelper::CMyProcessHelper(void)
{
}

CMyProcessHelper::~CMyProcessHelper(void)
{
}


CString CMyProcessHelper::GetProcessName(DWORD dwPID)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, dwPID);
    if (hProcess == NULL)
    {
        return _T("(访问受限)");
    }

    TCHAR szPath[MAX_PATH] = { 0 };
    if (GetProcessImageFileName(hProcess, szPath, MAX_PATH))
    {
        CString strPath = szPath;
        int nPos = strPath.ReverseFind(_T('\\'));
        if (nPos != -1)
        {
            CloseHandle(hProcess);
            return strPath.Mid(nPos + 1);
        }
        CloseHandle(hProcess);
        return strPath;
    }

    CloseHandle(hProcess);
    return _T("");
}


// 窗口枚举回调函数（静态成员，用于EnumWindows）
BOOL CALLBACK CMyProcessHelper::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    EnumWndContext* pContext = (EnumWndContext*)lParam;
    if (!pContext) return TRUE;

    DWORD dwProcessID = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessID);

    // 匹配目标PID
    if (dwProcessID == pContext->dwTargetPID)
    {

        // 过滤条件：窗口可见、不是子窗口（排除控件、寄生窗口）
        //if (!IsIconic(hWnd) && GetAncestor(hWnd, GA_PARENT) == GetDesktopWindow())
        if (GetAncestor(hWnd, GA_PARENT) == GetDesktopWindow())
        {
            RECT rect;

            // 获取窗口在屏幕上的矩形区域
            GetWindowRect(hWnd, &rect);

            if (((rect.right - rect.left) > 0)
                && ((rect.bottom - rect.top) > 0))
            {
                // 找到主窗口，直接记录并停止枚举
                pContext->hMainWnd = hWnd;

                TRACE(_T("HWND = 0x%08X width=%d height=%d %d\r\n"), hWnd, rect.right - rect.left, rect.bottom - rect.top, IsWindowVisible(hWnd));

                return FALSE;
            }
        }

        // 如果是获取所有窗口，直接添加到列表
        if (pContext->pWndList != nullptr)
        {
            pContext->pWndList->AddTail(hWnd);
        }
    }

    return TRUE;
}

// 窗口枚举回调函数（静态成员，用于EnumWindows）
BOOL CALLBACK CMyProcessHelper::EnumMultiMainWindowsProc(HWND hWnd, LPARAM lParam)
{
    EnumWndContext* pContext = (EnumWndContext*)lParam;
    if (!pContext) return TRUE;

    DWORD dwProcessID = 0;
    GetWindowThreadProcessId(hWnd, &dwProcessID);

    // 匹配目标PID
    if (dwProcessID == pContext->dwTargetPID)
    {

        // 过滤条件：窗口可见、不是子窗口（排除控件、寄生窗口）
        //if (!IsIconic(hWnd) && GetAncestor(hWnd, GA_PARENT) == GetDesktopWindow())
        if (GetAncestor(hWnd, GA_PARENT) == GetDesktopWindow())
        {
            RECT rect;

            // 获取窗口在屏幕上的矩形区域
            GetWindowRect(hWnd, &rect);

            if (((rect.right - rect.left) > 0)
                && ((rect.bottom - rect.top) > 0))
            {
                // 找到主窗口，直接记录并停止枚举
                pContext->hMainWnd = hWnd;

                TRACE(_T("HWND = 0x%08X width=%d height=%d %d\r\n"), hWnd, rect.right - rect.left, rect.bottom - rect.top, IsWindowVisible(hWnd));

                // 如果是获取所有窗口，直接添加到列表
                if (pContext->pWndList != nullptr)
                {
                    pContext->pWndList->AddTail(hWnd);
                }
            }
        }
    }

    return TRUE;
}

// 根据PID获取进程主窗口句柄
HWND CMyProcessHelper::GetMainWindowByPID(DWORD dwPID)
{
    if (dwPID == 0) return NULL;

    EnumWndContext context = { 0 };
    context.dwTargetPID = dwPID;
    context.hMainWnd = NULL;
    context.pWndList = nullptr;

    // 枚举所有顶层窗口
    EnumWindows(EnumWindowsProc, (LPARAM)&context);

    return context.hMainWnd;
}

// 根据PID获取进程主窗口句柄列表
void CMyProcessHelper::CMyProcessHelper::GetMainWindowListByPID(DWORD dwPID, CList<HWND, HWND>& lstHWnd)
{
    lstHWnd.RemoveAll();

    if (dwPID == 0) return;

    EnumWndContext context = { 0 };
    context.dwTargetPID = dwPID;
    context.hMainWnd = NULL;
    context.pWndList = new CList<HWND, HWND>;

    // 枚举所有顶层窗口
    EnumWindows(EnumMultiMainWindowsProc, (LPARAM)&context);

    POSITION pos;

    pos = context.pWndList->GetHeadPosition();
    while (pos)
    {
        HWND hWnd = context.pWndList->GetNext(pos);
        
        lstHWnd.AddTail(hWnd);
    }

    delete context.pWndList;
    context.pWndList = NULL;

}

// ==============================================
// 根据窗口句柄获取窗口标题
// ==============================================
CString CMyProcessHelper::GetWindowTitle(HWND hWnd)
{
    if (!IsWindow(hWnd))
        return _T("");

    CString strTitle;
    int len = GetWindowTextLength(hWnd);
    if (len > 0)
    {
        GetWindowText(hWnd, strTitle.GetBuffer(len + 1), len + 1);
        strTitle.ReleaseBuffer();
    }
    else
    {
        strTitle = _T("");
    }
    return strTitle;
}

typedef LONG(NTAPI* NtSuspendProcess)(IN HANDLE ProcessHandle);

BOOL CMyProcessHelper::SuspendProcess(DWORD nPID, BOOL bHideWindow, BOOL bHideWindowExMode) {

    if (bHideWindow)
    {
        if (!bHideWindowExMode)
        {
            //隐藏窗口
            HWND hWndMain = GetMainWindowByPID(nPID);

            if (hWndMain)
            {
                ShowWindow(hWndMain, FALSE);
            }
        }
        else
        {
            CList<HWND, HWND> lstWnd;

            GetMainWindowListByPID(nPID, lstWnd);

            POSITION pos = lstWnd.GetHeadPosition();
            while (pos)
            {
                HWND hWndMain = lstWnd.GetNext(pos);
                if (hWndMain)
                {
                    ShowWindow(hWndMain, FALSE);
                }
            }
        }

    }


    HANDLE procHandle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, nPID);
    if (!procHandle)
    {
        return FALSE;
    }

    //挂起进程
    NtSuspendProcess pfnProcess = (NtSuspendProcess)GetProcAddress(GetModuleHandle(L"ntdll"), "NtSuspendProcess");

    if (!pfnProcess)
    {
        CloseHandle(procHandle);
        return FALSE;
    }

    pfnProcess(procHandle);
    CloseHandle(procHandle);



    return TRUE;
}


typedef LONG(NTAPI* NtResumeProcess)(IN HANDLE ProcessHandle);
BOOL CMyProcessHelper::ResumeProcess(DWORD nPID, BOOL bHideWindow, BOOL bHideWindowExMode) {

    HANDLE procHandle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, nPID);
    if (!procHandle)
    {
        return FALSE;
    }

    //恢复进程
    NtResumeProcess pfnProcess = (NtResumeProcess)GetProcAddress(GetModuleHandle(L"ntdll"), "NtResumeProcess");
    if (!pfnProcess)
    {
        CloseHandle(procHandle);
        return FALSE;
    }

    pfnProcess(procHandle);
    CloseHandle(procHandle);

    if (bHideWindow)
    {
        if (!bHideWindowExMode)
        {
            //显示窗口
            HWND hWndMain = GetMainWindowByPID(nPID);

            if (hWndMain)
            {
                ShowWindow(hWndMain, TRUE);
            }
        }
        else
        {
            CList<HWND, HWND> lstWnd;

            GetMainWindowListByPID(nPID, lstWnd);

            POSITION pos = lstWnd.GetHeadPosition();
            while (pos)
            {
                HWND hWndMain = lstWnd.GetNext(pos);
                if (hWndMain)
                {
                    ShowWindow(hWndMain, TRUE);
                }
            }
        }

    }


    return TRUE;
}

// 功能：根据进程名获取进程ID
// 参数：strProcessName = 进程名（如 notepad.exe、calc.exe）
// 返回：找到返回 PID，没找到返回 0
DWORD CMyProcessHelper::GetProcessIdByName(const CString& strProcessName)
{
    // 1. 创建进程快照
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // 2. 遍历进程
    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            // 不区分大小写比较进程名
            if (_tcsicmp(pe32.szExeFile, strProcessName) == 0)
            {
                DWORD pid = pe32.th32ProcessID;
                CloseHandle(hSnapshot);
                return pid; // 找到，返回PID
            }

        } while (Process32Next(hSnapshot, &pe32));
    }

    // 3. 没找到
    CloseHandle(hSnapshot);
    return 0;
}
