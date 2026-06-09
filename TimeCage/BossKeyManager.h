// BossKeyManager.h: 老板键管理类头文件
//

#pragma once

#include <afxwin.h>

// 老板键管理类
class CBossKeyManager
{
public:
    CBossKeyManager();
    ~CBossKeyManager();

    // 设置窗口句柄
    void SetHWnd(HWND hWnd);

    // 注册老板键
    // strHotKey: 组合键字符串，如 "Ctrl+Shift+F1"
    // 返回: 是否注册成功
    BOOL RegisterBossKey(LPCTSTR strHotKey);

    // 注销老板键
    void UnregisterBossKey();

    // 检查是否已注册
    BOOL IsRegistered() const;

    // 处理热键消息
    // nHotKeyId: 热键ID
    // 返回: 是否是我们的热键
    BOOL OnHotKey(UINT nHotKeyId, UINT nReserved = 0);

    // 获取当前注册的热键字符串
    CString GetHotKeyString() const;

private:
    // 解析组合键字符串
    // strHotKey: 组合键字符串
    // Modifiers: 输出，修饰键（MOD_CONTROL/MOD_ALT/MOD_SHIFT/MOD_WIN）
    // VirtualKey: 输出，虚拟键码
    // 返回: 是否解析成功
    static BOOL ParseHotKeyString(LPCTSTR strHotKey, UINT&Modifiers, UINT&VirtualKey);

private:
    HWND m_hWnd;           // 窗口句柄
    ATOM m_hHotKey;        // 热键原子
    BOOL m_bRegistered;    // 是否已注册
    CString m_strHotKey;   // 热键字符串
};
