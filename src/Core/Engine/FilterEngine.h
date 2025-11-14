#pragma once
#include "../Memory/BufferManager.h"
#include <vector>
#include <memory>

class FilterBase;

class FilterEngine {
public:
    FilterEngine();
    ~FilterEngine();

    // Apply filter to buffer
    bool ApplyFilter(FilterBase* filter, BufferManager::Buffer& buffer);
    
    // Batch apply multiple filters
    bool ApplyFilters(const std::vector<FilterBase*>& filters, BufferManager::Buffer& buffer);
};

