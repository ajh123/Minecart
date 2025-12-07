#include "minecart/model.hpp"

#include <cstring>
#include <stdexcept>

namespace minecart::graphics {

    Model::Model(SDL_GPUDevice* device)
        : m_device(device)
        , m_vertexBuffer(nullptr, SDLGPUBufferDeleter{device})
        , m_indexBuffer(nullptr, SDLGPUBufferDeleter{device})
    {
        if (!device) {
            throw ModelException("Device cannot be null");
        }
    }

    void Model::set_vertices(std::span<const Vertex> vertices) {
        m_vertices.assign(vertices.begin(), vertices.end());
        m_vertexCount = static_cast<uint32_t>(vertices.size());
        m_uploaded = false;
    }

    void Model::set_indices(std::span<const uint32_t> indices) {
        m_indices.assign(indices.begin(), indices.end());
        m_indexCount = static_cast<uint32_t>(indices.size());
        m_useIndexBuffer = !indices.empty();
        m_uploaded = false;
    }

    void Model::upload_vertex_data() {
        if (m_vertices.empty()) {
            throw ModelException("No vertices to upload");
        }

        size_t bufferSize = m_vertices.size() * sizeof(Vertex);

        // Create vertex buffer
        SDL_GPUBufferCreateInfo bufferInfo{};
        bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
        bufferInfo.size = static_cast<Uint32>(bufferSize);

        SDL_GPUBuffer* vertexBuffer = SDL_CreateGPUBuffer(m_device, &bufferInfo);
        if (!vertexBuffer) {
            throw ModelException(std::string("Failed to create vertex buffer: ") + SDL_GetError());
        }
        m_vertexBuffer.reset(vertexBuffer);

        // Create transfer buffer for uploading data
        SDL_GPUTransferBufferCreateInfo transferInfo{};
        transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transferInfo.size = static_cast<Uint32>(bufferSize);

        SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(m_device, &transferInfo);
        if (!transferBuffer) {
            throw ModelException(std::string("Failed to create transfer buffer: ") + SDL_GetError());
        }

        // Map and copy data
        void* mappedData = SDL_MapGPUTransferBuffer(m_device, transferBuffer, false);
        if (!mappedData) {
            SDL_ReleaseGPUTransferBuffer(m_device, transferBuffer);
            throw ModelException(std::string("Failed to map transfer buffer: ") + SDL_GetError());
        }
        std::memcpy(mappedData, m_vertices.data(), bufferSize);
        SDL_UnmapGPUTransferBuffer(m_device, transferBuffer);

        // Upload to GPU
        SDL_GPUCommandBuffer* uploadCmdBuffer = SDL_AcquireGPUCommandBuffer(m_device);
        if (!uploadCmdBuffer) {
            SDL_ReleaseGPUTransferBuffer(m_device, transferBuffer);
            throw ModelException(std::string("Failed to acquire command buffer: ") + SDL_GetError());
        }

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuffer);
        if (!copyPass) {
            SDL_SubmitGPUCommandBuffer(uploadCmdBuffer);
            SDL_ReleaseGPUTransferBuffer(m_device, transferBuffer);
            throw ModelException(std::string("Failed to begin copy pass: ") + SDL_GetError());
        }

        SDL_GPUTransferBufferLocation srcLocation{};
        srcLocation.transfer_buffer = transferBuffer;
        srcLocation.offset = 0;

        SDL_GPUBufferRegion dstRegion{};
        dstRegion.buffer = m_vertexBuffer.get();
        dstRegion.offset = 0;
        dstRegion.size = static_cast<Uint32>(bufferSize);

        SDL_UploadToGPUBuffer(copyPass, &srcLocation, &dstRegion, false);
        SDL_EndGPUCopyPass(copyPass);

