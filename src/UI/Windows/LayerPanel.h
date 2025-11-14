#pragma once
#include <windows.h>
#include <vector>
#include <string>

struct LayerPanelItem {
    std::string name;
    bool visible = true;
    float opacity = 1.0f;
    void* textureData = nullptr; // For thumbnail
    UINT width = 0;
    UINT height = 0;
};

class LayerPanel {
public:
    LayerPanel(HWND parent, HINSTANCE hInstance);
    ~LayerPanel();

    void Create(int x, int y, int width, int height);
    void AddLayer(const std::string& name);
    void DeleteLayer(int index);
    void SetLayerVisible(int index, bool visible);
    void UpdateThumbnail(int index, const uint8_t* pixels, UINT width, UINT height);
    void Render();

    int GetSelectedLayer() const { return selectedLayer_; }
    int GetLayerCount() const { return (int)layers_.size(); }
    LayerPanelItem* GetLayer(int index);

private:
    HWND parent_;
    HWND hwnd_ = nullptr;
    HINSTANCE hInstance_;
    std::vector<LayerPanelItem> layers_;
    int selectedLayer_ = -1;
    int scrollY_ = 0;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnPaint();
    void OnLButtonDown(int x, int y);
};

