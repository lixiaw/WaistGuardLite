// RestWindow.cpp
#include "RestWindow.h"
#include <strsafe.h>

HWND RestWindow::s_hwnd = NULL;
int RestWindow::s_remainingSeconds = 0;
UINT_PTR RestWindow::s_timer = 0;
int RestWindow::s_currentTipIndex = 0;

// 休息提示语
const wchar_t* RestWindow::s_tips[] = {
    L"护腰神器提醒您：长时间的疲劳容易导致错误和失误...",
    L"护腰神器提醒您：适当休息可以提高工作效率...",
    L"护腰神器提醒您：站起来活动一下，让身体保持活力...",
    L"护腰神器提醒您：记得多喝水，保护好腰椎..."
};

bool RestWindow::Create(int duration)
{
    if (s_hwnd != NULL)
        return false;

    // 注册窗口类
    RegisterWindowClass(GetModuleHandle(NULL));

    // 获取屏幕尺寸
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 创建全屏窗口
    s_hwnd = CreateWindowEx(
        WS_EX_TOPMOST,           // 总在最前
        REST_WINDOW_CLASS,       // 窗口类名
        L"休息时间",             // 窗口标题
        WS_POPUP,               // 无边框窗口
        0, 0,                   // 位置
        screenWidth,            // 宽度
        screenHeight,           // 高度
        NULL, NULL,             // 父窗口和菜单
        GetModuleHandle(NULL),  // 实例句柄
        NULL                    // 附加数据
    );

    if (s_hwnd)
    {
        // 初始化
        s_remainingSeconds = duration * 60;
        s_currentTipIndex = 0;
        s_timer = SetTimer(s_hwnd, 1, 1000, TimerProc);

        // 显示窗口
        ShowWindow(s_hwnd, SW_SHOW);
        UpdateWindow(s_hwnd);

        // 禁用键盘
        DisableKeyboard();
        return true;
    }

    return false;
}

void RestWindow::RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = REST_WINDOW_CLASS;
    wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
    RegisterClass(&wc);
}

LRESULT CALLBACK RestWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 设置文本颜色和背景模式
        SetTextColor(hdc, RGB(236, 65, 65));  // 网易云红色
        SetBkMode(hdc, TRANSPARENT);

        // 创建标题字体
        HFONT hTitleFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

        // 创建倒计时字体
        HFONT hCountdownFont = CreateFont(72, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

        // 创建提示文本字体
        HFONT hTipsFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

        // 获取窗口客户区大小
        RECT rect;
        GetClientRect(hwnd, &rect);

        // 绘制标题
        HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
        RECT titleRect = rect;
        titleRect.top = rect.bottom / 3;
        DrawText(hdc, L"护腰神器提醒您：休息结束时间", -1, &titleRect, DT_CENTER | DT_SINGLELINE);

        // 绘制倒计时
        SelectObject(hdc, hCountdownFont);
        RECT countdownRect = rect;
        countdownRect.top = titleRect.top + 50;
        wchar_t countdownText[32];
        swprintf_s(countdownText, L"%02d:%02d", s_remainingSeconds / 60, s_remainingSeconds % 60);
        DrawText(hdc, countdownText, -1, &countdownRect, DT_CENTER | DT_SINGLELINE);

        // 绘制提示文本
        SetTextColor(hdc, RGB(51, 51, 51));  // 深灰色
        SelectObject(hdc, hTipsFont);
        RECT tipsRect = rect;
        tipsRect.top = countdownRect.top + 100;
        tipsRect.left = rect.right / 4;
        tipsRect.right = rect.right * 3 / 4;
        DrawText(hdc, s_tips[s_currentTipIndex], -1, &tipsRect, DT_CENTER | DT_WORDBREAK);

        // 清理
        SelectObject(hdc, hOldFont);
        DeleteObject(hTitleFont);
        DeleteObject(hCountdownFont);
        DeleteObject(hTipsFont);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        return 0;  // 忽略所有键盘输入
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

VOID CALLBACK RestWindow::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    s_remainingSeconds--;
    if (s_remainingSeconds <= 0)
    {
        Close();
        return;
    }

    if (s_remainingSeconds % 5 == 0)  // 每5秒更新一次提示语
    {
        s_currentTipIndex = (s_currentTipIndex + 1) % (sizeof(s_tips) / sizeof(s_tips[0]));
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

void RestWindow::Close()
{
    if (s_hwnd)
    {
        KillTimer(s_hwnd, s_timer);
        EnableKeyboard();
        DestroyWindow(s_hwnd);
        s_hwnd = NULL;
    }
}

// 添加全局变量
HHOOK g_keyboardHook = NULL;
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        return 1; // 阻止所有键盘输入
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// RestWindow.cpp 中的键盘禁用功能
void RestWindow::DisableKeyboard()
{
    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc,
        GetModuleHandle(NULL), 0);
}

void RestWindow::EnableKeyboard()
{
    if (g_keyboardHook)
    {
        UnhookWindowsHookEx(g_keyboardHook);
        g_keyboardHook = NULL;
    }
}