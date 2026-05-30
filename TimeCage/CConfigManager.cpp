#include "pch.h"
#include "ConfigManager.h"
#include <shlobj.h>

/**
 * @brief 构造函数
 */
CConfigManager::CConfigManager()
{
}

/**
 * @brief 析构函数
 */
CConfigManager::~CConfigManager()
{
}

/**
 * @brief 获取配置文件的完整路径
 *
 * 首先尝试获取 AppData\Roaming\ 目录，如果失败则回退到
 * 用户的 Documents\ 目录，然后在该目录下创建 TimeCage 子目录。
 *
 * @return 配置文件的完整路径
 */
CString CConfigManager::GetConfigFilePath()
{
    CString strConfigPath;
    PWSTR pwszPath = NULL;

    // 获取 AppData\Roaming 目录
    HRESULT hr = SHGetKnownFolderPath(
        FOLDERID_RoamingAppData,
        KF_FLAG_CREATE,
        NULL,
        &pwszPath
    );

    if (SUCCEEDED(hr))
    {
        strConfigPath = pwszPath;
        CoTaskMemFree(pwszPath);
    }
    else
    {
        // 回退方案：获取 Documents 目录
        hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, NULL, &pwszPath);
        if (SUCCEEDED(hr))
        {
            strConfigPath = pwszPath;
            CoTaskMemFree(pwszPath);
        }
    }

    // 拼接子目录
    strConfigPath += _T("\\TimeCage\\");

    // 确保目录存在
    CreateDirectory(strConfigPath, NULL);

    // 拼接文件名
    strConfigPath += _T("ShortCutConfig.ini");

    return strConfigPath;
}

/**
 * @brief 从配置文件加载快捷按钮数据
 * @param buttons 输出参数，用于接收加载的按钮数据
 * @param count 按钮数量
 * @return 加载是否成功
 */
bool CConfigManager::LoadShortcutButtons(ShortcutButtonData buttons[], int count)
{
    // 参数校验
    if (buttons == nullptr || count <= 0)
        return false;

    CString strIniPath = GetConfigFilePath();

    // 逐个加载按钮配置
    for (int i = 0; i < count; i++)
    {
        CString strSection;
        strSection.Format(_T("Shortcut%d"), i + 1);  // Shortcut1、Shortcut2...

        buttons[i].strProcessName = ReadString(strSection, _T("ProcessName"), _T(""), strIniPath);
        buttons[i].strComment = ReadString(strSection, _T("Comment"), _T(""), strIniPath);
        buttons[i].strIconIndex = ReadString(strSection, _T("IconIndex"), _T("0"), strIniPath);
    }

    return true;
}

/**
 * @brief 保存快捷按钮数据到配置文件
 * @param buttons 要保存的按钮数据数组
 * @param count 按钮数量
 * @return 保存是否成功
 */
bool CConfigManager::SaveShortcutButtons(const ShortcutButtonData buttons[], int count)
{
    // 参数校验
    if (buttons == nullptr || count <= 0)
        return false;

    CString strIniPath = GetConfigFilePath();

    // 逐个保存按钮配置
    for (int i = 0; i < count; i++)
    {
        CString strSection;
        strSection.Format(_T("Shortcut%d"), i + 1);

        WriteString(strSection, _T("ProcessName"), buttons[i].strProcessName, strIniPath);
        WriteString(strSection, _T("Comment"), buttons[i].strComment, strIniPath);
        WriteString(strSection, _T("IconIndex"), buttons[i].strIconIndex, strIniPath);
    }

    return true;
}

/**
 * @brief 从配置文件读取字符串值
 * @param section INI 节名
 * @param key INI 键名
 * @param defaultValue 默认值，当键不存在时返回
 * @param filePath 配置文件路径
 * @return 读取到的字符串值
 */
CString CConfigManager::ReadString(LPCTSTR section, LPCTSTR key, LPCTSTR defaultValue, LPCTSTR filePath)
{
    TCHAR szBuffer[256] = { 0 };
    GetPrivateProfileString(section, key, defaultValue, szBuffer, _countof(szBuffer), filePath);
    return CString(szBuffer);
}

/**
 * @brief 向配置文件写入字符串值
 * @param section INI 节名
 * @param key INI 键名
 * @param value 要写入的字符串值
 * @param filePath 配置文件路径
 * @return 写入是否成功
 */
bool CConfigManager::WriteString(LPCTSTR section, LPCTSTR key, LPCTSTR value, LPCTSTR filePath)
{
    return WritePrivateProfileString(section, key, value, filePath) != FALSE;
}

