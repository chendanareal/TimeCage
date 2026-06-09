
#include "pch.h"
#include "WebView2Manager.h"

/**
 * @brief 构造函数
 */
CWebView2Manager::CWebView2Manager()
    : m_pMessageHandler(nullptr)
    , m_hParentWnd(nullptr)
    , m_bInitialized(false)
{
}

/**
 * @brief 析构函数
 */
CWebView2Manager::~CWebView2Manager()
{
    Shutdown();
}

/**
 * @brief 初始化 WebView2 控件
 *
 * @param hParentWnd 父窗口句柄
 * @param pHandler 消息回调处理器
 * @return 初始化是否成功（仅表示异步启动是否成功）
 */
bool CWebView2Manager::Initialize(HWND hParentWnd, IWebView2MessageHandler* pHandler)
{
    if (m_bInitialized)
        return true;

    m_hParentWnd = hParentWnd;
    m_pMessageHandler = pHandler;

    // 获取用户数据目录
    std::wstring userDataFolder = GetWebView2TempDataPath();

    // 创建 WebView2 环境
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        userDataFolder.c_str(),
        nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                if (FAILED(result))
                    return result;

                // 创建 WebView2 控制器
                env->CreateCoreWebView2Controller(
                    m_hParentWnd,
                    Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (FAILED(result))
                                return result;

                            m_webController = controller;
                            m_webController->get_CoreWebView2(&m_webView);

                            if (m_webView)
                            {
                                // 注册消息接收事件
                                RegisterWebMessageReceived();

                                // 启用非客户区支持（用于自定义标题栏）
                                wil::com_ptr<ICoreWebView2Settings> settings;
                                HRESULT hr = m_webView->get_Settings(&settings);

                                if (FAILED(hr))
                                {
                                    TRACE(_T("获取 WebView2 设置失败: 0x%08X\n"), hr);
                                    return hr;
                                }

                                wil::com_ptr<ICoreWebView2Settings9> settings9;
                                if (SUCCEEDED(settings->QueryInterface(IID_PPV_ARGS(&settings9))))
                                {
                                    settings9->put_IsNonClientRegionSupportEnabled(TRUE);
                                }

                                // 注册导航完成事件
                                EventRegistrationToken token;
                                m_webView->add_NavigationCompleted(
                                    Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                        [this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
                                        {
                                            BOOL success = FALSE;
                                            args->get_IsSuccess(&success);

                                            // 通知回调处理器
                                            if (m_pMessageHandler)
                                            {
                                                m_pMessageHandler->OnNavigationCompleted(success != FALSE);
                                            }

                                            return S_OK;
                                        }
                                        ).Get(),
                                            &token
                                            );

                                // 标记为已初始化
                                m_bInitialized = true;

                                if (m_pMessageHandler)
                                {
                                    m_pMessageHandler->OnWebViewInitialized();
                                }
                            }

                            return S_OK;
                        }
                        ).Get()
                            );

                return S_OK;
            }
            ).Get()
                );

    return SUCCEEDED(hr);
}


/**
 * @brief 获取系统临时目录路径
 * @return 临时目录路径（宽字符串）
 */
std::wstring CWebView2Manager::GetSystemTempPath()
{
    WCHAR buf[MAX_PATH] = { 0 };
    ::GetTempPathW(MAX_PATH, buf);
    return std::wstring(buf);
}

/**
     * @brief 获取 纯EXE名称,不带 .exe
     * @return 纯EXE名称（宽字符串）
     */
std::wstring CWebView2Manager::GetExeName()
{
    WCHAR szPath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, szPath, MAX_PATH);

    // 取文件名（如 TimeCage.exe）
    PWSTR pFileName = PathFindFileNameW(szPath);
    std::wstring fileName = pFileName;

    // 去掉后缀 .exe
    size_t pos = fileName.find_last_of(L'.');
    if (pos != std::wstring::npos)
    {
        fileName = fileName.substr(0, pos);
    }

    return fileName;
}

/**
 * @brief 获取 WebView2 用户数据目录
 *
 * WebView2 的缓存、Cookie等用户数据都存储在此目录下。
 *
 * @return 用户数据目录路径（宽字符串）
 */
