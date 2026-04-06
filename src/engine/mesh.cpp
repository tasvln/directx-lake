#include "mesh.h"

#include "material.h"

Mesh::Mesh(
    ComPtr<ID3D12Device2> device, 
    const std::vector<VertexStruct>& vertices,
    const std::vector<uint32_t>& indices,
    std::shared_ptr<Material> mat
) : 
    device(device),
    material(mat)
{
    LOG_INFO(L"MeshBuffer -> Creating vertex and index buffers...");
    vertex = std::make_unique<VertexBuffer>(
        device,
        vertices
    );
    
    index = std::make_unique<IndexBuffer>(
        device,
        indices
    );

    LOG_INFO(L"MeshBuffer -> Buffers created successfully.");
}

// void Mesh::draw(
//     ID3D12GraphicsCommandList* cmdList,
//     UINT rootIndex
// ) {
//     if (material) {
//         material->bind(cmdList, rootIndex);
//     }

//     auto vbView = vertex->getView();
//     auto ibView = index->getView();
//     auto indexCount = index->getCount();

//     cmdList->IASetVertexBuffers(0, 1, &vbView);
//     cmdList->IASetIndexBuffer(&ibView);
//     cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//     cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
// }

void Mesh::draw(
    ID3D12GraphicsCommandList* cmdList,
    UINT rootIndex
) {
    LOG_INFO(
        L"[Mesh] draw() called: vertices=%u, indices=%u",
        static_cast<UINT>(vertex->getCount()),
        static_cast<UINT>(index->getCount())
    );

    if (material) {
        LOG_INFO(L"[Mesh] Binding material texture at root index %u", rootIndex);

        // Dump any pending D3D12 debug messages BEFORE binding
        LOG_D3D12_MESSAGES(device);

        material->bind(cmdList, rootIndex);

        // Dump any D3D12 debug messages AFTER binding
        LOG_D3D12_MESSAGES(device);
    } else {
        LOG_INFO(L"[Mesh] No material assigned, skipping texture binding");
    }

    // Set vertex and index buffers
    auto vbView = vertex->getView();
    auto ibView = index->getView();
    auto indexCount = index->getCount();

    LOG_INFO(
        L"[Mesh] Setting vertex and index buffers: vb=%p, ib=%p, indexCount=%u",
        vbView.BufferLocation,
        ibView.BufferLocation,
        indexCount
    );

    cmdList->IASetVertexBuffers(0, 1, &vbView);
    cmdList->IASetIndexBuffer(&ibView);
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    LOG_INFO(L"[Mesh] Drawing indexed instanced");
    cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);

    LOG_INFO(L"[Mesh] Draw call completed");
}


