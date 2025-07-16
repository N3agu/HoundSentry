#pragma once
#include <windows.h>
#include <evntcons.h>
#include <thread>
#include <atomic>

class Monitor {
public:
    Monitor();
    ~Monitor();

    bool Start();
    void Stop();
    void WaitForExit();

private:
    void ProcessEvent(PEVENT_RECORD pEvent);
    static void WINAPI EventRecordCallback(PEVENT_RECORD pEvent);
    void TraceThread();

    TRACEHANDLE m_hSession;
    TRACEHANDLE m_hTrace;
    std::thread m_traceThread;
    std::atomic<bool> m_isRunning;
};