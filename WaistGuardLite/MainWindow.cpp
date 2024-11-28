﻿// src/MainWindow.cpp
#include "MainWindow.h"
#include <strsafe.h>

bool MainWindow::Create(HINSTANCE hInstance)
{
    // 注册窗口类
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WAISTGUARDLITE));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(APP_BG_COLOR);
    RegisterClass(&wc);

    // 创建窗口
    g_appState.hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 225,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!g_appState.hwnd)
        return false;

    // 初始化应用状态
    g_appState.workDuration = DEFAULT_WORK_MINUTES;
    g_appState.breakDuration = DEFAULT_BREAK_MINUTES;

    // 设置休息窗口的定时器回调
    RestWindow::SetTimerCallbacks(WorkTimerProc, DisplayTimerProc);

    return true;
}

void MainWindow::InitTimers()
{
    TimerManager::StartTimer();
}

void MainWindow::CreateTrayIcon()
{
    ZeroMemory(&g_appState.nid, sizeof(NOTIFYICONDATA));
    g_appState.nid.cbSize = sizeof(NOTIFYICONDATA);
    g_appState.nid.hWnd = g_appState.hwnd;
    g_appState.nid.uID = 1;
    g_appState.nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
    g_appState.nid.uCallbackMessage = WM_TRAYICON;
    g_appState.nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WAISTGUARDLITE));
    StringCchCopy(g_appState.nid.szTip, ARRAYSIZE(g_appState.nid.szTip), WINDOW_TITLE);
    Shell_NotifyIcon(NIM_ADD, &g_appState.nid);
}

void MainWindow::UpdateTrayIcon()
{
    // 根据状态更新托盘图标提示文本
    if (g_appState.isResting)
    {
        StringCchCopy(g_appState.nid.szTip, ARRAYSIZE(g_appState.nid.szTip), L"正在休息...");
    }
    else if (g_appState.isPreResting)
    {
        StringCchCopy(g_appState.nid.szTip, ARRAYSIZE(g_appState.nid.szTip), L"即将开始休息...");
    }
    else
    {
        StringCchCopy(g_appState.nid.szTip, ARRAYSIZE(g_appState.nid.szTip), WINDOW_TITLE);
    }
    Shell_NotifyIcon(NIM_MODIFY, &g_appState.nid);
}

void MainWindow::UpdateWorkTime()
{
    SYSTEMTIME currentTime;
    GetSystemTime(&currentTime);

    // 计算工作时长
    FILETIME ft1, ft2;
    SystemTimeToFileTime(&g_appState.startTime, &ft1);
    SystemTimeToFileTime(&currentTime, &ft2);

    ULARGE_INTEGER u1, u2;
    u1.LowPart = ft1.dwLowDateTime;
    u1.HighPart = ft1.dwHighDateTime;
    u2.LowPart = ft2.dwLowDateTime;
    u2.HighPart = ft2.dwHighDateTime;

    ULONGLONG diff = (u2.QuadPart - u1.QuadPart) / 10000000;  // 转换为秒
    int minutes = (int)(diff / 60);
    int seconds = (int)(diff % 60);

    // 更新窗口标题
    wchar_t titleText[64];
    StringCchPrintf(titleText, ARRAYSIZE(titleText), L"%s — By：程序员七平", WINDOW_TITLE);
    SetWindowText(g_appState.hwnd, titleText);

    // 更新客户区显示
    RECT rect;
    GetClientRect(g_appState.hwnd, &rect);
    rect.bottom -= 25;  // 为状态栏留出空间
    InvalidateRect(g_appState.hwnd, &rect, TRUE);
}

VOID CALLBACK MainWindow::DisplayTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    UpdateWorkTime();
}

VOID CALLBACK MainWindow::WorkTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (!g_appState.isResting && !g_appState.isPreResting)
    {
        ShowRestWindow(false);
    }
}

