
// WebView2AppDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "TimeCage.h"
#include "TimeCageDlg.h"
#include "afxdialogex.h"
#include "MyProcessHelper.h"
#include "define.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWebView2Dlg 对话框
using namespace Microsoft::WRL;
using namespace std;

CTimeCageDlg::CTimeCageDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_WEBVIEW2APP_DIALOG, pParent)
    , m_bTrayIconCreated(FALSE)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    ZeroMemory(&m_nid, sizeof(NOTIFYICONDATA));
    ZeroMemory(m_arSuspendStatus, sizeof(m_arSuspendStatus));
}

void CTimeCageDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTimeCageDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
    ON_WM_HOTKEY()
    ON_MESSAGE(WM_TRAYMESSAGE, &CTimeCageDlg::OnTrayMessage)
    ON_MESSAGE(RegisterWindowMessage(_T("TaskbarCreated")), &CTimeCageDlg::OnTaskbarCreated)
    ON_COMMAND(ID_MENU_TRAY_SHOW, &CTimeCageDlg::OnMenuTrayShow)
    ON_COMMAND(ID_MENU_TRAY_EXIT, &CTimeCageDlg::OnMenuTrayExit)
END_MESSAGE_MAP()


// CWebView2Dlg 消息处理程序

BOOL CTimeCageDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);            // 设置大图标
    SetIcon(m_hIcon, FALSE);        // 设置小图标

    // TODO: 在此添加额外的初始化代码
    ModifyStyle(WS_CAPTION, 0, SWP_FRAMECHANGED);
    ModifyStyleEx(WS_EX_DLGMODALFRAME, 0);

    // 根据当前 DPI 动态计算窗口尺寸
    int windowWidth = WEBVIEW_WIDTH;
    int windowHeight = WEBVIEW_HEIGHT;
    CalculateWindowSizeForCurrentDPI(windowWidth, windowHeight);
    SetWindowPos(NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);


    SetWindowText(_T("游戏冻结工具TimeCage"));

    CRect rect;
    GetClientRect(&rect);

    // 裁剪窗口
    int w = rect.Width();
    int h = rect.Height();
    int radius = ROUND_CORNER_RADIUS; // 圆角半径，自己调（12~20 比较好看）

    // 创建圆角矩形区域
    HRGN hRgn = CreateRoundRectRgn(0, 0, w, h, radius, radius);
    if (hRgn)
    {
        //SetWindowRgn(hRgn, TRUE); // 绑定到窗口
        // 注意：SetWindowRgn 成功后，不要 DeleteObject(hRgn)！系统会管
    }

    // 初始化WebView
    m_webViewManager.Initialize(m_hWnd, this);

    // 创建托盘图标
    CreateTrayIcon();
    // 初始化老板键管理器
    m_bossKeyManager.SetHWnd(m_hWnd);

    // 检查命令行是否有 /autostart 参数（来自开机自动启动）
    BOOL bAutoStartLaunch = FALSE;
    LPTSTR lpCmdLine = GetCommandLine();
    CString strCmdLine(lpCmdLine);
    if (strCmdLine.Find(_T("/autostart")) >= 0 || strCmdLine.Find(_T("-autostart")) >= 0)
    {
        bAutoStartLaunch = TRUE;
    }


    // 如果是开机自启，则隐藏窗口到托盘
    if (bAutoStartLaunch)
    {
        // 避免任务栏按钮显示
        LONG_PTR dwExStyle = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
        dwExStyle &= ~WS_EX_APPWINDOW; // 去掉：强制任务栏按钮
        dwExStyle |= WS_EX_TOOLWINDOW; // 加上：工具窗口（无任务栏）
        dwExStyle |= WS_EX_LAYERED;     // 可选：层叠窗口，WebView2 更稳
        SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, dwExStyle);

        // 刷新样式，立即生效
        SetWindowPos(NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

        // 因为有WebView影响，导致初始隐藏会被再次显示出来
        SetWindowPos(NULL, -20000, -20000, 0, 0, SWP_NOSIZE);
    }


    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTimeCageDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTimeCageDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


