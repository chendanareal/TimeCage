// BossKeyManager.cpp: 老板键管理类实现
//

#include "pch.h"
#include "BossKeyManager.h"

CBossKeyManager::CBossKeyManager()
    : m_hWnd(NULL)
    , m_hHotKey(NULL)
    , m_bRegistered(FALSE)
{
}

CBossKeyManager::~CBossKeyManager()
{
    UnregisterBossKey();
}

void CBossKeyManager::SetHWnd(HWND hWnd)
{
    m_hWnd = hWnd;
    // 如果已有热键注册，重新注册
    if (m_bRegistered && !m_strHotKey.IsEmpty())
    {
        CString strHotKey = m_strHotKey;
        UnregisterBossKey();
        RegisterBossKey(strHotKey);
    }
}

BOOL CBossKeyManager::RegisterBossKey(LPCTSTR strHotKey)
{
    // 先注销之前的热键
    UnregisterBossKey();

    if (m_hWnd == NULL || strHotKey == NULL || _tcslen(strHotKey) == 0)
        return FALSE;

    UINT Modifiers = 0;
    UINT VirtualKey = 0;

    if (!ParseHotKeyString(strHotKey, Modifiers, VirtualKey))
        return FALSE;

    // 注册热键
    m_hHotKey = GlobalAddAtom(_T("TimeCageBossKey"));
    if (RegisterHotKey(m_hWnd, m_hHotKey, Modifiers, VirtualKey))
    {
        m_bRegistered = TRUE;
        m_strHotKey = strHotKey;
        TRACE(_T("老板键注册成功: %s\n"), strHotKey);
        return TRUE;
    }
    else
    {
        TRACE(_T("老板键注册失败: %s\n"), strHotKey);
        GlobalDeleteAtom(m_hHotKey);
        m_hHotKey = NULL;
        m_bRegistered = FALSE;
        m_strHotKey.Empty();
        return FALSE;
    }
}

void CBossKeyManager::UnregisterBossKey()
{
    if (m_bRegistered && m_hHotKey != NULL && m_hWnd != NULL)
    {
        UnregisterHotKey(m_hWnd, m_hHotKey);
        GlobalDeleteAtom(m_hHotKey);
        m_hHotKey = NULL;
        m_bRegistered = FALSE;
        m_strHotKey.Empty();
        TRACE(_T("老板键已注销\n"));
    }
}

BOOL CBossKeyManager::IsRegistered() const
{
    return m_bRegistered;
}

BOOL CBossKeyManager::OnHotKey(UINT nHotKeyId, UINT nReserved)
{
    if (nHotKeyId == m_hHotKey)
    {
        TRACE(_T("老板键触发: %s\n"), m_strHotKey);
        return TRUE;
    }
    return FALSE;
}

CString CBossKeyManager::GetHotKeyString() const
{
    return m_strHotKey;
}

