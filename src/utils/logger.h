#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <windows.h>
#include <cstdarg>
#include <filesystem>
#include <vector>
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

enum class LogType {
    Info,
    Warning,
    Error,
    Debug
};

class Logger {
public:
    static Logger& instance();
    void log(LogType level, const char* file, const char* function, int line, const wchar_t* format, ...);
    void setLogFilePath(const std::wstring& path);

    // New: Dump all D3D12 debug messages from the device
    void dumpD3D12DebugMessages(ComPtr<ID3D12Device2> device);

private:
    Logger();
    ~Logger();

    void writeLog(const std::wstring& message);
    std::wstring formatLogMessage(LogType level, const std::wstring& msg, const char* file, const char* function, int line);

    std::wofstream file;
    std::mutex mutex;
};