BOOL CTimeCageDlg::OnEraseBkgnd(CDC* pDC)
{
    // 1. 获取当前对话框的客户区大小
    CRect rect;
    GetClientRect(&rect);

    // 2. 填充整个背景区域颜色，防止页面加载中的闪烁
    pDC->FillSolidRect(&rect, WEBVIEW_COLOR);

    // 3. 关键：直接返回 TRUE，告诉系统“我已经画好背景了”
    return TRUE;
}


// 从INI文件加载配置
void CTimeCageDlg::LoadConfig()
{
    CConfigManager::LoadShortcutButtons(m_arShortcutButton, SHORTCUT_BUTTON_COUNT);

    for (int i = 0; i < SHORTCUT_BUTTON_COUNT; i++)
    {
        SendWebViewCmd_SetShortCutButtonInfo(i, m_arShortcutButton[i].strProcessName, m_arShortcutButton[i].strComment, m_arShortcutButton[i].strIconIndex);
    }

    CConfigManager::LoadAppSettings(m_configData);
    SendWebViewCmd_SetConfigInfo(m_configData);

    CConfigManager::LoadOther(m_nLastSelShortButtonIndex);
    SendWebViewCmd_SelShortCutButton(m_nLastSelShortButtonIndex);


    // 注册老板键
    if (m_configData.bBossKeyEnabled && !m_configData.strBossKey.IsEmpty())
    {
        m_bossKeyManager.RegisterBossKey(m_configData.strBossKey);
    }
}

// 保存配置到INI文件
void CTimeCageDlg::SaveConfig()
{
    CConfigManager::SaveShortcutButtons(m_arShortcutButton, SHORTCUT_BUTTON_COUNT);
    CConfigManager::SaveAppSettings(m_configData);
}


// 冻结游戏
void CTimeCageDlg::ProcessWebViewCmd_FreezeGame(int nIndex)
{
    CString strProcessName = m_arShortcutButton[nIndex].strProcessName;
    if (strProcessName.IsEmpty())
    {
        SendWebViewCmd_ShowMessageBox(_T("请先设置好进程名（*.exe）,一般是启动文件名，如果不行就去任务管理器查看进程名。"));
        return;
    }

    CMyProcessHelper helper;
    int nPID;

    nPID = helper.GetProcessIdByName(strProcessName);

    if (0 == nPID)
    {
        CString strTip;

        strTip.Format(_T("进程（%s）未在运行中"), strProcessName);
        SendWebViewCmd_ShowMessageBox(strTip);
        return;
    }

    if (helper.SuspendProcess(nPID, m_configData.bHideWindow, m_configData.bHideWindowEnhanced))
    {
        SendWebViewCmd_FreezeGame(nIndex);
        m_arSuspendStatus[nIndex] = TRUE;
    }
    else
    {
        SendWebViewCmd_ShowMessageBox(_T("冻结游戏失败！"));
    }

}

// 解冻游戏
void CTimeCageDlg::ProcessWebViewCmd_ThawGame(int nIndex)
{
    CString strProcessName = m_arShortcutButton[nIndex].strProcessName;
    if (strProcessName.IsEmpty())
    {
        SendWebViewCmd_ShowMessageBox(_T("请先设置好进程名（*.exe）,一般是启动文件名，如果不行就去任务管理器查看进程名。"));
        return;
    }

    CMyProcessHelper helper;
    int nPID;

    nPID = helper.GetProcessIdByName(strProcessName);

    if (0 == nPID)
    {
        CString strTip;

        strTip.Format(_T("进程（%s）未在运行中"), strProcessName);
        SendWebViewCmd_ShowMessageBox(strTip);
        return;
    }

    if (helper.ResumeProcess(nPID, m_configData.bHideWindow, m_configData.bHideWindowEnhanced))
    {
        SendWebViewCmd_ThawGame(nIndex);
        m_arSuspendStatus[nIndex] = FALSE;
    }
    else
    {
        SendWebViewCmd_ShowMessageBox(_T("解冻游戏失败！"));
    }

}