BOOL CBossKeyManager::ParseHotKeyString(LPCTSTR strHotKey, UINT&Modifiers, UINT&VirtualKey)
{
    Modifiers = 0;
    VirtualKey = 0;

    if (strHotKey == NULL || _tcslen(strHotKey) == 0)
        return FALSE;

    // 分割字符串
    CStringArray arrParts;
    CString strTemp = strHotKey;
    CString strDelimiters = _T("+");

    int nPos = strTemp.Find(strDelimiters);
    while (nPos != -1)
    {
        CString strPart = strTemp.Left(nPos);
        strPart.Trim();
        if (!strPart.IsEmpty())
            arrParts.Add(strPart);
        strTemp = strTemp.Mid(nPos + 1);
        nPos = strTemp.Find(strDelimiters);
    }
    strTemp.Trim();
    if (!strTemp.IsEmpty())
        arrParts.Add(strTemp);

    if (arrParts.GetSize() == 0)
        return FALSE;

    // 最后一个部分是虚拟键码
    CString strKey = arrParts.GetAt(arrParts.GetSize() - 1);
    strKey.Trim();

    // 映射虚拟键码
    if (strKey.CompareNoCase(_T("A")) >= 0 && strKey.CompareNoCase(_T("Z")) <= 0)
    {
        VirtualKey = strKey[0];
    }
    else if (strKey.CompareNoCase(_T("0")) >= 0 && strKey.CompareNoCase(_T("9")) <= 0)
    {
        VirtualKey = strKey[0];
    }
    else if (strKey.CompareNoCase(_T("F1")) == 0) VirtualKey = VK_F1;
    else if (strKey.CompareNoCase(_T("F2")) == 0) VirtualKey = VK_F2;
    else if (strKey.CompareNoCase(_T("F3")) == 0) VirtualKey = VK_F3;
    else if (strKey.CompareNoCase(_T("F4")) == 0) VirtualKey = VK_F4;
    else if (strKey.CompareNoCase(_T("F5")) == 0) VirtualKey = VK_F5;
    else if (strKey.CompareNoCase(_T("F6")) == 0) VirtualKey = VK_F6;
    else if (strKey.CompareNoCase(_T("F7")) == 0) VirtualKey = VK_F7;
    else if (strKey.CompareNoCase(_T("F8")) == 0) VirtualKey = VK_F8;
    else if (strKey.CompareNoCase(_T("F9")) == 0) VirtualKey = VK_F9;
    else if (strKey.CompareNoCase(_T("F10")) == 0) VirtualKey = VK_F10;
    else if (strKey.CompareNoCase(_T("F11")) == 0) VirtualKey = VK_F11;
    else if (strKey.CompareNoCase(_T("F12")) == 0) VirtualKey = VK_F12;
    else if (strKey.CompareNoCase(_T("Space")) == 0) VirtualKey = VK_SPACE;
    else if (strKey.CompareNoCase(_T("Tab")) == 0) VirtualKey = VK_TAB;
    else if (strKey.CompareNoCase(_T("Enter")) == 0) VirtualKey = VK_RETURN;
    else if (strKey.CompareNoCase(_T("Esc")) == 0) VirtualKey = VK_ESCAPE;
    else if (strKey.CompareNoCase(_T("Backspace")) == 0) VirtualKey = VK_BACK;
    else if (strKey.CompareNoCase(_T("Delete")) == 0) VirtualKey = VK_DELETE;
    else if (strKey.CompareNoCase(_T("Up")) == 0) VirtualKey = VK_UP;
    else if (strKey.CompareNoCase(_T("Down")) == 0) VirtualKey = VK_DOWN;
    else if (strKey.CompareNoCase(_T("Left")) == 0) VirtualKey = VK_LEFT;
    else if (strKey.CompareNoCase(_T("Right")) == 0) VirtualKey = VK_RIGHT;
    else if (strKey.CompareNoCase(_T("Home")) == 0) VirtualKey = VK_HOME;
    else if (strKey.CompareNoCase(_T("End")) == 0) VirtualKey = VK_END;
    else if (strKey.CompareNoCase(_T("PageUp")) == 0) VirtualKey = VK_PRIOR;
    else if (strKey.CompareNoCase(_T("PageDown")) == 0) VirtualKey = VK_NEXT;
    else if (strKey.CompareNoCase(_T("Insert")) == 0) VirtualKey = VK_INSERT;
    else
        return FALSE;

    // 解析修饰键
    for (int i = 0; i < arrParts.GetSize() - 1; i++)
    {
        CString strMod = arrParts.GetAt(i);
        strMod.Trim();

        if (strMod.CompareNoCase(_T("Ctrl")) == 0 || strMod.CompareNoCase(_T("Control")) == 0)
            Modifiers |= MOD_CONTROL;
        else if (strMod.CompareNoCase(_T("Alt")) == 0)
            Modifiers |= MOD_ALT;
        else if (strMod.CompareNoCase(_T("Shift")) == 0)
            Modifiers |= MOD_SHIFT;
        else if (strMod.CompareNoCase(_T("Win")) == 0)
            Modifiers |= MOD_WIN;
    }

    return (Modifiers != 0 && VirtualKey != 0);
}
