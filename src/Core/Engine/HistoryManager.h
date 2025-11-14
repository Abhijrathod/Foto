#pragma once
#include "../Memory/BufferManager.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

// Undo/Redo system
class HistoryManager {
public:
    struct HistoryState {
        std::string description;
        BufferManager::Buffer snapshot;
        size_t layerIndex;
        
        HistoryState(const std::string& desc, const BufferManager::Buffer& buf, size_t layerIdx)
            : description(desc), snapshot(BufferManager::Clone(buf)), layerIndex(layerIdx) {}
        
        ~HistoryState() {
            BufferManager::Destroy(snapshot);
        }
    };

    HistoryManager(size_t maxStates = 50);
    ~HistoryManager();

    void PushState(const std::string& description, const BufferManager::Buffer& buffer, size_t layerIndex);
    bool CanUndo() const { return currentIndex_ > 0; }
    bool CanRedo() const { return currentIndex_ < states_.size() - 1; }

    HistoryState* Undo();
    HistoryState* Redo();

    void Clear();
    size_t GetStateCount() const { return states_.size(); }
    size_t GetCurrentIndex() const { return currentIndex_; }

private:
    std::vector<std::unique_ptr<HistoryState>> states_;
    size_t currentIndex_ = 0;
    size_t maxStates_;
};