std::wstring CWebView2Manager::GetWebView2TempDataPath()
{
    std::wstring temp = GetSystemTempPath();

    // 动态获取工程名（EXE名）
    std::wstring projectName = GetExeName();

    // 拼接：系统临时目录\\工程名_Data
    temp += projectName;
    temp += L"_Data\\";

    return temp;
}

/**
 * @brief 从资源加载 HTML 内容
 * @param nResourceID HTML 资源 ID
 * @return 加载的 HTML 字符串
 */
CString CWebView2Manager::LoadHtmlFromResource(UINT nResourceID)
{
    CString strHtml = _T("");

    // 查找资源
    HRSRC hRes = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResourceID), RT_HTML);
    if (hRes == NULL)
        return strHtml;

    // 加载资源
    HGLOBAL hData = LoadResource(AfxGetResourceHandle(), hRes);
    DWORD dwSize = SizeofResource(AfxGetResourceHandle(), hRes);

    if (hData != NULL && dwSize > 0)
    {
        // 锁定资源获取数据指针
        const char* pData = (const char*)LockResource(hData);
        if (pData != NULL)
        {
#ifdef _UNICODE
            // Unicode 模式下转换为宽字符串
            strHtml = CA2W(pData, CP_UTF8);
            strHtml = strHtml.Left(dwSize);
#else
            // ANSI 模式下直接使用
            strHtml.SetString(pData, dwSize);
#endif
        }
    }

    return strHtml;
}

/**
 * @brief 向网页发送 JSON 命令
 *
 * 该方法会将命令和参数打包成 JSON 对象，
 * 然后通过 ExecuteScript 发送给网页的 onReceiveCppJson 函数。
 *
 * @param strCmd 命令名称
 * @param strParam1 参数1
 * @param strParam2 参数2
 * @param strParam3 参数3
 * @param strParam4 参数4
 * @return 发送是否成功
 */
bool CWebView2Manager::SendJsonCommand(CString strCmd, CString strParam1,
    CString strParam2, CString strParam3,
    CString strParam4)
{
    // 检查是否已初始化
    if (!m_bInitialized || !m_webView)
        return false;

    // 构建 JSON 对象
    json myJson;
    myJson["cmd"] = std::string(CW2A(strCmd, CP_UTF8));
    myJson["param1"] = std::string(CW2A(strParam1, CP_UTF8));
    myJson["param2"] = std::string(CW2A(strParam2, CP_UTF8));
    myJson["param3"] = std::string(CW2A(strParam3, CP_UTF8));
    myJson["param4"] = std::string(CW2A(strParam4, CP_UTF8));

    // 序列化 JSON 为字符串
    std::string stdJsonStr = myJson.dump();
    CString strJsonCmd(CA2W(stdJsonStr.c_str(), CP_UTF8));

    // 执行脚本
    return ExecuteScript(strJsonCmd);
}

/**
 * @brief 向网页发送 JSON 命令
 *
 * 该方法会将命令和参数打包成 JSON 对象，
 * 然后通过 ExecuteScript 发送给网页的 onReceiveCppJson 函数。
 *
 * @param strCmd 命令名称
 * @param jsonParam1 参数1
 * @return 发送是否成功
 */
bool CWebView2Manager::SendJsonCommand(CString strCmd, json &jsonParam1)
{
    // 检查是否已初始化
    if (!m_bInitialized || !m_webView)
        return false;

    // 构建 JSON 对象
    json myJson;
    myJson["cmd"] = std::string(CW2A(strCmd, CP_UTF8));
    myJson["param1"] = jsonParam1;

    // 序列化 JSON 为字符串
    std::string stdJsonStr = myJson.dump();
    CString strJsonCmd(CA2W(stdJsonStr.c_str(), CP_UTF8));

    // 执行脚本
    return ExecuteScript(strJsonCmd);
}


/**
 * @brief 在网页中执行 JavaScript 脚本
 * @param strScript 要执行的脚本
 * @return 执行是否成功
 */