// 保存选择的快捷按钮序号
void CTimeCageDlg::ProcessWebViewCmd_SelShortCutButton(int nIndex)
{
    // 保存序号到成员变量
    m_nLastSelShortButtonIndex = nIndex;

    // 保存到配置文件
    CConfigManager::SaveOther(m_nLastSelShortButtonIndex);
}

// 弹出提示框
void CTimeCageDlg::SendWebViewCmd_ShowMessageBox(CString strTip)
{
    m_webViewManager.SendJsonCommand(WebViewCmd::ShowMessageBox, strTip);
}

// 设置标题栏文本
void CTimeCageDlg::SendWebViewCmd_SetAppName(CString strAppName)
{
    m_webViewManager.SendJsonCommand(WebViewCmd::SetAppName, strAppName);
}

// 设置配置信息
void CTimeCageDlg::SendWebViewCmd_SetConfigInfo(const ConfigData& configData)
{
    // 构建 JSON 对象
    json myJson;
    CString strParam;

    strParam.Format(_T("%d"), configData.bHideWindow);
    myJson["hideWindow"] = std::string(CW2A(strParam, CP_UTF8));
    strParam.Format(_T("%d"), configData.bHideWindowEnhanced);
    myJson["hideWindowEnhanced"] = std::string(CW2A(strParam, CP_UTF8));
    strParam.Format(_T("%d"), configData.bSingleInstance);
    myJson["singleInstance"] = std::string(CW2A(strParam, CP_UTF8));
    strParam.Format(_T("%d"), configData.bMinimizeToTray);
    myJson["minimizeToTray"] = std::string(CW2A(strParam, CP_UTF8));
    strParam.Format(_T("%d"), configData.bBossKeyEnabled);
    myJson["bossKeyEnabled"] = std::string(CW2A(strParam, CP_UTF8));
    myJson["bossKey"] = std::string(CW2A(configData.strBossKey, CP_UTF8));
    strParam.Format(_T("%d"), configData.bAutoStart);
    myJson["autoStart"] = std::string(CW2A(strParam, CP_UTF8));

    m_webViewManager.SendJsonCommand(WebViewCmd::SetConfigInfo, myJson);
}

// 设置快捷按钮信息
void CTimeCageDlg::SendWebViewCmd_SetShortCutButtonInfo(int nButtonIndex, CString strProcessName, CString strComment, CString strIconIndex)
{
    m_webViewManager.SendJsonCommand(WebViewCmd::SetShortCutButtonInfo, CString(std::to_string(nButtonIndex).c_str()), strProcessName, strComment, strIconIndex);
}

// 游戏冻结
void CTimeCageDlg::SendWebViewCmd_FreezeGame(int nIndex)
{
    CString strParam1;

    strParam1.Format(_T("%d"), nIndex);
    m_webViewManager.SendJsonCommand(WebViewCmd::FreezeGame, strParam1);
}

// 解冻冻结
void CTimeCageDlg::SendWebViewCmd_ThawGame(int nIndex)
{
    CString strParam1;

    strParam1.Format(_T("%d"), nIndex);
    m_webViewManager.SendJsonCommand(WebViewCmd::ThawGame, strParam1);
}

// 设置当前选中的快捷按钮
void CTimeCageDlg::SendWebViewCmd_SelShortCutButton(int nIndex)
{
    CString strParam1;

    strParam1.Format(_T("%d"), nIndex);
    m_webViewManager.SendJsonCommand(WebViewCmd::SelShortCutButton, strParam1);
}

// 处理页面来的命令
// 设置配置信息
void CTimeCageDlg::ProcessWebViewCmd_SetConfigInfo(const ConfigData& configData)
{
    // 记录旧的自动启动状态，用于判断是否需要更新注册表
    BOOL bOldAutoStart = m_configData.bAutoStart;

    m_configData = configData;

    // 更新老板键
    if (configData.bBossKeyEnabled && !configData.strBossKey.IsEmpty())
    {
        m_bossKeyManager.RegisterBossKey(configData.strBossKey);
    }
    else
    {
        m_bossKeyManager.UnregisterBossKey();
    }

    // 应用开机自动启动设置
    if (bOldAutoStart != configData.bAutoStart)
    {
        ApplyAutoStart(configData.bAutoStart);
    }

    SaveConfig();
}