        SDL_SubmitGPUCommandBuffer(uploadCmdBuffer);
        SDL_ReleaseGPUTransferBuffer(m_device, transferBuffer);
    }

    void Model::upload_index_data() {
        if (m_indices.empty()) {
            return; // No index data to upload
        }

        size_t bufferSize = m_indices.size() * sizeof(uint32_t);

        // Create index buffer
        SDL_GPUBufferCreateInfo bufferInfo{};
        bufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
        bufferInfo.size = static_cast<Uint32>(bufferSize);

        SDL_GPUBuffer* indexBuffer = SDL_CreateGPUBuffer(m_device, &bufferInfo);
        if (!indexBuffer) {
            throw ModelException(std::string("Failed to create index buffer: ") + SDL_GetError());
        }
        m_indexBuffer.reset(indexBuffer);

        // Create transfer buffer
        SDL_GPUTransferBufferCreateInfo transferInfo{};
        transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transferInfo.size = static_cast<Uint32>(bufferSize);

        SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(m_device, &transferInfo);
        if (!transferBuffer) {
            throw ModelException(std::string("Failed to create transfer buffer: ") + SDL_GetError());
        }

        // Map and copy data
        void* mappedData = SDL_MapGPUTransferBuffer(m_device, transferBuffer, false);
        if (!mappedData) {
            SDL_ReleaseGPUTransferBuffer(m_device, transferBuffer);
            throw ModelException(std::string("Failed to map transfer buffer: ") + SDL_GetError());
        }
        std::memcpy(mappedData, m_indices.data(), bufferSize);
        SDL_UnmapGPUTransferBuffer(m_device, transferBuffer);

        // Upload to GPU
        SDL_GPUCommandBuffer* uploadCmdBuffer = SDL_AcquireGPUCommandBuffer(m_device);
        if (!uploadCmdBuffer) {
            SDL_ReleaseGPUTransferBuffer(m_device, transferBuffer);
            throw ModelException(std::string("Failed to acquire command buffer: ") + SDL_GetError());
        }

        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuffer);
        if (!copyPass) {
            SDL_SubmitGPUCommandBuffer(uploadCmdBuffer);
            SDL_ReleaseGPUTransferBuffer(m_device, transferBuffer);
            throw ModelException(std::string("Failed to begin copy pass: ") + SDL_GetError());
        }

        SDL_GPUTransferBufferLocation srcLocation{};
        srcLocation.transfer_buffer = transferBuffer;
        srcLocation.offset = 0;

        SDL_GPUBufferRegion dstRegion{};
        dstRegion.buffer = m_indexBuffer.get();
        dstRegion.offset = 0;
        dstRegion.size = static_cast<Uint32>(bufferSize);

        SDL_UploadToGPUBuffer(copyPass, &srcLocation, &dstRegion, false);
        SDL_EndGPUCopyPass(copyPass);

        SDL_SubmitGPUCommandBuffer(uploadCmdBuffer);
        SDL_ReleaseGPUTransferBuffer(m_device, transferBuffer);
    }

    void Model::upload() {
        if (m_vertices.empty()) {
            throw ModelException("No vertices set - call set_vertices() first");
        }

        upload_vertex_data();
        upload_index_data();
        m_uploaded = true;
    }

    void Model::render(SDL_GPURenderPass* renderPass) const {
        if (!is_ready()) {
            return; // Silently skip if not ready
        }

        // Bind vertex buffer
        SDL_GPUBufferBinding vertexBufferBinding{};
        vertexBufferBinding.buffer = m_vertexBuffer.get();
        vertexBufferBinding.offset = 0;

        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);

        // Draw
        if (m_useIndexBuffer && m_indexBuffer) {
            SDL_GPUBufferBinding indexBufferBinding{};
            indexBufferBinding.buffer = m_indexBuffer.get();
            indexBufferBinding.offset = 0;

            SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
            SDL_DrawGPUIndexedPrimitives(renderPass, m_indexCount, 1, 0, 0, 0);
        } else {
            SDL_DrawGPUPrimitives(renderPass, m_vertexCount, 1, 0, 0);
        }
    }

    bool Model::is_ready() const noexcept {
        return m_uploaded && m_vertexBuffer && m_vertexCount > 0;
    }

} // namespace minecart::graphics