void MainWindow::ShowRestWindow(bool isManualTrigger)
{
    if (!g_appState.isResting && !g_appState.isPreResting)
    {
        // 停止当前计时器
        KillTimer(g_appState.hwnd, g_appState.workTimer);
        KillTimer(g_appState.hwnd, g_appState.displayTimer);

        // 显示预休息窗口
        g_appState.isPreResting = true;
        if (PreRestWindow::Create(isManualTrigger))
        {
            // 预休息窗口会自行处理后续逻辑
            // 不需要在这里处理休息窗口的显示
            
            // 更新托盘图标状态
            StringCchCopy(g_appState.nid.szTip, ARRAYSIZE(g_appState.nid.szTip), L"即将开始休息...");
            UpdateTrayIcon();
        }
        else
        {
            // 如果预休息窗口创建失败，重置状态
            g_appState.isPreResting = false;
            RestartTimer();
        }
    }
}

void MainWindow::ShowTrayMenu(HWND hwnd, POINT pt)
{
    HMENU hMenu = CreatePopupMenu();
    
    // 添加菜单项并根据状态设置
    InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_SHOW, L"显示主窗口");
    InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING | (g_appState.isResting ? MF_GRAYED : 0), 
        ID_TRAY_REST, L"立即休息");
    InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING, ID_TRAY_RESTART, L"重新计时");
    InsertMenu(hMenu, 3, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenu(hMenu, 4, MF_BYPOSITION | MF_STRING, ID_TRAY_SETTINGS, L"设置");
    InsertMenu(hMenu, 5, MF_BYPOSITION | MF_STRING, ID_TRAY_ABOUT, L"关于");
    InsertMenu(hMenu, 6, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenu(hMenu, 7, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, L"退出");

    // 设置默认菜单项
    SetMenuDefaultItem(hMenu, ID_TRAY_SHOW, FALSE);

    // 确保窗口处于前台
    SetForegroundWindow(hwnd);

    // 显示菜单
    TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
        pt.x, pt.y, 0, hwnd, NULL);

    // 清理
    PostMessage(hwnd, WM_NULL, 0, 0);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hStatus;  // 将变量声明移到 switch 外部
    RECT rect;
    POINT pt;

    switch (uMsg)
    {
    case WM_CREATE:
    {
        // 创建托盘图标
        ZeroMemory(&g_appState.nid, sizeof(NOTIFYICONDATA));
        g_appState.nid.cbSize = sizeof(NOTIFYICONDATA);
        g_appState.nid.hWnd = hwnd;
        g_appState.nid.uID = 1;
        g_appState.nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        g_appState.nid.uCallbackMessage = WM_TRAYICON;
        g_appState.nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WAISTGUARDLITE));
        StringCchCopy(g_appState.nid.szTip, ARRAYSIZE(g_appState.nid.szTip), WINDOW_TITLE);
        Shell_NotifyIcon(NIM_ADD, &g_appState.nid);

        // 初始化定时器
        GetSystemTime(&g_appState.startTime);
        g_appState.workTimer = SetTimer(hwnd, 1, g_appState.workDuration * 60 * 1000, WorkTimerProc);
        g_appState.displayTimer = SetTimer(hwnd, 2, 1000, DisplayTimerProc);

        // 初始化通用控件
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_BAR_CLASSES;
        InitCommonControlsEx(&icex);

        // 创建状态栏
        hStatus = CreateWindowEx(
            0,
            STATUSCLASSNAME,
            NULL,
            WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0,
            hwnd,
            (HMENU)IDC_STATUSBAR,
            GetModuleHandle(NULL),
            NULL);

        if (hStatus)
        {
            // 设置状态栏文本
            const wchar_t* tipText = L"双击托盘图标显示主窗口 | 右键托盘图标可设置、立即休息等";
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)tipText);
        }
        return 0;
    }

    case WM_COMMAND:
        // 处理菜单命令
        HandleTrayCommand(hwnd, wParam);
        return 0;

    case WM_TRAYICON:
        switch (lParam)
        {
        case WM_RBUTTONUP:
        case WM_CONTEXTMENU:
            GetCursorPos(&pt);
            ShowTrayMenu(hwnd, pt);
            return 0;

        case WM_LBUTTONDBLCLK:
            ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_RESTORE);
            if (IsWindowVisible(hwnd))
            {
                SetForegroundWindow(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }
        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }

        GetClientRect(hwnd, &rect);
        hStatus = GetDlgItem(hwnd, IDC_STATUSBAR);
        if (hStatus)
        {
            SetWindowPos(hStatus, HWND_BOTTOM, 0, rect.bottom - 25, rect.right, 25, SWP_NOACTIVATE);
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 设置文本颜色和背景模式
        SetTextColor(hdc, RGB(51, 51, 51));
        SetBkMode(hdc, TRANSPARENT);

        // 计算当前工作时长
        SYSTEMTIME currentTime;
        GetSystemTime(&currentTime);
        FILETIME ft1, ft2;
        SystemTimeToFileTime(&g_appState.startTime, &ft1);
        SystemTimeToFileTime(&currentTime, &ft2);

        ULARGE_INTEGER u1, u2;
        u1.LowPart = ft1.dwLowDateTime;
        u1.HighPart = ft1.dwHighDateTime;
        u2.LowPart = ft2.dwLowDateTime;
        u2.HighPart = ft2.dwHighDateTime;

        ULONGLONG diff = (u2.QuadPart - u1.QuadPart) / 10000000;  // 转换为秒
        int minutes = (int)(diff / 60);
        int seconds = (int)(diff % 60);

        // 创建字体
        HFONT hFont = CreateFont(32, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,  // 增大字体
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        // 获取窗口客户区大小并调整绘制区域
        GetClientRect(hwnd, &rect);
        rect.bottom -= 25;  // 为状态栏留出空间
        rect.top += 10;     // 上方留出一些空间

        // 绘制工作时长
        wchar_t timeText[64];
        swprintf_s(timeText, L"已工作时长：%02d:%02d", minutes, seconds);
        DrawText(hdc, timeText, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // 清理
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_GETMINMAXINFO:
    {
        // 设置窗口最小尺寸
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = 400;  // 最小宽度从500改为400
        mmi->ptMinTrackSize.y = 225;  // 最小高度保持不变
        return 0;
    }

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &g_appState.nid);
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);  // 点击关闭按钮时隐藏窗口而不是退出
        return 0;

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH hBrush = CreateSolidBrush(APP_BG_COLOR);
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
        return TRUE;
    }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void MainWindow::HandleTrayCommand(HWND hwnd, WPARAM wParam)
{
    switch (LOWORD(wParam))
    {
    case ID_TRAY_SHOW:
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
        break;

    case ID_TRAY_REST:
        if (!g_appState.isResting && !g_appState.isPreResting)
        {
            // 停止当前计时器
            KillTimer(g_appState.hwnd, g_appState.workTimer);
            KillTimer(g_appState.hwnd, g_appState.displayTimer);
            
            // 显示预休息窗口，标记为手动触发
            g_appState.isPreResting = true;
            if (PreRestWindow::Create(true))  // true 表示手动触发
            {
                // 预休息窗口会自行处理后续逻辑
                // 不需要在这里处理休息窗口的显示
            }
        }
        break;

    case ID_TRAY_RESTART:
        RestartTimer();
        break;

    case ID_TRAY_SETTINGS:
        Settings::Create(hwnd);
        break;

    case ID_TRAY_ABOUT:
        About::Create(hwnd);
        break;

    case ID_TRAY_EXIT:
        DestroyWindow(hwnd);
        break;
    }
}

void MainWindow::RestartTimer()
{
    TimerManager::RestartTimer();
}