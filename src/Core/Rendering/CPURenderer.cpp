#include "CPURenderer.h"

CPURenderer::CPURenderer() {
}

CPURenderer::~CPURenderer() {
    Shutdown();
}

bool CPURenderer::Initialize() {
    return true;
}

void CPURenderer::Shutdown() {
}

void CPURenderer::Render(const BufferManager::Buffer& buffer) {
    // CPU rendering implementation
}

void CPURenderer::Resize(uint32_t width, uint32_t height) {
}

