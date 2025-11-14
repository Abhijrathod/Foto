#pragma once
#include <string>
#include <chrono>
#include <unordered_map>
#include <mutex>

// Simple performance profiler
class Profiler {
public:
    static Profiler& GetInstance();
    
    void Start(const std::string& name);
    void End(const std::string& name);
    void Reset();
    
    double GetAverageTime(const std::string& name) const;
    uint64_t GetCallCount(const std::string& name) const;
    
    void PrintReport() const;

private:
    Profiler() = default;
    ~Profiler() = default;
    
    struct ProfileData {
        double totalTime = 0.0;
        uint64_t callCount = 0;
        std::chrono::high_resolution_clock::time_point startTime;
        bool active = false;
    };
    
    std::unordered_map<std::string, ProfileData> profiles_;
    mutable std::mutex mutex_;
};

#define PROFILE_SCOPE(name) ProfilerScope _prof_scope(name)

class ProfilerScope {
public:
    ProfilerScope(const std::string& name) : name_(name) {
        Profiler::GetInstance().Start(name_);
    }
    ~ProfilerScope() {
        Profiler::GetInstance().End(name_);
    }
private:
    std::string name_;
};

