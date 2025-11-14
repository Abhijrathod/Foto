#pragma once
#include "../Engine/ImageEngine.h"
#include <string>

class PSDFormat {
public:
    static bool Load(const std::string& filepath, ImageEngine* engine);
    static bool Save(const std::string& filepath, ImageEngine* engine);
};

