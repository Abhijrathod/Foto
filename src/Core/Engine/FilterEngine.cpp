#include "FilterEngine.h"
#include "../../Filters/FilterBase.h"

FilterEngine::FilterEngine() {
}

FilterEngine::~FilterEngine() {
}

bool FilterEngine::ApplyFilter(FilterBase* filter, BufferManager::Buffer& buffer) {
    if (!filter || !filter->CanApply(buffer)) {
        return false;
    }
    return filter->Apply(buffer);
}

bool FilterEngine::ApplyFilters(const std::vector<FilterBase*>& filters, BufferManager::Buffer& buffer) {
    for (FilterBase* filter : filters) {
        if (!ApplyFilter(filter, buffer)) {
            return false;
        }
    }
    return true;
}

