#pragma once

#include <afxwin.h>

// 快捷按钮信息
struct ShortcutButtonData
{
    CString strProcessName;   // 进程名
    CString strComment;       // 描述
    CString strIconIndex;     // 图标索引
};


// 应用配置信息
struct ConfigData
{
    ConfigData()
    {
        bHideWindow = TRUE;
        bHideWindowEnhanced = FALSE;
        bSingleInstance = TRUE;
        bMinimizeToTray = TRUE;
        bBossKeyEnabled = FALSE;
        strBossKey = _T("");
        bAutoStart = FALSE;
    }

    BOOL bHideWindow;               // 冻结时自动隐藏窗口
    BOOL bHideWindowEnhanced;       // 隐藏窗口增强
    BOOL bSingleInstance;           // 单实例
    BOOL bMinimizeToTray;           // 隐藏到托盘区
    BOOL bBossKeyEnabled;           // 是否启用老板键
    CString strBossKey;             // 老板键组合键
    BOOL bAutoStart;                // 开机自动启动
};

/**
 * @brief 配置管理器 - 负责 INI 配置文件的读写
 *
 * 该类用于管理应用程序的配置文件，特别是快捷按钮的配置信息。
 * 配置文件存储在用户的 AppData\Roaming\TimeCage\ 目录下。
 */
class CConfigManager
{
public:
    /**
     * @brief 构造函数
     */
    CConfigManager();

    /**
     * @brief 析构函数
     */
    ~CConfigManager();

    /**
     * @brief 获取配置文件的完整路径
     * @return 配置文件路径字符串
     */
    static CString GetConfigFilePath();

    /**
     * @brief 从配置文件加载快捷按钮数据
     * @param buttons 输出参数，用于接收加载的按钮数据
     * @param count 按钮数量
     * @return 加载是否成功
     */
    static bool LoadShortcutButtons(ShortcutButtonData buttons[], int count);

    /**
     * @brief 保存快捷按钮数据到配置文件
     * @param buttons 要保存的按钮数据数组
     * @param count 按钮数量
     * @return 保存是否成功
     */
    static bool SaveShortcutButtons(const ShortcutButtonData buttons[], int count);

    /**
     * @brief 加载应用程序设置
     * @param configData 输出参数，配置信息
     * @return 加载是否成功
     */
    static bool LoadAppSettings(ConfigData& configData);


    /**
     * @brief 保存应用程序设置
     * @param configData 配置信息
     * @return 保存是否成功
     */
    static bool SaveAppSettings(const ConfigData& configData);

    /**
     * @brief 从配置文件加载其它配置
     * @param nLastSelShortButtonIndex 输出参数，最后一次选择快捷按钮序号
     * @return 加载是否成功
     */
    static bool LoadOther(int &nLastSelShortButtonIndex);

    /**
     * @brief 保存其它配置
     * @param nLastSelShortButtonIndex 最后一次选择快捷按钮序号
     * @return 保存是否成功
     */
    static bool SaveOther(int nLastSelShortButtonIndex);

private:
    /**
     * @brief 从配置文件读取字符串值
     * @param section INI 节名
     * @param key INI 键名
     * @param defaultValue 默认值，当键不存在时返回
     * @param filePath 配置文件路径
     * @return 读取到的字符串值
     */
    static CString ReadString(LPCTSTR section, LPCTSTR key, LPCTSTR defaultValue, LPCTSTR filePath);

    /**
     * @brief 向配置文件写入字符串值
     * @param section INI 节名
     * @param key INI 键名
     * @param value 要写入的字符串值
     * @param filePath 配置文件路径
     * @return 写入是否成功
     */
    static bool WriteString(LPCTSTR section, LPCTSTR key, LPCTSTR value, LPCTSTR filePath);

    /**
     * @brief 从配置文件读取布尔值
     * @param section INI 节名
     * @param key INI 键名
     * @param defaultValue 默认值，当键不存在时返回
     * @param filePath 配置文件路径
     * @return 读取到的布尔值
     */
    static BOOL ReadBool(LPCTSTR section, LPCTSTR key, BOOL defaultValue, LPCTSTR filePath);

    /**
     * @brief 向配置文件写入布尔值
     * @param section INI 节名
     * @param key INI 键名
     * @param value 要写入的布尔值
     * @param filePath 配置文件路径
     * @return 写入是否成功
     */
    static bool WriteBool(LPCTSTR section, LPCTSTR key, BOOL value, LPCTSTR filePath);

    /**
     * @brief 从配置文件读取整数
     * @param section INI 节名
     * @param key INI 键名
     * @param defaultValue 默认值，当键不存在时返回
     * @param filePath 配置文件路径
     * @return 读取到的整数
     */
    static int ReadInt(LPCTSTR section, LPCTSTR key, int defaultValue, LPCTSTR filePath);

    /**
     * @brief 向配置文件写入整数
     * @param section INI 节名
     * @param key INI 键名
     * @param value 要写入的整数
     * @param filePath 配置文件路径
     * @return 写入是否成功
     */
    static bool WriteInt(LPCTSTR section, LPCTSTR key, int value, LPCTSTR filePath);
};

