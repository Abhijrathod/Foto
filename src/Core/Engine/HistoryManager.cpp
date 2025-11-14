#include "HistoryManager.h"

HistoryManager::HistoryManager(size_t maxStates) : maxStates_(maxStates) {
}

HistoryManager::~HistoryManager() {
    Clear();
}

void HistoryManager::PushState(const std::string& description, const BufferManager::Buffer& buffer, size_t layerIndex) {
    // Remove any states after current index (when undoing then making new change)
    if (currentIndex_ < states_.size()) {
        states_.erase(states_.begin() + currentIndex_ + 1, states_.end());
    }
    
    // Add new state
    states_.push_back(std::make_unique<HistoryState>(description, buffer, layerIndex));
    currentIndex_ = states_.size() - 1;
    
    // Limit history size
    if (states_.size() > maxStates_) {
        states_.erase(states_.begin());
        currentIndex_--;
    }
}

HistoryManager::HistoryState* HistoryManager::Undo() {
    if (!CanUndo()) return nullptr;
    currentIndex_--;
    return states_[currentIndex_].get();
}

HistoryManager::HistoryState* HistoryManager::Redo() {
    if (!CanRedo()) return nullptr;
    currentIndex_++;
    return states_[currentIndex_].get();
}

void HistoryManager::Clear() {
    states_.clear();
    currentIndex_ = 0;
}

