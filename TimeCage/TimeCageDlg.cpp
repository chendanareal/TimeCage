
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
	, m_bHideWindow(TRUE)
	, m_bHideWindowEnhanced(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTimeCageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTimeCageDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CWebView2Dlg 消息处理程序

BOOL CTimeCageDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

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
	m_configManager.LoadShortcutButtons(m_arShortcutButton, SHORTCUT_BUTTON_COUNT);

	for (int i = 0; i < SHORTCUT_BUTTON_COUNT; i++)
	{
		SendWebViewCmd_SetShortCutButtonInfo(i, m_arShortcutButton[i].strProcessName, m_arShortcutButton[i].strComment, m_arShortcutButton[i].strIconIndex);
	}
}

// 保存配置到INI文件
void CTimeCageDlg::SaveConfig()
{
	m_configManager.SaveShortcutButtons(m_arShortcutButton, SHORTCUT_BUTTON_COUNT);
}

// 切换选项
void CTimeCageDlg::ProcessWebViewCmd_ToggleOption(CString strOption)
{
	if (_T("hideWindow") == strOption)
	{
		m_bHideWindow = !m_bHideWindow;
	}
	else if (_T("hideWindowEnhanced") == strOption)
	{
		m_bHideWindowEnhanced = !m_bHideWindowEnhanced;
	}
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

	if (helper.SuspendProcess(nPID, m_bHideWindow, m_bHideWindowEnhanced))
	{
		SendWebViewCmd_FreezeGame(nIndex);
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

	if (helper.ResumeProcess(nPID, m_bHideWindow, m_bHideWindowEnhanced))
	{
		SendWebViewCmd_ThawGame(nIndex);
	}
	else
	{
		SendWebViewCmd_ShowMessageBox(_T("解冻游戏失败！"));
	}

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

// 处理页面来的命令
void CTimeCageDlg::ProcessWebViewCmd_SetShortCutButtonInfo(int nButtonIndex, CString strProcessName, CString strComment, CString strIconIndex)
{
	m_arShortcutButton[nButtonIndex].strProcessName = strProcessName;
	m_arShortcutButton[nButtonIndex].strComment = strComment;
	m_arShortcutButton[nButtonIndex].strIconIndex = strIconIndex;

	SaveConfig();
}

// 退出程序
void CTimeCageDlg::ProcessWebViewCmd_SysQuit()
{
	PostQuitMessage(0);
}

// 最小化窗口
void CTimeCageDlg::ProcessWebViewCmd_SysMinimize()
{
	ShowWindow(SW_MINIMIZE);
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

		if (WebViewCmd::SetShortCutButtonInfo == strCmd)
		{// 处理页面来的命令
			ProcessWebViewCmd_SetShortCutButtonInfo(_ttoi(strParam1.GetString()), strParam2, strParam3, strParam4);
		}
		else if (WebViewCmd::ToggleOption == strCmd)
		{// 切换选项
			ProcessWebViewCmd_ToggleOption(strParam1);
		}
		else if (WebViewCmd::FreezeGame == strCmd)
		{// 冻结游戏
			ProcessWebViewCmd_FreezeGame(_ttoi(strParam1.GetString()));
		}
		else if (WebViewCmd::ThawGame == strCmd)
		{// 解冻游戏
			ProcessWebViewCmd_ThawGame(_ttoi(strParam1.GetString()));
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