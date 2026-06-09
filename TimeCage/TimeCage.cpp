
// WebView2App.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "TimeCage.h"
#include "TimeCageDlg.h"
#include "define.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWebView2App

BEGIN_MESSAGE_MAP(CTimeCageApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CWebView2App 构造

CTimeCageApp::CTimeCageApp()
{
    // 支持重新启动管理器
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

    m_hMutex = NULL;
}

CTimeCageApp::~CTimeCageApp()
{
    if (NULL != m_hMutex)
    {
        CloseHandle(m_hMutex);
        m_hMutex = NULL;
    }
}

// 检查单实例
BOOL CTimeCageApp::CheckSingleInstance()
{
    // 创建一个唯一的互斥体名称
    CString strMutexName;
    strMutexName.Format(_T("TimeCage_SingleInstance_Mutex_%s"), AfxGetAppName());

    // 尝试创建互斥体
    m_hMutex = CreateMutex(NULL, TRUE, strMutexName);
    
    if (NULL == m_hMutex)
    {
        // 创建失败，可能有其他实例在运行
        return FALSE;
    }

    // 检查是否已经存在该互斥体
    if (ERROR_ALREADY_EXISTS == GetLastError())
    {
        // 互斥体已存在，说明有另一个实例在运行
        CloseHandle(m_hMutex);
        m_hMutex = NULL;
        return FALSE;
    }

    // 没有其他实例在运行
    return TRUE;
}

// 激活已存在的实例
void CTimeCageApp::ActivateExistingInstance()
{
    // 查找已存在的窗口 - 使用多种方式查找
    CWnd* pWnd = CWnd::FindWindow(NULL, _T("游戏冻结工具TimeCage"));
    
    if (NULL != pWnd)
    {
        // 显示并激活窗口
        pWnd->ShowWindow(SW_RESTORE);
        pWnd->SetForegroundWindow();
        pWnd->SetFocus();
        return;
    }
    
    // 如果没有找到，尝试查找带版本号的窗口
    CString strWindowTitle;
    strWindowTitle.Format(_T("%s (v%s)"), APP_NAME, VERSION);
    pWnd = CWnd::FindWindow(NULL, strWindowTitle);
    
    if (NULL != pWnd)
    {
        pWnd->ShowWindow(SW_RESTORE);
        pWnd->SetForegroundWindow();
        pWnd->SetFocus();
        return;
    }
    
    // 如果还是没有找到，尝试通过窗口类名查找
    // 或者枚举所有窗口查找
    CWnd* pDesktopWnd = CWnd::GetDesktopWindow();
    CWnd* pChildWnd = pDesktopWnd->GetWindow(GW_CHILD);
    
    while (NULL != pChildWnd)
    {
        CString strWindowText;
        pChildWnd->GetWindowText(strWindowText);
        
        // 检查窗口标题是否包含我们的程序名称
        if (-1 != strWindowText.Find(_T("游戏冻结工具")))
        {
            pChildWnd->ShowWindow(SW_RESTORE);
            pChildWnd->SetForegroundWindow();
            pChildWnd->SetFocus();
            break;
        }
        
        pChildWnd = pChildWnd->GetNextWindow(GW_HWNDNEXT);
    }
}


// 唯一的 CWebView2App 对象

CTimeCageApp theApp;


// CWebView2App 初始化

BOOL CTimeCageApp::InitInstance()
{
    // 首先检查是否有其他实例在运行
    ConfigData configData;

    if (CConfigManager::LoadAppSettings(configData))
    {
        if (configData.bSingleInstance)
        {
            if (!CheckSingleInstance())
            {
                // 有其他实例在运行，激活它并退出
                ActivateExistingInstance();
                return FALSE;
            }
        }

    }


    // 如果一个运行在 Windows XP 上的应用程序清单指定要
    // 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
    //则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // 将它设置为包括所有要在应用程序中使用的
    // 公共控件类。
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();


    AfxEnableControlContainer();

    // 创建 shell 管理器，以防对话框包含
    // 任何 shell 树视图控件或 shell 列表视图控件。
    CShellManager *pShellManager = new CShellManager;

    // 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    // 标准初始化
    // 如果未使用这些功能并希望减小
    // 最终可执行文件的大小，则应移除下列
    // 不需要的特定初始化例程
    // 更改用于存储设置的注册表项
    // TODO: 应适当修改该字符串，
    // 例如修改为公司或组织名
    SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    CTimeCageDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // TODO: 在此放置处理何时用
        //  “确定”来关闭对话框的代码
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: 在此放置处理何时用
        //  “取消”来关闭对话框的代码
    }
    else if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
        TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
    }

    // 删除上面创建的 shell 管理器。
    if (pShellManager != nullptr)
    {
        delete pShellManager;
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    // 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
    //  而不是启动应用程序的消息泵。
    return FALSE;
}

