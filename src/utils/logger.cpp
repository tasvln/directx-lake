#include "logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

Logger::Logger() {
    std::wstring logDir = L"logs";
    CreateDirectoryW(logDir.c_str(), nullptr);
    setLogFilePath(logDir + L"\\engine.log");
}

Logger::~Logger() {
    if (file.is_open())
        file.close();
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::setLogFilePath(const std::wstring& path) {
    std::lock_guard<std::mutex> lock(mutex);
    if (file.is_open()) file.close();
    file.open(path, std::ios_base::app);
}

void Logger::log(LogType level, const char* file, const char* function, int line, const wchar_t* format, ...) {
    wchar_t buffer[4096];
    va_list args;
    va_start(args, format);
    vswprintf_s(buffer, sizeof(buffer)/sizeof(wchar_t), format, args);
    va_end(args);

    std::wstring message = formatLogMessage(level, buffer, file, function, line);
    OutputDebugStringW(message.c_str());
    writeLog(message);
}

void Logger::writeLog(const std::wstring& message) {
    std::lock_guard<std::mutex> lock(mutex);
    if (file.is_open()) {
        file << message;
        file.flush();
    }
}

std::wstring Logger::formatLogMessage(LogType level, const std::wstring& msg, const char* file, const char* function, int line) {
    auto now = std::chrono::system_clock::now();
    std::time_t timeT = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
    localtime_s(&localTime, &timeT);

    std::wstringstream ss;
    ss << L"[" << std::put_time(&localTime, L"%Y-%m-%d %H:%M:%S") << L"] ";

    switch (level) {
        case LogType::Info:    ss << L"[INFO]    "; break;
        case LogType::Warning: ss << L"[WARNING] "; break;
        case LogType::Error:   ss << L"[ERROR]   "; break;
        case LogType::Debug:   ss << L"[DEBUG]   "; break;
        default:               ss << L"[LOG]     "; break;
    }

    ss << L"[" << std::filesystem::path(file).filename().wstring() << L":" << line << L"] ";
    ss << msg << L"\n";
    return ss.str();
}

void Logger::dumpD3D12DebugMessages(ComPtr<ID3D12Device2> device) {
#if defined(_DEBUG)
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(device.As(&infoQueue))) {
        UINT64 num = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
        for (UINT64 i = 0; i < num; ++i) {
            SIZE_T messageLength = 0;
            infoQueue->GetMessage(i, nullptr, &messageLength);

            std::vector<char> bytes(messageLength);
            auto message = reinterpret_cast<D3D12_MESSAGE*>(bytes.data());

            if (SUCCEEDED(infoQueue->GetMessage(i, message, &messageLength))) {
                std::cerr << "[D3D12 Debug] " << message->pDescription << std::endl;
                Logger::instance().log(LogType::Error, __FILE__, __FUNCTION__, __LINE__, L"[D3D12 Debug] %S", message->pDescription);
            }
        }
        infoQueue->ClearStoredMessages();
    }
#endif
}


