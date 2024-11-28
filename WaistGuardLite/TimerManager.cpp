#include "TimerManager.h"
#include "main.h"
#include "MainWindow.h"

void TimerManager::StartTimer()
{
    // 创建工作定时器
    g_appState.workTimer = SetTimer(
        g_appState.hwnd,
        1,
        g_appState.workDuration * 60 * 1000,
        MainWindow::WorkTimerProc
    );

    // 创建显示定时器
    g_appState.displayTimer = SetTimer(
        g_appState.hwnd,
        2,
        1000,  // 1秒更新一次
        MainWindow::DisplayTimerProc
    );
}

void TimerManager::StopTimer()
{
    if (g_appState.workTimer)
        KillTimer(g_appState.hwnd, g_appState.workTimer);
    if (g_appState.displayTimer)
        KillTimer(g_appState.hwnd, g_appState.displayTimer);
}

void TimerManager::RestartTimer()
{
    // 先清理现有计时器
    StopTimer();

    // 重置状态
    GetSystemTime(&g_appState.startTime);
    g_appState.isResting = false;
    g_appState.isPreResting = false;

    // 重新创建计时器
    StartTimer();

    // 更新显示
    InvalidateRect(g_appState.hwnd, NULL, TRUE);
    UpdateWindow(g_appState.hwnd);
} 