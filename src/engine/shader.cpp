#include "shader.h"

Shader::Shader(const std::wstring& filename)
{
    HRESULT hr = D3DReadFileToBlob(filename.c_str(), &bytecode);
    if (FAILED(hr)) {
        LOG_ERROR(L"Failed to load shader: %s", filename.c_str());
        throw std::runtime_error("Failed to load shader");
    }
    LOG_INFO(L"Shader loaded: %s", filename.c_str());
}


