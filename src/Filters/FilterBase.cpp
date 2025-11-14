#include "FilterBase.h"

FilterBase::FilterBase(const std::string& name) : name_(name) {
}

BufferManager::Buffer FilterBase::ApplyCopy(const BufferManager::Buffer& source) {
    BufferManager::Buffer result = BufferManager::Clone(source);
    Apply(result);
    return result;
}

bool FilterBase::CanApply(const BufferManager::Buffer& buffer) const {
    return buffer.data != nullptr && buffer.width > 0 && buffer.height > 0;
}

