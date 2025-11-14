#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

void Logger::Initialize(const std::string& logFile) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) return;
    
    logFile_.open(logFile, std::ios::app);
    initialized_ = true;
    Info("Logger initialized");
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        logFile_.close();
    }
    initialized_ = false;
}

void Logger::Log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    const char* levelStr = "";
    switch (level) {
        case LogLevel::Debug: levelStr = "DEBUG"; break;
        case LogLevel::Info: levelStr = "INFO "; break;
        case LogLevel::Warning: levelStr = "WARN "; break;
        case LogLevel::Error: levelStr = "ERROR"; break;
    }
    
    std::string logLine = ss.str() + " [" + levelStr + "] " + message;
    
    std::cout << logLine << std::endl;
    if (logFile_.is_open()) {
        logFile_ << logLine << std::endl;
        logFile_.flush();
    }
}

