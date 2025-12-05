#include "PSDFormat.h"
#include <fstream>
#include <cstring>

// PSD file header constants
#define PSD_SIGNATURE "8BPS"
#define PSD_VERSION 1

// Helper functions to write/read big-endian data
static void WriteU16BE(std::ofstream& file, uint16_t value) {
    uint8_t bytes[2] = {
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
    file.write(reinterpret_cast<char*>(bytes), 2);
}

static void WriteU32BE(std::ofstream& file, uint32_t value) {
    uint8_t bytes[4] = {
        static_cast<uint8_t>((value >> 24) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
    file.write(reinterpret_cast<char*>(bytes), 4);
}

static uint16_t ReadU16BE(std::ifstream& file) {
    uint8_t bytes[2];
    file.read(reinterpret_cast<char*>(bytes), 2);
    return (static_cast<uint16_t>(bytes[0]) << 8) | bytes[1];
}

static uint32_t ReadU32BE(std::ifstream& file) {
    uint8_t bytes[4];
    file.read(reinterpret_cast<char*>(bytes), 4);
    return (static_cast<uint32_t>(bytes[0]) << 24) |
           (static_cast<uint32_t>(bytes[1]) << 16) |
           (static_cast<uint32_t>(bytes[2]) << 8) |
           bytes[3];
}

bool PSDFormat::Load(const std::string& filepath, ImageEngine* engine) {
    if (!engine) return false;

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    // Read header
    char signature[4];
    file.read(signature, 4);
    if (std::memcmp(signature, PSD_SIGNATURE, 4) != 0) {
        return false;
    }

    uint16_t version = ReadU16BE(file);
    if (version != PSD_VERSION) {
        return false;
    }

    // Skip reserved bytes
    file.seekg(6, std::ios::cur);

    uint16_t channels = ReadU16BE(file);
    uint32_t height = ReadU32BE(file);
    uint32_t width = ReadU32BE(file);
    uint16_t depth = ReadU16BE(file);
    uint16_t colorMode = ReadU16BE(file);

    // Skip Color Mode Data section
    uint32_t colorModeDataLength = ReadU32BE(file);
    file.seekg(colorModeDataLength, std::ios::cur);

    // Skip Image Resources section
    uint32_t imageResourcesLength = ReadU32BE(file);
    file.seekg(imageResourcesLength, std::ios::cur);

    // Read Layer and Mask Information section
    uint32_t layerMaskLength = ReadU32BE(file);

    // Create new document
    engine->CreateNew(width, height);

    if (layerMaskLength > 0) {
        uint32_t layerInfoLength = ReadU32BE(file);
        int16_t layerCount = static_cast<int16_t>(ReadU16BE(file));

        if (layerCount < 0) {
            layerCount = -layerCount; // Absolute value
        }

        // Read layer records
        std::vector<Layer*> layers;
        for (int i = 0; i < layerCount; i++) {
            // Read layer bounds
            uint32_t top = ReadU32BE(file);
            uint32_t left = ReadU32BE(file);
            uint32_t bottom = ReadU32BE(file);
            uint32_t right = ReadU32BE(file);

            uint16_t numChannels = ReadU16BE(file);

            // Skip channel info
            for (int c = 0; c < numChannels; c++) {
                file.seekg(6, std::ios::cur); // channel ID + length
            }

            // Read blend mode signature
            char blendSig[4];
            file.read(blendSig, 4);

            // Read blend mode key
            char blendKey[4];
            file.read(blendKey, 4);

            uint8_t opacity = file.get();
            uint8_t clipping = file.get();
            uint8_t flags = file.get();
            file.get(); // filler

            // Read extra data size
            uint32_t extraDataSize = ReadU32BE(file);
            long extraDataStart = file.tellg();

            // Skip layer mask data
            uint32_t layerMaskSize = ReadU32BE(file);
            file.seekg(layerMaskSize, std::ios::cur);

            // Skip layer blending ranges
            uint32_t blendingRangesSize = ReadU32BE(file);
            file.seekg(blendingRangesSize, std::ios::cur);

            // Read layer name
            uint8_t nameLength = file.get();
            char layerName[256] = {0};
            file.read(layerName, nameLength);

            // Padding to multiple of 4
            int namePadding = ((nameLength + 1 + 3) & ~3) - (nameLength + 1);
            file.seekg(namePadding, std::ios::cur);

            // Skip to end of extra data
            file.seekg(extraDataStart + extraDataSize, std::ios::beg);

            // Create layer
            Layer* layer = engine->GetLayerManager().CreateLayer(
                right - left,
                bottom - top,
                std::string(layerName)
            );
            layer->SetOpacity(opacity / 255.0f);
            layer->SetVisible((flags & 2) == 0);

            layers.push_back(layer);
        }

        // Read layer pixel data
        for (size_t i = 0; i < layers.size(); i++) {
            Layer* layer = layers[i];
            auto& buffer = layer->GetBuffer();

            // For simplicity, assume raw uncompressed RGBA data
            if (buffer.data && buffer.size > 0) {
                file.read(reinterpret_cast<char*>(buffer.data), buffer.size);
            }
        }
    }

    return true;
}

bool PSDFormat::Save(const std::string& filepath, ImageEngine* engine) {
    if (!engine) return false;

    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    uint32_t width = engine->GetWidth();
    uint32_t height = engine->GetHeight();

    // Write File Header
    file.write(PSD_SIGNATURE, 4);
    WriteU16BE(file, PSD_VERSION);

    // Reserved bytes
    for (int i = 0; i < 6; i++) {
        file.put(0);
    }

    WriteU16BE(file, 4); // channels (RGBA)
    WriteU32BE(file, height);
    WriteU32BE(file, width);
    WriteU16BE(file, 8); // depth (8 bits per channel)
    WriteU16BE(file, 3); // color mode (RGB)

    // Color Mode Data section (empty for RGB)
    WriteU32BE(file, 0);

    // Image Resources section (empty for simplicity)
    WriteU32BE(file, 0);

    // Layer and Mask Information section
    LayerManager& layerMgr = engine->GetLayerManager();
    size_t layerCount = layerMgr.GetLayerCount();

    // Calculate layer info size (approximate)
    std::streampos layerMaskLengthPos = file.tellp();
    WriteU32BE(file, 0); // Placeholder for layer mask length

    std::streampos layerInfoLengthPos = file.tellp();
    WriteU32BE(file, 0); // Placeholder for layer info length

    WriteU16BE(file, static_cast<uint16_t>(layerCount));

    // Write layer records
    for (size_t i = 0; i < layerCount; i++) {
        const Layer* layer = layerMgr.GetLayer(i);
        if (!layer) continue;

        // Layer bounds
        WriteU32BE(file, 0); // top
        WriteU32BE(file, 0); // left
        WriteU32BE(file, layer->GetHeight()); // bottom
        WriteU32BE(file, layer->GetWidth()); // right

        WriteU16BE(file, 4); // number of channels (RGBA)

        // Channel info
        for (int c = 0; c < 4; c++) {
            WriteU16BE(file, c < 3 ? c : -1); // channel ID (0=R, 1=G, 2=B, -1=transparency)
            WriteU32BE(file, layer->GetWidth() * layer->GetHeight()); // data length
        }

        // Blend mode signature
        file.write("8BIM", 4);

        // Blend mode key (normal)
        file.write("norm", 4);

        file.put(static_cast<uint8_t>(layer->GetOpacity() * 255)); // opacity
        file.put(0); // clipping
        file.put(layer->IsVisible() ? 0 : 2); // flags
        file.put(0); // filler

        // Extra data
        std::streampos extraDataSizePos = file.tellp();
        WriteU32BE(file, 0); // placeholder

        // Layer mask data (empty)
        WriteU32BE(file, 0);

        // Layer blending ranges (empty)
        WriteU32BE(file, 0);

        // Layer name
        std::string name = layer->GetName();
        if (name.length() > 255) name = name.substr(0, 255);

        file.put(static_cast<uint8_t>(name.length()));
        file.write(name.c_str(), name.length());

        // Padding to multiple of 4
        int totalNameBytes = 1 + name.length();
        int padding = ((totalNameBytes + 3) & ~3) - totalNameBytes;
        for (int p = 0; p < padding; p++) {
            file.put(0);
        }

        // Update extra data size
        std::streampos currentPos = file.tellp();
        uint32_t extraDataSize = static_cast<uint32_t>(currentPos - extraDataSizePos - 4);
        file.seekp(extraDataSizePos);
        WriteU32BE(file, extraDataSize);
        file.seekp(currentPos);
    }

    // Write layer pixel data
    for (size_t i = 0; i < layerCount; i++) {
        const Layer* layer = layerMgr.GetLayer(i);
        if (!layer) continue;

        const auto& buffer = layer->GetBuffer();
        if (buffer.data && buffer.size > 0) {
            file.write(reinterpret_cast<const char*>(buffer.data), buffer.size);
        }
    }

    // Update layer info length
    std::streampos endPos = file.tellp();
    uint32_t layerInfoLength = static_cast<uint32_t>(endPos - layerInfoLengthPos - 4);
    file.seekp(layerInfoLengthPos);
    WriteU32BE(file, layerInfoLength);

    // Update layer mask length
    uint32_t layerMaskLength = static_cast<uint32_t>(endPos - layerMaskLengthPos - 4);
    file.seekp(layerMaskLengthPos);
    WriteU32BE(file, layerMaskLength);

    // Write composite image data (flattened)
    file.seekp(endPos);

    // Compression method (0 = raw)
    WriteU16BE(file, 0);

    // Write composite RGBA data
    BufferManager::Buffer composite = engine->GetCompositeImage();
    if (composite.data && composite.size > 0) {
        file.write(reinterpret_cast<const char*>(composite.data), composite.size);
        BufferManager::Destroy(composite);
    }

    return true;
}
