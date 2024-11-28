// src/MainWindow.cpp
#include "MainWindow.h"
#include <strsafe.h>

bool MainWindow::Create(HINSTANCE hInstance)
{
    // ��ʼ��Ӧ��״̬
    g_appState.workDuration = DEFAULT_WORK_MINUTES;
    g_appState.breakDuration = DEFAULT_BREAK_MINUTES;
    GetSystemTime(&g_appState.startTime);

    // ��������ͼ��
    CreateTrayIcon();

    // ��ʼ����ʱ��
    InitTimers();

    return true;
}

void MainWindow::InitTimers()
{
    // ����������ʱ��
    g_appState.workTimer = SetTimer(
        g_appState.hwnd,
        1,
        g_appState.workDuration * 60 * 1000,  // ת��Ϊ����
        WorkTimerProc
    );

    // ������ʾ��ʱ��
    g_appState.displayTimer = SetTimer(
        g_appState.hwnd,
        2,
        1000,  // 1�����һ��
        DisplayTimerProc
    );
}

void MainWindow::CreateTrayIcon()
{
    ZeroMemory(&g_appState.nid, sizeof(NOTIFYICONDATA));
    g_appState.nid.cbSize = sizeof(NOTIFYICONDATA);
    g_appState.nid.hWnd = g_appState.hwnd;
    g_appState.nid.uID = 1;
    g_appState.nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_appState.nid.uCallbackMessage = WM_TRAYICON;
    g_appState.nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WAISTGUARDLITE));
    StringCchCopy(g_appState.nid.szTip, ARRAYSIZE(g_appState.nid.szTip), WINDOW_TITLE);
    Shell_NotifyIcon(NIM_ADD, &g_appState.nid);
}

void MainWindow::UpdateWorkTime()
{
    SYSTEMTIME currentTime;
    GetSystemTime(&currentTime);

    // ���㹤��ʱ��
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

    // ������ʾ
    wchar_t text[64];
    StringCchPrintf(text, ARRAYSIZE(text), L"�ѹ���ʱ����%d����%d��", minutes, seconds);
    SetWindowText(g_appState.hwnd, text);
}

VOID CALLBACK MainWindow::WorkTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    if (!g_appState.isResting)
    {
        ShowRestWindow();
    }
}

VOID CALLBACK MainWindow::DisplayTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    UpdateWorkTime();
}
void MainWindow::ShowRestWindow()
{
    if (!g_appState.isResting && !g_appState.isPreResting)
    {
        // ����ʾԤ��Ϣ����
        g_appState.isPreResting = true;
        if (PreRestWindow::Create(false))  // false ��ʾ�Զ�����
        {
            // ����û�û��ѡ���ӳ�
            if (!PreRestWindow::IsDelayed())
            {
                // ��ʾȫ����Ϣ����
                g_appState.isResting = true;
                if (RestWindow::Create(g_appState.breakDuration))
                {
                    // ���ü�ʱ
                    GetSystemTime(&g_appState.startTime);
                }
            }
            else
            {
                // �û�ѡ���ӳ٣�3���Ӻ���������
                g_appState.workTimer = SetTimer(
                    g_appState.hwnd,
                    1,  // ʹ��ԭ���Ĺ�����ʱ��
                    3 * 60 * 1000,  // 3����
                    WorkTimerProc
                );
            }
        }
        g_appState.isPreResting = false;
    }
}

void MainWindow::ShowTrayMenu(HWND hwnd, POINT pt)
{
    HMENU hMenu = CreatePopupMenu();
    InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, IDM_TRAY_SHOW, L"��ʾ������");
    InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING, IDM_TRAY_REST, L"������Ϣ");
    InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING, IDM_TRAY_RESTART, L"���¼�ʱ");
    InsertMenu(hMenu, 3, MF_BYPOSITION | MF_STRING, IDM_TRAY_SETTINGS, L"����");
    InsertMenu(hMenu, 4, MF_BYPOSITION | MF_STRING, IDM_TRAY_ABOUT, L"����");
    InsertMenu(hMenu, 5, MF_BYPOSITION | MF_STRING, IDM_TRAY_EXIT, L"�˳�");

    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}