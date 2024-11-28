// PreRestWindow.cpp
#include "PreRestWindow.h"
#include <strsafe.h>

HWND PreRestWindow::s_hwnd = NULL;
int PreRestWindow::s_remainingSeconds = 0;
UINT_PTR PreRestWindow::s_timer = 0;
bool PreRestWindow::s_isManualTriggered = false;
bool PreRestWindow::s_isDelayed = false;

bool PreRestWindow::Create(bool isManual)
{
    if (s_hwnd != NULL)
        return false;

    // ע�ᴰ����
    RegisterWindowClass(GetModuleHandle(NULL));

    // ��������
    s_hwnd = CreateWindowEx(
        WS_EX_TOPMOST,           // ������ǰ
        PRE_REST_WINDOW_CLASS,   // ��������
        L"������Ϣ",             // ���ڱ���
        WS_POPUP | WS_VISIBLE,   // ������ʽ
        0, 0,                    // λ��
        350, 200,               // ��С
        NULL, NULL,             // �����ںͲ˵�
        GetModuleHandle(NULL),  // ʵ�����
        NULL                    // ��������
    );

    if (s_hwnd)
    {
        // ��ʼ��
        s_isManualTriggered = isManual;
        s_remainingSeconds = isManual ? 5 : 10;  // �ֶ�����5�룬�Զ�����10��
        s_isDelayed = false;
        s_timer = SetTimer(s_hwnd, 1, 1000, TimerProc);

        // ���д���
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        RECT rect;
        GetWindowRect(s_hwnd, &rect);
        int windowWidth = rect.right - rect.left;
        int windowHeight = rect.bottom - rect.top;
        int x = (screenWidth - windowWidth) / 2;
        int y = (screenHeight - windowHeight) / 2;
        SetWindowPos(s_hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // �����ӳٰ�ť
        CreateDelayButton(s_hwnd);

        // ��ʾ����
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
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
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
        rect.top = 20;  // �����ı�λ��

        // �����ı�
        wchar_t text[64];
        swprintf_s(text, s_isManualTriggered ?
            L"������ʼ��Ϣ��%d��" :
            L"����ʱ�䵽�ˣ�%d���ʼ��Ϣ",
            s_remainingSeconds);
        DrawText(hdc, text, -1, &rect, DT_CENTER | DT_SINGLELINE);

        // ����
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == 1001)  // �ӳٰ�ť
        {
            s_isDelayed = true;
            Close();
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

void PreRestWindow::CreateDelayButton(HWND hwnd)
{
    // �����ӳٰ�ť
    CreateWindow(
        L"BUTTON",              // ��ť��
        L"�ӳ�3����",           // ��ť�ı�
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,  // ��ʽ
        125,                    // X λ��
        100,                    // Y λ��
        100,                    // ���
        30,                     // �߶�
        hwnd,                   // ������
        (HMENU)1001,           // ��ť ID
        GetModuleHandle(NULL),  // ʵ�����
        NULL                    // ��������
    );
}