// 获取当前程序exe完整路径
CString CTimeCageDlg::GetAppExePath()
{
    TCHAR szPath[MAX_PATH] = { 0 };
    ::GetModuleFileName(NULL, szPath, MAX_PATH);
    return CString(szPath);
}

// 启用开机自动启动
BOOL CTimeCageDlg::EnableAutoStart()
{
    HKEY hKey = NULL;
    LONG lRet = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        0,
        KEY_SET_VALUE | KEY_QUERY_VALUE,
        &hKey);

    if (lRet != ERROR_SUCCESS)
    {
        TRACE(_T("打开 Run 注册表项失败, 错误码: %ld\n"), lRet);
        return FALSE;
    }

    CString strExePath = GetAppExePath();
    // 加上双引号，防止路径中有空格，并添加 /autostart 参数表示是开机自启
    CString strValue;
    strValue.Format(_T("\"%s\" /autostart"), strExePath);

    lRet = RegSetValueEx(
        hKey,
        _T("TimeCage"),
        0,
        REG_SZ,
        (const BYTE*)(LPCTSTR)strValue,
        (strValue.GetLength() + 1) * sizeof(TCHAR));

    RegCloseKey(hKey);

    if (lRet != ERROR_SUCCESS)
    {
        TRACE(_T("设置 Run 注册表值失败, 错误码: %ld\n"), lRet);
        return FALSE;
    }

    return TRUE;
}

// 禁用开机自动启动
BOOL CTimeCageDlg::DisableAutoStart()
{
    HKEY hKey = NULL;
    LONG lRet = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        0,
        KEY_SET_VALUE | KEY_QUERY_VALUE,
        &hKey);

    if (lRet != ERROR_SUCCESS)
    {
        // 如果打开失败，可能根本不存在，也算"禁用成功"
        if (lRet == ERROR_FILE_NOT_FOUND)
        {
            return TRUE;
        }
        TRACE(_T("打开 Run 注册表项失败, 错误码: %ld\n"), lRet);
        return FALSE;
    }

    lRet = RegDeleteValue(hKey, _T("TimeCage"));

    RegCloseKey(hKey);

    if (lRet == ERROR_SUCCESS || lRet == ERROR_FILE_NOT_FOUND)
    {
        return TRUE;
    }

    TRACE(_T("删除 Run 注册表值失败, 错误码: %ld\n"), lRet);
    return FALSE;
}

// 根据配置应用开机自动启动
void CTimeCageDlg::ApplyAutoStart(BOOL bEnable)
{
    if (bEnable)
    {
        if (!EnableAutoStart())
        {
            SendWebViewCmd_ShowMessageBox(_T("设置开机自动启动失败，请检查是否有注册表访问权限！"));
        }
    }
    else
    {
        if (!DisableAutoStart())
        {
            SendWebViewCmd_ShowMessageBox(_T("取消开机自动启动失败，请检查是否有注册表访问权限！"));
        }
    }
}

// 设置快捷按钮信息
void CTimeCageDlg::ProcessWebViewCmd_SetShortCutButtonInfo(int nButtonIndex, CString strProcessName, CString strComment, CString strIconIndex)
{
    m_arShortcutButton[nButtonIndex].strProcessName = strProcessName;
    m_arShortcutButton[nButtonIndex].strComment = strComment;
    m_arShortcutButton[nButtonIndex].strIconIndex = strIconIndex;

    SaveConfig();
}

// 退出程序 - 改为隐藏窗口到托盘
void CTimeCageDlg::ProcessWebViewCmd_SysQuit()
{
    if (m_configData.bMinimizeToTray)
    {
        HideMainWindow();
    }
    else
    {
        DeleteTrayIcon();
        PostQuitMessage(0);
    }
    
}

// 最小化窗口
void CTimeCageDlg::ProcessWebViewCmd_SysMinimize()
{
    ShowWindow(SW_MINIMIZE);
}

