// main.cpp
#include "main.h"
#include "PreRestWindow.h"
#include "Settings.h"
#include "About.h"
#include "RestWindow.h"
#include <strsafe.h>

// ȫ�ֱ�������
AppState g_appState = { 0 };

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // ��ʼ��Ĭ��ֵ
    g_appState.workDuration = 45;
    g_appState.breakDuration = 5;
    Settings::LoadSettings();  // ���ر��������

    // ע�ᴰ����
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WAISTGUARDLITE));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // ��������
    g_appState.hwnd = CreateWindowEx(
        0,                              // ��չ������ʽ
        CLASS_NAME,                     // ��������
        WINDOW_TITLE,                   // ���ڱ���
        WS_OVERLAPPEDWINDOW,           // ������ʽ
        CW_USEDEFAULT, CW_USEDEFAULT,  // λ��
        350, 200,                      // ��С
        NULL,                          // ������
        NULL,                          // �˵�
        hInstance,                     // ʵ�����
        NULL                           // ��������
    );

    if (g_appState.hwnd == NULL)
    {
        return 0;
    }

    // ���д���
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    RECT rect;
    GetWindowRect(g_appState.hwnd, &rect);
    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;
    SetWindowPos(g_appState.hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    ShowWindow(g_appState.hwnd, nCmdShow);
    UpdateWindow(g_appState.hwnd);

    // ��Ϣѭ��
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        // ��������ͼ��
        ZeroMemory(&g_appState.nid, sizeof(NOTIFYICONDATA));
        g_appState.nid.cbSize = sizeof(NOTIFYICONDATA);
        g_appState.nid.hWnd = hwnd;
        g_appState.nid.uID = 1;
        g_appState.nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        g_appState.nid.uCallbackMessage = WM_TRAYICON;
        g_appState.nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WAISTGUARDLITE));
        StringCchCopy(g_appState.nid.szTip, ARRAYSIZE(g_appState.nid.szTip), WINDOW_TITLE);
        Shell_NotifyIcon(NIM_ADD, &g_appState.nid);

        // ��ʼ����ʱ��
        GetSystemTime(&g_appState.startTime);
        g_appState.workTimer = SetTimer(hwnd, 1, g_appState.workDuration * 60 * 1000, NULL);
        g_appState.displayTimer = SetTimer(hwnd, 2, 1000, NULL);

        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // �����ı���ɫ�ͱ���ģʽ
        SetTextColor(hdc, RGB(51, 51, 51));
        SetBkMode(hdc, TRANSPARENT);

        // ��������
        HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        // ��ȡ���ڿͻ�����С
        RECT rect;
        GetClientRect(hwnd, &rect);

        // ���㹤��ʱ��
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

        ULONGLONG diff = (u2.QuadPart - u1.QuadPart) / 10000000;  // ת��Ϊ��
        int minutes = (int)(diff / 60);
        int seconds = (int)(diff % 60);

        // �����ı�
        wchar_t text[64];
        swprintf_s(text, L"�ѹ���ʱ����%02d:%02d", minutes, seconds);
        DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // ����
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_TIMER:
    {
        switch (wParam)
        {
        case 1:  // ������ʱ��
            if (!g_appState.isResting && !g_appState.isPreResting)
            {
                PreRestWindow::Create(false);  // ��ʾԤ��Ϣ����
            }
            break;

        case 2:  // ��ʾ��ʱ��
            InvalidateRect(hwnd, NULL, TRUE);  // ˢ����ʾ
            break;
        }
        return 0;
    }

    case WM_TRAYICON:
    {
        if (lParam == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, ID_TRAY_SHOW, L"��ʾ������");
            AppendMenu(hMenu, MF_STRING, ID_TRAY_REST, L"������Ϣ");
            AppendMenu(hMenu, MF_STRING, ID_TRAY_RESTART, L"���¼�ʱ");
            AppendMenu(hMenu, MF_STRING, ID_TRAY_SETTINGS, L"����");
            AppendMenu(hMenu, MF_STRING, ID_TRAY_ABOUT, L"����");
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"�˳�");

            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        else if (lParam == WM_LBUTTONDBLCLK)
        {
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
        }
        return 0;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case ID_TRAY_SHOW:
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
            break;

        case ID_TRAY_REST:
            PreRestWindow::Create(true);  // �ֶ�������Ϣ
            break;

        case ID_TRAY_RESTART:
            GetSystemTime(&g_appState.startTime);
            g_appState.isResting = false;
            g_appState.isPreResting = false;
            InvalidateRect(hwnd, NULL, TRUE);
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
        return 0;
    }

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);  // ����رհ�ťʱ���ش��ڶ������˳�
        return 0;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &g_appState.nid);  // ɾ������ͼ��
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}