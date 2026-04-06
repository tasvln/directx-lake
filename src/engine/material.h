#pragma once

#include "utils/pch.h"

class Texture;

class Material
{
    public:
        Material() = default;
        Material(std::shared_ptr<Texture> tex) : texture(tex) {}
        
        ~Material() = default;

        void bind(ID3D12GraphicsCommandList* cmdList, UINT rootIndex);

    private:
        std::shared_ptr<Texture> texture;
};