// 创建托盘图标
void CTimeCageDlg::CreateTrayIcon()
{
    if (m_bTrayIconCreated)
    {
        return;
    }

    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hWnd;
    m_nid.uID = IDR_MAINFRAME;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYMESSAGE;
    m_nid.hIcon = m_hIcon;
    _tcscpy_s(m_nid.szTip, _T("游戏冻结工具TimeCage"));

    Shell_NotifyIcon(NIM_ADD, &m_nid);
    m_bTrayIconCreated = TRUE;
}

// 删除托盘图标
void CTimeCageDlg::DeleteTrayIcon()
{
    if (!m_bTrayIconCreated)
    {
        return;
    }

    Shell_NotifyIcon(NIM_DELETE, &m_nid);
    m_bTrayIconCreated = FALSE;
}

// 显示窗口
void CTimeCageDlg::ShowMainWindow()
{
    if (::IsIconic(m_hWnd))
    {
        // 如果最小化了，强行还原窗口大小和位置
        ShowWindow(SW_RESTORE);
    }
    else if (!IsWindowVisible())
    {
        // 如果你之前是用 SW_HIDE 彻底隐藏了窗口，这里需要显示出来
        ShowWindow(SW_SHOW);
    }

    SetForegroundWindow();
    SetFocus();
}

// 隐藏窗口
void CTimeCageDlg::HideMainWindow()
{
    CDialogEx::ShowWindow(SW_HIDE);
}

// 托盘图标消息处理
LRESULT CTimeCageDlg::OnTrayMessage(WPARAM wParam, LPARAM lParam)
{
    if (wParam != IDR_MAINFRAME)
    {
        return 0;
    }

    switch (lParam)
    {
    case WM_LBUTTONDBLCLK:
        // 双击托盘图标，显示窗口
        ShowMainWindow();
        break;
    case WM_RBUTTONUP:
        // 右键弹出菜单
        {
            CMenu menu;
            if (menu.LoadMenu(IDR_MENU_POPUP))
            {
                CMenu* pSubMenu = menu.GetSubMenu(0);
                if (pSubMenu)
                {
                    POINT pt;
                    GetCursorPos(&pt);
                    
                    // 确保当前窗口是前台窗口，否则菜单可能无法正常工作
                    SetForegroundWindow();
                    
                    // 显示弹出菜单
                    pSubMenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_VERTICAL, pt.x, pt.y, this);
                    
                    // 处理菜单消息
                    PostMessage(WM_NULL, 0, 0);
                }
            }
        }
        break;
    }

    return 0;
}

// 托盘菜单-显示窗口
void CTimeCageDlg::OnMenuTrayShow()
{
    ShowMainWindow();
}

// 托盘菜单-退出程序
void CTimeCageDlg::OnMenuTrayExit()
{
    DeleteTrayIcon();
    PostQuitMessage(0);
}

// 窗口销毁时清理托盘图标
void CTimeCageDlg::OnDestroy()
{
    CDialogEx::OnDestroy();
    DeleteTrayIcon();
}

// 处理Explorer重启的情况
LRESULT CTimeCageDlg::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
    // Explorer重启了，重新创建托盘图标
    if (m_bTrayIconCreated)
    {
        DeleteTrayIcon();
    }
    CreateTrayIcon();
    return 0;
}

// WebView组件初始化完毕
void CTimeCageDlg::OnWebViewInitialized()
{
    CRect rect;
    GetClientRect(&rect);
    
    //设置页面的背景色，防止加载中的白色闪烁
    m_webViewManager.SetDefaultBackgroundColor(WEBVIEW_COLOR);

    CString strHtml = CWebView2Manager::LoadHtmlFromResource(IDR_HTML_INDEX);
    if (!strHtml.IsEmpty())
    {
        m_webViewManager.NavigateToString(strHtml);
    }

    m_webViewManager.SetBounds(rect);

    // 检查命令行是否有 /autostart 参数（来自开机自动启动）
    BOOL bAutoStartLaunch = FALSE;
    LPTSTR lpCmdLine = GetCommandLine();
    CString strCmdLine(lpCmdLine);
    if (strCmdLine.Find(_T("/autostart")) >= 0 || strCmdLine.Find(_T("-autostart")) >= 0)
    {
        bAutoStartLaunch = TRUE;
    }

    // 如果是开机自启，则隐藏窗口到托盘
    if (bAutoStartLaunch)
    {
        HideMainWindow();

        LONG_PTR dwExStyle = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
        if ((dwExStyle & WS_EX_APPWINDOW) == 0)
        {
            LONG_PTR exStyle = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
            exStyle &= ~WS_EX_TOOLWINDOW;
            exStyle |= WS_EX_APPWINDOW; // 恢复任务栏按钮
            SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, exStyle);
        }

        CenterWindow();
    }
}

