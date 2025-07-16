#include "Monitor.h"
#include "ProcessUtils.h"
#include <iostream>
#include <vector>
#include <tdh.h>
#include <memory>

#pragma comment(lib, "tdh.lib")

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::wstring;

// GUIDs - https://learn.microsoft.com/en-us/windows/win32/etw/nt-kernel-logger-constants
static const GUID SystemTraceControlGuid = { 0x9e814aad, 0x3204, 0x11d2, { 0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39 } };
static const GUID FileIoProviderGuid = { 0x90cbdc39, 0x4a3e, 0x421c, { 0x9a, 0x61, 0xb4, 0xad, 0xf3, 0x63, 0xfd, 0x24 } };
static const GUID RegistryProviderGuid = { 0xae53722e, 0xc863, 0x11d2, { 0x86, 0x5a, 0x00, 0x60, 0x08, 0x75, 0x1d, 0x7b } };

static string WStringToString(const wstring& wstr) {
    if (wstr.empty()) return string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

Monitor::Monitor() : m_hSession(0), m_hTrace(0), m_isRunning(false) {}

Monitor::~Monitor() {
    Stop();
}

bool Monitor::Start() {
    if (m_isRunning) {
        return true;
    }
    m_isRunning = true;
    m_traceThread = std::thread(&Monitor::TraceThread, this);
    return true;
}

void Monitor::Stop() {
    if (!m_isRunning.exchange(false)) {
        return;
    }

    if (m_hTrace != 0) {
        CloseTrace(m_hTrace);
        m_hTrace = 0;
    }

    if (m_hSession != 0) {
        auto pProperties = std::make_unique<EVENT_TRACE_PROPERTIES>();
        ZeroMemory(pProperties.get(), sizeof(EVENT_TRACE_PROPERTIES));
        pProperties->Wnode.BufferSize = sizeof(EVENT_TRACE_PROPERTIES);
        pProperties->Wnode.Guid = SystemTraceControlGuid;
        ControlTraceW(m_hSession, NULL, pProperties.get(), EVENT_TRACE_CONTROL_STOP);
    }
}

void Monitor::WaitForExit() {
    if (m_traceThread.joinable()) {
        m_traceThread.join();
    }
}

void Monitor::TraceThread() {
    const size_t bufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAMEW);
    auto pPropertiesBuffer = std::make_unique<char[]>(bufferSize);
    PEVENT_TRACE_PROPERTIES pProperties = (PEVENT_TRACE_PROPERTIES)pPropertiesBuffer.get();

    ZeroMemory(pProperties, bufferSize);
    pProperties->Wnode.BufferSize = bufferSize;
    pProperties->Wnode.Guid = SystemTraceControlGuid;
    pProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    pProperties->Wnode.ClientContext = 1;
    pProperties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    pProperties->EnableFlags = EVENT_TRACE_FLAG_REGISTRY | EVENT_TRACE_FLAG_FILE_IO;
    pProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

    wcscpy_s((wchar_t*)((char*)pProperties + pProperties->LoggerNameOffset),
        wcslen(KERNEL_LOGGER_NAMEW) + 1,
        KERNEL_LOGGER_NAMEW);

    HRESULT hr = StartTraceW(&m_hSession, KERNEL_LOGGER_NAMEW, pProperties);
    if (hr == ERROR_ALREADY_EXISTS) {
        cout << "[DEBUG] Session already exists. Forcing a restart..." << endl;
        hr = ControlTraceW(NULL, KERNEL_LOGGER_NAMEW, pProperties, EVENT_TRACE_CONTROL_STOP);

        if (hr == ERROR_SUCCESS) {
            Sleep(100);
            hr = StartTraceW(&m_hSession, KERNEL_LOGGER_NAMEW, pProperties);
        }
    }

    if (hr != ERROR_SUCCESS) {
        cerr << "[ERROR] Failed to start a clean trace session. Run `logman stop \"NT Kernel Logger\" -ets` as an administrator. Code: " << hr << endl;
        m_isRunning = false;
        return;
    }

    EVENT_TRACE_LOGFILEW logFile = { 0 };
    logFile.LoggerName = (LPWSTR)KERNEL_LOGGER_NAMEW;
    logFile.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    logFile.EventRecordCallback = Monitor::EventRecordCallback;
    logFile.Context = this;

    m_hTrace = OpenTraceW(&logFile);
    if (m_hTrace == INVALID_PROCESSTRACE_HANDLE) {
        cerr << "[ERROR] OpenTraceW failed. Code: " << GetLastError() << endl;
        m_isRunning = false;
        return;
    }

    cout << "[INFO] Now monitoring system events... (Press Ctrl+C to stop)" << endl;
    ProcessTrace(&m_hTrace, 1, 0, 0);

    Stop();
}

void WINAPI Monitor::EventRecordCallback(PEVENT_RECORD pEvent) {
    if (pEvent->UserContext) {
        Monitor* pThis = static_cast<Monitor*>(pEvent->UserContext);
        if (pThis->m_isRunning) {
            pThis->ProcessEvent(pEvent);
        }
    }
}

void Monitor::ProcessEvent(PEVENT_RECORD pEvent) {
    wchar_t guidStr[39];
    StringFromGUID2(pEvent->EventHeader.ProviderId, guidStr, 39);
    cout << "[DEBUG] ETW Event received from provider: " << WStringToString(guidStr) << endl;
}