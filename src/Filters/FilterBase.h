#pragma once
#include "../Core/Memory/BufferManager.h"
#include <string>

// Base class for all image filters
class FilterBase {
public:
    FilterBase(const std::string& name);
    virtual ~FilterBase() = default;

    const std::string& GetName() const { return name_; }

    // Apply filter to buffer (modifies buffer in-place)
    virtual bool Apply(BufferManager::Buffer& buffer) = 0;
    
    // Apply filter creating new buffer (doesn't modify source)
    virtual BufferManager::Buffer ApplyCopy(const BufferManager::Buffer& source);

    // Check if filter can be applied
    virtual bool CanApply(const BufferManager::Buffer& buffer) const;

protected:
    std::string name_;
};

