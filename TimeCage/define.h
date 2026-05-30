#pragma once

// 版本号
#define VERSION _T("2.0")

// 程序名
#define APP_NAME _T("游戏冻结工具TimeCage")

// 页面底色，避免加载中的闪烁
#define WEBVIEW_COLOR    RGB(30, 34, 43)

// 页面宽度
#define WEBVIEW_WIDTH   1340
// 页面高度
#define WEBVIEW_HEIGHT  770
// 设计时的DPI
#define BASE_DPI        96 * 2.0 //测试页面是按照200%设计的

// 窗口四个角的弧度
#define ROUND_CORNER_RADIUS 16

// 快捷按钮数量
#define SHORTCUT_BUTTON_COUNT    4

// 框架与页面交互命令
namespace WebViewCmd
{
    const CString None = _T("WebViewCmd_None");
    const CString ShowMessageBox = _T("WebViewCmd_ShowMessageBox");                     //显示提示框
    const CString SetAppName = _T("WebViewCmd_SetAppName");                             //设置标题栏文本
    const CString SetShortCutButtonInfo = _T("WebViewCmd_SetShortCutButtonInfo");       //设置快捷按钮信息
    const CString FreezeGame = _T("WebViewCmd_FreezeGame");                             //冻结游戏 
    const CString ThawGame = _T("WebViewCmd_ThawGame");                                 //解冻游戏
    const CString ToggleOption = _T("WebViewCmd_ToggleOption");                         //切换选项
    const CString SysClose = _T("WebViewCmd_SysClose");                                 //退出程序
    const CString SysMinimize = _T("WebViewCmd_SysMinimize");                           //最小化窗口
}               


// 快捷按钮信息
struct ShortcutButtonData
{
    CString strProcessName;   // 进程名
    CString strComment;       // 描述
    CString strIconIndex;     // 图标索引
};