bool CWebView2Manager::ExecuteScript(const CString& strScript)
{
    if (!m_bInitialized || !m_webView)
        return false;

    // 转义双引号，防止 JSON 解析错误
    CString strEscaped = strScript;
    strEscaped.Replace(_T("\""), _T("\\\""));

    // 构建完整的调用语句
    CString strExecute;
    strExecute.Format(_T("onReceiveCppJson(\"%s\");"), strEscaped);

    // 执行脚本
    m_webView->ExecuteScript(strExecute, nullptr);
    return true;
}

/**
 * @brief 导航到 HTML 内容
 *
 * 如果 WebView2 尚未初始化完成，该方法会缓存 HTML 内容，
 * 等待初始化完成后自动执行导航。
 *
 * @param strHtml HTML 内容
 * @return 操作是否成功
 */
bool CWebView2Manager::NavigateToString(const CString& strHtml)
{
    if (m_bInitialized && m_webView)
    {
        // 已初始化，直接导航
        m_webView->NavigateToString(strHtml);
        return true;
    }

    return false;
}
/**
 * @brief 导航到 URL 地址
 *
 * @param strUrl URL 地址
 * @return 操作是否成功
 */
bool CWebView2Manager::Navigate(const CString & strUrl)
{
    if (!m_bInitialized || !m_webView)
        return false;

    m_webView->Navigate(strUrl);
    return true;
}

/**
 * @brief 设置 WebView2 控件的可见性
 *
 * 如果 WebView2 尚未初始化完成，该方法会缓存可见性状态，
 * 等待初始化完成后自动应用。
 *
 * @param bVisible 是否可见
 */
void CWebView2Manager::SetVisible(bool bVisible)
{
    if (m_bInitialized && m_webController)
    {
        // 已初始化，直接设置
        m_webController->put_IsVisible(bVisible);
    }
}

/**
 * @brief 设置 WebView2 控件的边界矩形
 *
 * 如果 WebView2 尚未初始化完成，该方法会缓存边界信息，
 * 等待初始化完成后自动应用。
 *
 * @param rect 边界矩形
 */
void CWebView2Manager::SetBounds(const RECT & rect)
{
    if (m_bInitialized && m_webController)
    {
        m_webController->put_Bounds(rect);
    }
}

/**
 * @brief 关闭并清理 WebView2 资源
 */
void CWebView2Manager::Shutdown()
{
    // 释放 COM 指针
    m_webController.reset();
    m_webView.reset();

    // 重置状态
    m_bInitialized = false;
}

/**
 * @brief 注册网页消息接收事件
 *
 * 当网页通过 window.chrome.webview.postMessage 发送消息时，
 * 该方法会被调用，并将消息转发给回调处理器。
 */
void CWebView2Manager::RegisterWebMessageReceived()
{
    if (!m_webView)
        return;

    m_webView->add_WebMessageReceived(
        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
            {
                LPWSTR message = nullptr;
                args->TryGetWebMessageAsString(&message);

                if (message != nullptr && m_pMessageHandler)
                {
                    try
                    {
                        // 解析 JSON 消息
                        std::string utf8Str = CW2A(message, CP_UTF8);
                        json data = json::parse(utf8Str);

                        // 转发给回调处理器
                        m_pMessageHandler->OnWebMessageReceived(data);
                    }
                    catch (const std::exception& e)
                    {
                        // JSON 解析失败，输出调试信息
                        TRACE(_T("JSON 解析失败: %hs\n"), e.what());
                    }
                }

                return S_OK;
            }
            ).Get(),
                nullptr
                );
}

/**
 * @brief 设置页面底色
 */
void CWebView2Manager::SetDefaultBackgroundColor(COLORREF color)
{
    if (m_bInitialized && m_webController)
    {
        // 尝试获取 ICoreWebView2Controller2 接口（包含背景色设置）
        wil::com_ptr<ICoreWebView2Controller2> controller2;
        if (SUCCEEDED(m_webController->QueryInterface(IID_PPV_ARGS(&controller2))))
        {
            COREWEBVIEW2_COLOR webViewColor;
            webViewColor.R = GetRValue(color);
            webViewColor.G = GetGValue(color);
            webViewColor.B = GetBValue(color);
            webViewColor.A = 255;

            controller2->put_DefaultBackgroundColor(webViewColor);
        }
    }
}