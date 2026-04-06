#pragma once

#include "utils/pch.h"
#include "engine/resources/vertex.h"
#include "engine/resources/index.h"

class Material;

class Mesh {
    public:
        Mesh(
            ComPtr<ID3D12Device2> device, 
            const std::vector<VertexStruct>& vertices,
            const std::vector<uint32_t>& indices,
            std::shared_ptr<Material> mat
        );

        ~Mesh() = default;

        VertexBuffer* getVertex() const {
            return vertex.get();
        }   

        IndexBuffer* getIndex() const {
            return index.get();
        } 

        void draw(
            ID3D12GraphicsCommandList* cmdList,
            UINT rootIndex
        );

    private:
        ComPtr<ID3D12Device2> device;
        
        std::unique_ptr<VertexBuffer> vertex;
        std::unique_ptr<IndexBuffer> index;

        std::shared_ptr<Material> material;
};