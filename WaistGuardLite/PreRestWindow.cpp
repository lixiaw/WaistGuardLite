﻿// PreRestWindow.cpp
#include "PreRestWindow.h"
#include <strsafe.h>

HWND PreRestWindow::s_hwnd = NULL;
int PreRestWindow::s_remainingSeconds = 0;
UINT_PTR PreRestWindow::s_timer = 0;
bool PreRestWindow::s_isManualTriggered = false;
bool PreRestWindow::s_isDelayed = false;
const wchar_t PreRestWindow::PRE_REST_WINDOW_CLASS[] = L"WaistGuardLitePreRest";

void PreRestWindow::CreateControls(HWND hwnd)
{
    // 定义边距和间距
    const int LEFT_MARGIN = 30;
    const int TOP_MARGIN = 25;
    const int WINDOW_WIDTH = 400;
    const int CONTENT_WIDTH = WINDOW_WIDTH - (LEFT_MARGIN * 2);

    // 创建统一字体
    HFONT hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

    int currentY = TOP_MARGIN;

    // 创建图标
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WAISTGUARDLITE));
    if (hIcon)
    {
        HWND hIconCtrl = CreateWindow(L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_ICON,
            LEFT_MARGIN, currentY, 32, 32,
            hwnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessage(hIconCtrl, STM_SETICON, (WPARAM)hIcon, 0);
    }

    // 创建提示文本
    wchar_t text[64];
    swprintf_s(text, s_isManualTriggered ?
        L"即将开始休息：%d秒" :
        L"工作时间到了，%d秒后开始休息",
        s_remainingSeconds);

    HWND hTips = CreateWindow(L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_CENTER,  // 居中对齐
        LEFT_MARGIN, currentY + 50, CONTENT_WIDTH, 25,
        hwnd, NULL, GetModuleHandle(NULL), NULL);
    SendMessage(hTips, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 创建延迟按钮和关闭按钮
    const int BTN_WIDTH = 90;
    const int BTN_HEIGHT = 30;
    const int BTN_SPACING = 10;
    const int TOTAL_BTN_WIDTH = (BTN_WIDTH * 2) + BTN_SPACING;
    const int BTN_START_X = (WINDOW_WIDTH - TOTAL_BTN_WIDTH) / 2;

    // 延迟按钮
    HWND hDelayBtn = CreateWindow(L"BUTTON", L"延迟3分钟",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        BTN_START_X, currentY + 100,
        BTN_WIDTH, BTN_HEIGHT,
        hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
    SendMessage(hDelayBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
}

bool PreRestWindow::Create(bool isManual)
{
    if (s_hwnd != NULL)
        return false;

    // 注册窗口类
    RegisterWindowClass(GetModuleHandle(NULL));

    // 创建窗口
    s_hwnd = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,  // 扩展样式：对话框边框
        PRE_REST_WINDOW_CLASS,   // 窗口类名
        L"即将休息",             // 窗口标题
        WS_OVERLAPPEDWINDOW,    // 使用标准窗口样式，与设置界面保持一致
        0, 0,                    // 位置
        400, 200,               // 大小
        NULL, NULL,             // 父窗口和菜单
        GetModuleHandle(NULL),  // 实例句柄
        NULL                    // 附加数据
    );

    if (s_hwnd)
    {
        // 初始化
        s_isManualTriggered = isManual;
        s_remainingSeconds = isManual ? 5 : 10;  // 手动触发5秒，自动触发10秒
        s_isDelayed = false;
        s_timer = SetTimer(s_hwnd, 1, 1000, TimerProc);

        // 居中窗口
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        RECT rect;
        GetWindowRect(s_hwnd, &rect);
        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;
        int x = (screenWidth - windowWidth) / 2;
        int y = (screenHeight - windowHeight) / 2;
        SetWindowPos(s_hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // 创建控件
        CreateControls(s_hwnd);

        // 显示窗口
        ShowWindow(s_hwnd, SW_SHOW);
        UpdateWindow(s_hwnd);
        return true;
    }

    return false;
}

void PreRestWindow::RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = PRE_REST_WINDOW_CLASS;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WAISTGUARDLITE));  // 添加图标
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);  // 添加鼠标指针
    RegisterClass(&wc);
}

LRESULT CALLBACK PreRestWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 设置文本颜色和背景模式
        SetTextColor(hdc, RGB(51, 51, 51));
        SetBkMode(hdc, TRANSPARENT);

        // 创建字体
        HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        // 获取窗口客户区大小
        RECT rect;
        GetClientRect(hwnd, &rect);
        rect.top = 20;  // 调整文本位置

        // 绘制文本
        wchar_t text[64];
        swprintf_s(text, s_isManualTriggered ?
            L"即将开始休息：%d秒" :
            L"工作时间到了，%d秒后开始休息",
            s_remainingSeconds);
        DrawText(hdc, text, -1, &rect, DT_CENTER | DT_SINGLELINE);

        // 清理
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == 1001)  // 延迟按钮
        {
            s_isDelayed = true;
            Close();
        }
        else if (LOWORD(wParam) == IDCANCEL)  // 处理窗口关闭按钮
        {
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        s_hwnd = NULL;
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

VOID CALLBACK PreRestWindow::TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    s_remainingSeconds--;
    if (s_remainingSeconds <= 0)
    {
        Close();
        return;
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

void PreRestWindow::Close()
{
    if (s_hwnd)
    {
        KillTimer(s_hwnd, s_timer);
        DestroyWindow(s_hwnd);
        s_hwnd = NULL;
    }
}