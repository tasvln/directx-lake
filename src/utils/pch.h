#pragma once

#pragma once

#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl/client.h>
#include <wrl/event.h>

#include <shellapi.h>

#include <d3d12.h>

#include <dxgi1_6.h>

#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <directx/d3dx12.h>

#include <DirectXTex.h>

#include <iostream>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <memory>
#include <filesystem>
#include <vector>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <exception>
#include <stdexcept>
#include <system_error>
#include <functional>

#include <string>
#include <fstream>

#include "logger.h"

#ifdef _DEBUG
    #include <dxgidebug.h>
#endif

#include "structs.h"

#define LOG_INFO(fmt, ...)    Logger::instance().log(LogType::Info,    __FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_WARNING(fmt, ...) Logger::instance().log(LogType::Warning, __FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...)   Logger::instance().log(LogType::Error,   __FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_DEBUG(fmt, ...)   Logger::instance().log(LogType::Debug,   __FILE__, __FUNCTION__, __LINE__, fmt, __VA_ARGS__)
#define LOG_D3D12_MESSAGES(device) Logger::instance().dumpD3D12DebugMessages(device)

static const UINT FRAMEBUFFERCOUNT = 3;

using namespace Microsoft::WRL;
using namespace DirectX;

inline void throwFailed(HRESULT hr) {
    if (FAILED(hr)) {
        LOG_ERROR(L"throwFailed: HRESULT = 0x%08X", hr);
        throw std::runtime_error("HRESULT failed");
    }
}

constexpr wchar_t DEFAULT_WHITE_TEXTURE[] = L"assets/textures/white.bmp";

