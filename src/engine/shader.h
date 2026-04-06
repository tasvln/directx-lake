#pragma once

#include "utils/pch.h"

class Shader{
    public:
        Shader(const std::wstring& filename);

        ~Shader() = default;

        ComPtr<ID3DBlob> getBytecode() const { 
            return bytecode; 
        }

    private:
        ComPtr<ID3DBlob> bytecode;
};