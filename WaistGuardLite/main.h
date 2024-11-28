// main.h
#pragma once
#include <windows.h>
#include <shellapi.h>
#include "resource.h"

// ��������
const wchar_t CLASS_NAME[] = L"WaistGuardLite";
const wchar_t WINDOW_TITLE[] = L"�������� v1.0";

// Ĭ��ʱ������
#define DEFAULT_WORK_MINUTES  45   // Ĭ�Ϲ���ʱ��
#define DEFAULT_BREAK_MINUTES 5    // Ĭ����Ϣʱ��

// ���̲˵�����
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_SHOW      1001
#define ID_TRAY_REST      1002
#define ID_TRAY_RESTART   1003
#define ID_TRAY_SETTINGS  1004
#define ID_TRAY_ABOUT     1005
#define ID_TRAY_EXIT      1006

// ȫ��״̬
struct AppState {
    HWND hwnd;              // �����ھ��
    UINT_PTR workTimer;     // ������ʱ��
    UINT_PTR displayTimer;  // ��ʾ��ʱ��
    SYSTEMTIME startTime;   // ��ʼʱ��
    bool isResting;         // �Ƿ�����Ϣ
    bool isPreResting;      // �Ƿ���Ԥ��Ϣ
    int workDuration;       // ����ʱ�������ӣ�
    int breakDuration;      // ��Ϣʱ�������ӣ�
    bool autoStart;         // ����������
    NOTIFYICONDATA nid;     // ����ͼ������
};

// ȫ�ֱ���
extern AppState g_appState;

// ���ڹ��̺�������
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);