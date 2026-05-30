#pragma once

#include <afxwin.h>
#include "define.h"

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
    bool LoadShortcutButtons(ShortcutButtonData buttons[], int count);

    /**
     * @brief 保存快捷按钮数据到配置文件
     * @param buttons 要保存的按钮数据数组
     * @param count 按钮数量
     * @return 保存是否成功
     */
    bool SaveShortcutButtons(const ShortcutButtonData buttons[], int count);

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
};

