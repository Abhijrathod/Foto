#include "Profiler.h"
#include <iostream>
#include <iomanip>

Profiler& Profiler::GetInstance() {
    static Profiler instance;
    return instance;
}

void Profiler::Start(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto& data = profiles_[name];
    data.startTime = std::chrono::high_resolution_clock::now();
    data.active = true;
}

void Profiler::End(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = profiles_.find(name);
    if (it != profiles_.end() && it->second.active) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - it->second.startTime).count();
        it->second.totalTime += duration / 1000.0; // Convert to milliseconds
        it->second.callCount++;
        it->second.active = false;
    }
}

void Profiler::Reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    profiles_.clear();
}

double Profiler::GetAverageTime(const std::string& name) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    auto it = profiles_.find(name);
    if (it != profiles_.end() && it->second.callCount > 0) {
        return it->second.totalTime / it->second.callCount;
    }
    return 0.0;
}

uint64_t Profiler::GetCallCount(const std::string& name) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    auto it = profiles_.find(name);
    return (it != profiles_.end()) ? it->second.callCount : 0;
}

void Profiler::PrintReport() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    
    std::cout << "\n=== Performance Report ===\n";
    std::cout << std::left << std::setw(30) << "Function" 
              << std::setw(15) << "Calls" 
              << std::setw(15) << "Avg Time (ms)" 
              << std::setw(15) << "Total Time (ms)" << "\n";
    std::cout << std::string(75, '-') << "\n";
    
    for (const auto& pair : profiles_) {
        if (pair.second.callCount > 0) {
            double avg = pair.second.totalTime / pair.second.callCount;
            std::cout << std::left << std::setw(30) << pair.first
                      << std::setw(15) << pair.second.callCount
                      << std::setw(15) << std::fixed << std::setprecision(3) << avg
                      << std::setw(15) << std::fixed << std::setprecision(3) << pair.second.totalTime << "\n";
        }
    }
    std::cout << "\n";
}

