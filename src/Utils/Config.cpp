#include "Config.h"
#include <fstream>
#include <sstream>

Config& Config::GetInstance() {
    static Config instance;
    return instance;
}

void Config::Load(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    
    std::string line;
    while (std::getline(file, line)) {
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            values_[key] = value;
        }
    }
}

void Config::Save(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream file(filepath);
    if (!file.is_open()) return;
    
    for (const auto& pair : values_) {
        file << pair.first << "=" << pair.second << "\n";
    }
}

void Config::SetString(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    values_[key] = value;
}

void Config::SetInt(const std::string& key, int value) {
    SetString(key, std::to_string(value));
}

void Config::SetFloat(const std::string& key, float value) {
    SetString(key, std::to_string(value));
}

void Config::SetBool(const std::string& key, bool value) {
    SetString(key, value ? "1" : "0");
}

std::string Config::GetString(const std::string& key, const std::string& defaultValue) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    auto it = values_.find(key);
    return (it != values_.end()) ? it->second : defaultValue;
}

int Config::GetInt(const std::string& key, int defaultValue) const {
    std::string val = GetString(key);
    if (val.empty()) return defaultValue;
    return std::stoi(val);
}

float Config::GetFloat(const std::string& key, float defaultValue) const {
    std::string val = GetString(key);
    if (val.empty()) return defaultValue;
    return std::stof(val);
}

bool Config::GetBool(const std::string& key, bool defaultValue) const {
    std::string val = GetString(key);
    if (val.empty()) return defaultValue;
    return val == "1" || val == "true" || val == "True";
}

