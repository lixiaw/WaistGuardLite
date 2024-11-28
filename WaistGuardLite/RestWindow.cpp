// RestWindow.cpp
#include "RestWindow.h"
#include <strsafe.h>

HWND RestWindow::s_hwnd = NULL;
int RestWindow::s_remainingSeconds = 0;
UINT_PTR RestWindow::s_timer = 0;
int RestWindow::s_currentTipIndex = 0;

// ��Ϣ��ʾ��
const wchar_t* RestWindow::s_tips[] = {
    L"������������������ʱ���ƣ�����׵��´����ʧ��...",
    L"�����������������ʵ���Ϣ������߹���Ч��...",
    L"����������������վ�����һ�£������屣�ֻ���...",
    L"�����������������ǵö��ˮ����������׵..."
};

bool RestWindow::Create(int duration)
{
    if (s_hwnd != NULL)
        return false;

    // ע�ᴰ����
    RegisterWindowClass(GetModuleHandle(NULL));

    // ��ȡ��Ļ�ߴ�
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // ����ȫ������
    s_hwnd = CreateWindowEx(
        WS_EX_TOPMOST,           // ������ǰ
        REST_WINDOW_CLASS,       // ��������
        L"��Ϣʱ��",             // ���ڱ���
        WS_POPUP,               // �ޱ߿򴰿�
        0, 0,                   // λ��
        screenWidth,            // ���
        screenHeight,           // �߶�
        NULL, NULL,             // �����ںͲ˵�
        GetModuleHandle(NULL),  // ʵ�����
        NULL                    // ��������
    );

    if (s_hwnd)
    {
        // ��ʼ��
        s_remainingSeconds = duration * 60;
        s_currentTipIndex = 0;
        s_timer = SetTimer(s_hwnd, 1, 1000, TimerProc);

        // ��ʾ����
        ShowWindow(s_hwnd, SW_SHOW);
        UpdateWindow(s_hwnd);

        // ���ü���
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

        // �����ı���ɫ�ͱ���ģʽ
        SetTextColor(hdc, RGB(236, 65, 65));  // �����ƺ�ɫ
        SetBkMode(hdc, TRANSPARENT);

        // ������������
        HFONT hTitleFont = CreateFont(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

        // ��������ʱ����
        HFONT hCountdownFont = CreateFont(72, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

        // ������ʾ�ı�����
        HFONT hTipsFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei");

        // ��ȡ���ڿͻ�����С
        RECT rect;
        GetClientRect(hwnd, &rect);

        // ���Ʊ���
        HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);
        RECT titleRect = rect;
        titleRect.top = rect.bottom / 3;
        DrawText(hdc, L"������������������Ϣ����ʱ��", -1, &titleRect, DT_CENTER | DT_SINGLELINE);

        // ���Ƶ���ʱ
        SelectObject(hdc, hCountdownFont);
        RECT countdownRect = rect;
        countdownRect.top = titleRect.top + 50;
        wchar_t countdownText[32];
        swprintf_s(countdownText, L"%02d:%02d", s_remainingSeconds / 60, s_remainingSeconds % 60);
        DrawText(hdc, countdownText, -1, &countdownRect, DT_CENTER | DT_SINGLELINE);

        // ������ʾ�ı�
        SetTextColor(hdc, RGB(51, 51, 51));  // ���ɫ
        SelectObject(hdc, hTipsFont);
        RECT tipsRect = rect;
        tipsRect.top = countdownRect.top + 100;
        tipsRect.left = rect.right / 4;
        tipsRect.right = rect.right * 3 / 4;
        DrawText(hdc, s_tips[s_currentTipIndex], -1, &tipsRect, DT_CENTER | DT_WORDBREAK);

        // ����
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
        return 0;  // �������м�������
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

    if (s_remainingSeconds % 5 == 0)  // ÿ5�����һ����ʾ��
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

// ���ȫ�ֱ���
HHOOK g_keyboardHook = NULL;
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        return 1; // ��ֹ���м�������
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// RestWindow.cpp �еļ��̽��ù���
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