// 收到来自页面的消息
void CTimeCageDlg::OnWebMessageReceived(const json& data)
{
    try
    {
        // 1. 安全获取 cmd（必须存在且为字符串）
        if (!data.contains("cmd") || !data["cmd"].is_string())
        {
            TRACE(_T("【警告】JSON 命令缺少 cmd 字段或类型错误\n"));
            return;
        }

        std::string strCmdUtf8 = data["cmd"].get<std::string>();
        CString strCmd = CA2W(strCmdUtf8.c_str(), CP_UTF8);

        // 2. 安全获取字符串参数（不存在则用空字符串）
        auto safeGetString = [&](const char* key) -> CString {
            if (data.contains(key) && data[key].is_string()) {
                std::string utf8 = data[key].get<std::string>();
                return CString(CA2W(utf8.c_str(), CP_UTF8));
            }
            return _T("");
        };

        // 3. 安全获取整数参数（不存在则用0）
        auto safeGetInt = [&](const char* key) -> int {
            if (data.contains(key) && data[key].is_number_integer()) {
                return data[key].get<int>();
            }
            return 0;
        };

        CString strParam1 = safeGetString("param1");
        CString strParam2 = safeGetString("param2");
        CString strParam3 = safeGetString("param3");
        CString strParam4 = safeGetString("param4");

        if (WebViewCmd::SetConfigInfo == strCmd)
        {// 设置配置信息
            std::string json_str(CW2A(strParam1.GetString(), CP_UTF8)); 
            json jsonConfigData = json::parse(json_str);

            ConfigData configData;
            if (JsonToConfigData(jsonConfigData, configData))
            {
                ProcessWebViewCmd_SetConfigInfo(configData);
            }
        }
        else if (WebViewCmd::SetShortCutButtonInfo == strCmd)
        {// 处理页面来的命令
            ProcessWebViewCmd_SetShortCutButtonInfo(_ttoi(strParam1.GetString()), strParam2, strParam3, strParam4);
        }
        else if (WebViewCmd::FreezeGame == strCmd)
        {// 冻结游戏
            ProcessWebViewCmd_FreezeGame(_ttoi(strParam1.GetString()));
        }
        else if (WebViewCmd::ThawGame == strCmd)
        {// 解冻游戏
            ProcessWebViewCmd_ThawGame(_ttoi(strParam1.GetString()));
        }
        else if (WebViewCmd::SelShortCutButton == strCmd)
        {// 选择快捷按钮
            ProcessWebViewCmd_SelShortCutButton(_ttoi(strParam1.GetString()));
        }
        else if (WebViewCmd::SysClose == strCmd)
        {// 退出程序
            ProcessWebViewCmd_SysQuit();
        }
        else if (WebViewCmd::SysMinimize == strCmd)
        {// 最小化窗口
            ProcessWebViewCmd_SysMinimize();
        }
    }
    catch (const std::exception& e)
    {
        TRACE(_T("处理 Web 消息失败: %hs\n"), e.what());
    }
}

// 加载页面完毕
void CTimeCageDlg::OnNavigationCompleted(bool success)
{
    if (!success)
    {
        return;
    }

    CString strAppName;

    strAppName.Format(_T("%s (v%s)"), APP_NAME, VERSION);

    SendWebViewCmd_SetAppName(strAppName);

    LoadConfig();
}

