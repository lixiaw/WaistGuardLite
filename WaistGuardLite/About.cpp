// About.cpp
#include "About.h"
#include "resource.h"
#include <strsafe.h>

HWND About::s_hwnd = NULL;
const wchar_t About::CLASS_NAME[] = L"WaistGuardLiteAbout";

bool About::Create(HWND parentHwnd)
{
    if (s_hwnd != NULL)
        return false;

    // ע�ᴰ����
    RegisterWindowClass(GetModuleHandle(NULL));

    // ��������
    s_hwnd = CreateWindowEx(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,  // ��չ��ʽ
        CLASS_NAME,           // ��������
        L"����",             // ���ڱ���
        WS_POPUP | WS_CAPTION | WS_SYSMENU,  // ������ʽ
        0, 0,                // λ��
        400, 300,           // ��С
        parentHwnd,          // ������
        NULL,               // �˵�
        GetModuleHandle(NULL),  // ʵ�����
        NULL                // ��������
    );

    if (s_hwnd)
    {
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

        // �����ؼ�
        CreateControls(s_hwnd);

        // ��ʾ����
        ShowWindow(s_hwnd, SW_SHOW);
        UpdateWindow(s_hwnd);
        return true;
    }

    return false;
}

void About::RegisterWindowClass(HINSTANCE hInstance)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);
}

void About::CreateControls(HWND hwnd)
{
    // ����ͼ��
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WAISTGUARDLITE));
    if (hIcon)
    {
        HWND hIconCtrl = CreateWindow(L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_ICON,
            20, 20, 32, 32,
            hwnd, NULL, GetModuleHandle(NULL), NULL);
        SendMessage(hIconCtrl, STM_SETICON, (WPARAM)hIcon, 0);
    }

    // �����ı�
    CreateWindow(L"STATIC", L"�������� v1.0",
        WS_CHILD | WS_VISIBLE,
        70, 20, 200, 20,
        hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow(L"STATIC", L"Ϊ���Ľ������ݻ�����",
        WS_CHILD | WS_VISIBLE,
        70, 50, 200, 20,
        hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow(L"STATIC", L"Powered by ����Ա��ƽ",
        WS_CHILD | WS_VISIBLE,
        70, 80, 200, 20,
        hwnd, NULL, GetModuleHandle(NULL), NULL);

    CreateWindow(L"STATIC", L"��ϵ��ʽ��ɨ���ע΢�Ź��ںţ�",
        WS_CHILD | WS_VISIBLE,
        20, 120, 360, 20,
        hwnd, NULL, GetModuleHandle(NULL), NULL);

    // ������ά��ͼƬ
    HWND hQRCode = CreateWindow(L"STATIC", NULL,
        WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE,
        140, 150, 120, 120,  // ������ʾ // λ�úʹ�С
        hwnd, NULL, GetModuleHandle(NULL), NULL);

    // ���ض�ά��ͼƬ
    HBITMAP hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDB_QRCODE),  // ��Ҫ����Դ�ļ��ж���
        IMAGE_BITMAP,
        120, 120,  // �����Ĵ�С
        LR_DEFAULTCOLOR);

    if (hBitmap)
    {
        SendMessage(hQRCode, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
    }
}

LRESULT CALLBACK About::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_DESTROY:
        s_hwnd = NULL;
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}