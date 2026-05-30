
#pragma once

#include <afxwin.h>
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include <nlohmann/json.hpp>
#include "define.h"

using json = nlohmann::json;

/**
 * @brief WebView2 消息回调接口
 *
 * 实现此接口的类可以接收来自 WebView2 的消息和导航完成通知。
 */
class IWebView2MessageHandler
{
public:
    /**
     * @brief WebView2 初始化完成时调用
     */
    virtual void OnWebViewInitialized() = 0;

    /**
     * @brief 接收到来自网页的消息时调用
     * @param data JSON 格式的消息数据
     */
    virtual void OnWebMessageReceived(const json& data) = 0;

    /**
     * @brief WebView2 导航完成时调用
     * @param success 导航是否成功
     */
    virtual void OnNavigationCompleted(bool success) = 0;
};

/**
 * @brief WebView2 管理器 - 负责 WebView2 初始化、消息收发
 *
 * 该类封装了 WebView2 控件的所有操作，包括：
 * - 异步初始化 WebView2 控件
 * - 与网页之间的 JSON 消息通信
 * - 资源管理和清理
 *
 * ## 使用步骤
 *
 * ### 1. 实现回调接口
 * 让你的窗口类继承 IWebView2MessageHandler，并实现三个纯虚函数：
 * @code
 * class CMyDlg : public CDialogEx, public IWebView2MessageHandler
 * {
 * public:
 *     virtual void OnWebViewInitialized() override;
 *     virtual void OnWebMessageReceived(const json& data) override;
 *     virtual void OnNavigationCompleted(bool success) override;
 * };
 * @endcode
 *
 * ### 2. 在窗口类中声明管理器
 * @code
 * class CMyDlg : public CDialogEx
 * {
 * private:
 *     CWebView2Manager m_webViewManager;  // WebView2 管理器
 * };
 * @endcode
 *
 * ### 3. 初始化 WebView2
 * 在 OnInitDialog 中调用 Initialize：
 * @code
 * BOOL CMyDlg::OnInitDialog()
 * {
 *     CDialogEx::OnInitDialog();
 *
 *     // 初始化 WebView2，传入父窗口句柄和回调处理器
 *     m_webViewManager.Initialize(m_hWnd, this);
 *
 *     return TRUE;
 * }
 * @endcode
 *
 * ### 4. 在初始化完成回调中进行导航
 * 实现 OnWebViewInitialized 函数，在初始化完成后执行导航：
 * @code
 * void CMyDlg::OnWebViewInitialized()
 * {
 *     // 设置边界
 *     CRect rect;
 *     GetClientRect(&rect);
 *     m_webViewManager.SetBounds(rect);
 *
 *     // 方式1：导航到 HTML 字符串
 *     CString strHtml = CWebView2Manager::LoadHtmlFromResource(IDR_HTML_MY_PAGE);
 *     m_webViewManager.NavigateToString(strHtml);
 *
 *     // 方式2：导航到 URL 地址
 *     // m_webViewManager.Navigate(_T("https://www.example.com"));
 * }
 * @endcode
 *
 * ### 5. 动态导航
 * 在程序运行中可以随时导航到新的 URL：
 * @code
 * void CMyDlg::NavigateToWebsite()
 * {
 *     m_webViewManager.Navigate(_T("https://www.example.com"));
 * }
 * @endcode
 *
 * ### 4. 处理窗口大小变化
 * @code
 * void CMyDlg::OnSize(UINT nType, int cx, int cy)
 * {
 *     CDialogEx::OnSize(nType, cx, cy);
 *
 *     CRect rect;
 *     GetClientRect(&rect);
 *     m_webViewManager.SetBounds(rect);
 * }
 * @endcode
 *
 * ### 5. 发送消息到网页
 * @code
 * void CMyDlg::SendMessageToWeb()
 * {
 *     m_webViewManager.SendJsonCommand(
 *         _T("show_message"),
 *         _T("Hello from C++!"));
 * }
 * @endcode
 *
 * ### 6. 处理来自网页的消息
 * @code
 * void CMyDlg::OnWebMessageReceived(const json& data)
 * {
 *     std::string strCmd = data["cmd"];
 *
 *     if (strCmd == "user_click")
 *     {
 *         // 处理用户点击事件
 *     }
 * }
 * @endcode
 *
 * ### 7. 清理资源
 * 在窗口关闭时调用 Shutdown（析构函数会自动调用）：
 * @code
 * void CMyDlg::OnDestroy()
 * {
 *     CDialogEx::OnDestroy();
 *     m_webViewManager.Shutdown();
 * }
 * @endcode
 *
 * ## 注意事项
 *
 * - WebView2 的初始化是异步的，Initialize 不会阻塞线程
 * - 在 WebView 初始化完成前，不要调用 Navigate、NavigateToString 等方法
 * - 所有初始化完成后会触发 OnWebViewInitialized 回调
 * - 在 OnWebViewInitialized 回调中进行首次导航和设置
 * - 网页需要实现 onReceiveCppJson 函数来接收 C++ 发送的消息
 * - 网页通过 window.chrome.webview.postMessage 发送消息给 C++
 */
