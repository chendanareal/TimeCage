
// WebView2AppDlg.h: 头文件
//

#pragma once
#include "WebView2Manager.h"
#include "ConfigManager.h"


// CTimeCageDlg 对话框
class CTimeCageDlg : public CDialogEx, public IWebView2MessageHandler
{
// 构造
public:
	CTimeCageDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WEBVIEW2APP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	

private:
	BOOL m_bHideWindow;				//冻结时自动隐藏窗口
	BOOL m_bHideWindowEnhanced;		//隐藏窗口增强
	CConfigManager m_configManager; //配置文件管理


	// 快捷按钮信息
	struct ShortcutButtonData m_arShortcutButton[4];

	// 从INI文件加载配置
	void LoadConfig();
	// 保存配置到INI文件
	void SaveConfig();

	// 根据当前 DPI 计算缩放后的窗口尺寸
	void CalculateWindowSizeForCurrentDPI(int& outWidth, int& outHeight);

private:
	// 与WebView交互
	// WebView组件
	CWebView2Manager m_webViewManager;

	// WebView组件初始化完毕
	virtual void OnWebViewInitialized() override;
	// 收到来自页面的消息
	virtual void OnWebMessageReceived(const json& data) override;
	// 加载页面完毕
	virtual void OnNavigationCompleted(bool success) override;


	//发送给页面的命令
	// 弹出提示框
	void SendWebViewCmd_ShowMessageBox(CString strTip);
	// 设置标题栏文本
	void SendWebViewCmd_SetAppName(CString strAppName);
	// 设置快捷按钮信息
	void SendWebViewCmd_SetShortCutButtonInfo(int nButtonIndex, CString strProcessName, CString strComment, CString strIconIndex);
	// 游戏冻结
	void SendWebViewCmd_FreezeGame(int nIndex);
	// 游戏解冻
	void SendWebViewCmd_ThawGame(int nIndex);

	// 处理页面来的命令
	// 设置快捷按钮信息
	void ProcessWebViewCmd_SetShortCutButtonInfo(int nButtonIndex, CString strProcessName, CString strComment, CString strIconIndex);
	// 切换选项
	void ProcessWebViewCmd_ToggleOption(CString strOption);
	// 冻结游戏
	void ProcessWebViewCmd_FreezeGame(int nIndex);
	// 解冻游戏
	void ProcessWebViewCmd_ThawGame(int nIndex);
	// 退出程序
	void ProcessWebViewCmd_SysQuit();
	// 最小化窗口
	void ProcessWebViewCmd_SysMinimize();
};
