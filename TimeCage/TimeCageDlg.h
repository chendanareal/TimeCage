
// WebView2AppDlg.h: 头文件
//

#pragma once
#include "WebView2Manager.h"
#include "ConfigManager.h"
#include "BossKeyManager.h"


// CTimeCageDlg 对话框
class CTimeCageDlg : public CDialogEx, public IWebView2MessageHandler
{
// 构造
public:
    CTimeCageDlg(CWnd* pParent = nullptr);    // 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_WEBVIEW2APP_DIALOG };
#endif

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持


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
    afx_msg LRESULT OnTrayMessage(WPARAM wParam, LPARAM lParam);
    afx_msg void OnMenuTrayShow();
    afx_msg void OnMenuTrayExit();
    afx_msg void OnDestroy();
    afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
    afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);

    

private:
    ConfigData m_configData;            //配置信息

    // 最后一次选择快捷按钮序号
    int m_nLastSelShortButtonIndex;   
    // 挂起状态
    BOOL m_arSuspendStatus[SHORTCUT_BUTTON_COUNT];

    // 快捷按钮信息
    struct ShortcutButtonData m_arShortcutButton[SHORTCUT_BUTTON_COUNT];

    // 从INI文件加载配置
    void LoadConfig();
    // 保存配置到INI文件
    void SaveConfig();

    // 根据当前 DPI 计算缩放后的窗口尺寸
    void CalculateWindowSizeForCurrentDPI(int& outWidth, int& outHeight);

    // 托盘图标相关
    NOTIFYICONDATA m_nid;            // 托盘图标数据结构
    BOOL m_bTrayIconCreated;         // 托盘图标是否已创建

    // 创建托盘图标
    void CreateTrayIcon();
    // 删除托盘图标
    void DeleteTrayIcon();
    // 显示窗口
    void ShowMainWindow();
    // 隐藏窗口
    void HideMainWindow();

    // 老板键管理
    CBossKeyManager m_bossKeyManager;

    // 开机自动启动相关
// 启用开机自动启动（写注册表）
    BOOL EnableAutoStart();
    // 禁用开机自动启动（删除注册表项）
    BOOL DisableAutoStart();
    // 根据配置应用开机自动启动
    void ApplyAutoStart(BOOL bEnable);
    // 获取当前程序exe完整路径
    CString GetAppExePath();

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
    // 设置配置信息
    void SendWebViewCmd_SetConfigInfo(const ConfigData& configData);
    // 设置快捷按钮信息
    void SendWebViewCmd_SetShortCutButtonInfo(int nButtonIndex, CString strProcessName, CString strComment, CString strIconIndex);
    // 游戏冻结
    void SendWebViewCmd_FreezeGame(int nIndex);
    // 游戏解冻
    void SendWebViewCmd_ThawGame(int nIndex);
    // 设置当前选中的快捷按钮
    void SendWebViewCmd_SelShortCutButton(int nIndex);

    // 处理页面来的命令
    // 设置配置信息
    void ProcessWebViewCmd_SetConfigInfo(const ConfigData &configData);
    // 设置快捷按钮信息
    void ProcessWebViewCmd_SetShortCutButtonInfo(int nButtonIndex, CString strProcessName, CString strComment, CString strIconIndex);
    // 冻结游戏
    void ProcessWebViewCmd_FreezeGame(int nIndex);
    // 解冻游戏
    void ProcessWebViewCmd_ThawGame(int nIndex);
    // 选择快捷按钮
    void ProcessWebViewCmd_SelShortCutButton(int nIndex);
    // 退出程序
    void ProcessWebViewCmd_SysQuit();
    // 最小化窗口
    void ProcessWebViewCmd_SysMinimize();


    //json字符串转为配置项
    BOOL JsonToConfigData(json data, ConfigData &configData);
};