class CWebView2Manager
{
public:
    /**
     * @brief 构造函数
     */
    CWebView2Manager();

    /**
     * @brief 析构函数
     */
    ~CWebView2Manager();

    /**
     * @brief 初始化 WebView2 控件
     * @param hParentWnd 父窗口句柄
     * @param pHandler 消息回调处理器
     * @return 初始化是否成功（注意：实际初始化是异步的）
     */
    bool Initialize(HWND hParentWnd, IWebView2MessageHandler* pHandler);

    /**
     * @brief 获取系统临时目录路径
     * @return 临时目录路径（宽字符串）
     */
    static std::wstring GetSystemTempPath();

    /**
     * @brief 获取 WebView2 用户数据目录
     * @return 用户数据目录路径（宽字符串）
     */
    static std::wstring GetWebView2TempDataPath();

    /**
     * @brief 获取 纯EXE名称,不带 .exe
     * @return 纯EXE名称（宽字符串）
     */
    static std::wstring GetExeName();

    /**
     * @brief 从资源加载 HTML 内容
     * @param nResourceID HTML 资源 ID
     * @return 加载的 HTML 字符串
     */
    static CString LoadHtmlFromResource(UINT nResourceID);

    /**
     * @brief 向网页发送 JSON 命令
     * @param strCmd 命令名称
     * @param strParam1 参数1（可选）
     * @param strParam2 参数2（可选）
     * @param strParam3 参数3（可选）
     * @param strParam4 参数4（可选）
     * @return 发送是否成功
     */
    bool SendJsonCommand(CString strCmd, CString strParam1 = _T(""),
        CString strParam2 = _T(""), CString strParam3 = _T(""),
        CString strParam4 = _T(""));

    /**
     * @brief 在网页中执行 JavaScript 脚本
     * @param strScript 要执行的脚本
     * @return 执行是否成功
     */
    bool ExecuteScript(const CString& strScript);

    /**
     * @brief 导航到 HTML 内容
     *
     * @param strHtml HTML 内容
     * @return 操作是否成功
     */
    bool NavigateToString(const CString& strHtml);

    /**
     * @brief 导航到 URL 地址
     *
     * @param strUrl URL 地址（例如："https://www.example.com"）
     * @return 操作是否成功
     */
    bool Navigate(const CString& strUrl);

    /**
     * @brief 设置 WebView2 控件的可见性
     *
     * @param bVisible 是否可见
     */
    void SetVisible(bool bVisible);

    /**
     * @brief 设置 WebView2 控件的边界矩形
     *
     * @param rect 边界矩形
     */
    void SetBounds(const RECT& rect);

    /**
     * @brief 关闭并清理 WebView2 资源
     */
    void Shutdown();

    /**
    * @brief 设置页面底色
    */
    void SetDefaultBackgroundColor(COLORREF color);
private:
    /**
     * @brief 注册网页消息接收事件
     */
    void RegisterWebMessageReceived();

    /**
     * @brief WebView2 控制器创建完成的静态回调
     */
    static HRESULT CALLBACK OnCreateCoreWebView2ControllerCompleted(
        HRESULT result, ICoreWebView2Controller* controller, LPARAM param);

    // WebView2 控件相关成员
    wil::com_ptr<ICoreWebView2Controller> m_webController;  // WebView2 控制器
    wil::com_ptr<ICoreWebView2> m_webView;                  // WebView2 核心对象
    IWebView2MessageHandler* m_pMessageHandler;             // 消息回调处理器
    HWND m_hParentWnd;                                       // 父窗口句柄
    bool m_bInitialized;                                     // 是否已初始化完成
};