// 根据当前 DPI 计算缩放后的窗口尺寸
void CTimeCageDlg::CalculateWindowSizeForCurrentDPI(int& outWidth, int& outHeight)
{
    // 获取当前窗口的 DPI
    HDC hdc = ::GetDC(m_hWnd);
    int currentDpi = ::GetDeviceCaps(hdc, LOGPIXELSX);
    ::ReleaseDC(m_hWnd, hdc);

    // 计算缩放比例
    double scaleFactor = static_cast<double>(currentDpi) / static_cast<double>(BASE_DPI);

    // 根据基准尺寸计算缩放后的尺寸
    outWidth = static_cast<int>(WEBVIEW_WIDTH * scaleFactor);
    outHeight = static_cast<int>(WEBVIEW_HEIGHT * scaleFactor);

    // 输出调试信息
    TRACE(_T("当前 DPI: %d, 缩放比例: %.2f, 窗口尺寸: %d x %d\n"),
        currentDpi, scaleFactor, outWidth, outHeight);
}

//json字符串转为配置项
BOOL CTimeCageDlg::JsonToConfigData(json data, ConfigData& configData)
{
    auto safeGetString = [&](const char* key) -> CString {
        if (data.contains(key) && data[key].is_string()) {
            std::string utf8 = data[key].get<std::string>();
            return CString(CA2W(utf8.c_str(), CP_UTF8));
        }
        return _T("");
    };

    CString strHideWindow = safeGetString("hideWindow");

    if (_T("1") == strHideWindow)
    {
        configData.bHideWindow = TRUE;
    }
    else
    {
        configData.bHideWindow = FALSE;
    }

    CString strWindowEnhanced = safeGetString("hideWindowEnhanced");

    if (_T("1") == strWindowEnhanced)
    {
        configData.bHideWindowEnhanced = TRUE;
    }
    else
    {
        configData.bHideWindowEnhanced = FALSE;
    }

    CString strSingleInstance = safeGetString("singleInstance");

    if (_T("1") == strSingleInstance)
    {
        configData.bSingleInstance = TRUE;
    }
    else
    {
        configData.bSingleInstance = FALSE;
    }

    CString strMinimizeToTray = safeGetString("minimizeToTray");

    if (_T("1") == strMinimizeToTray)
    {
        configData.bMinimizeToTray = TRUE;
    }
    else
    {
        configData.bMinimizeToTray = FALSE;
    }

    CString strBossKeyEnabled = safeGetString("bossKeyEnabled");

    if (_T("1") == strBossKeyEnabled)
    {
        configData.bBossKeyEnabled = TRUE;
    }
    else
    {
        configData.bBossKeyEnabled = FALSE;
    }

    configData.strBossKey = safeGetString("bossKey");

    CString strAutoStart = safeGetString("autoStart");

    if (_T("1") == strAutoStart)
    {
        configData.bAutoStart = TRUE;
    }
    else
    {
        configData.bAutoStart = FALSE;
    }

    return TRUE;
}

// 处理热键消息
void CTimeCageDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
    if (m_bossKeyManager.OnHotKey(nHotKeyId, 0))
    {
        int nIndex = m_nLastSelShortButtonIndex;
        CString strProcessName = m_arShortcutButton[nIndex].strProcessName;
        if (strProcessName.IsEmpty())
        {
            return;
        }

        CMyProcessHelper helper;
        int nPID;

        nPID = helper.GetProcessIdByName(strProcessName);

        if (0 == nPID)
        {
            return;
        }

        // 将当前选择快捷按钮进程冻结或解冻
        if (m_arSuspendStatus[nIndex])
        {
            if (helper.ResumeProcess(nPID, m_configData.bHideWindow, m_configData.bHideWindowEnhanced))
            {
                SendWebViewCmd_ThawGame(nIndex);
                m_arSuspendStatus[nIndex] = FALSE;
            }
        }
        else
        {
            if (helper.SuspendProcess(nPID, m_configData.bHideWindow, m_configData.bHideWindowEnhanced))
            {
                SendWebViewCmd_FreezeGame(nIndex);
                m_arSuspendStatus[nIndex] = TRUE;
            }
        }
    }
}
