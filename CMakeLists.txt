cmake_minimum_required(VERSION 3.20)
project(PhotoEditor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PhotoEditor WIN32
    src/main.cpp

    # Windows UI
    src/UI/Windows/MainWindow.cpp
    src/UI/Windows/ToolbarLeft.cpp
    src/UI/Windows/RightPanels.cpp
    src/UI/Windows/LayerPanel.cpp

    # Canvas
    src/UI/Canvas/CanvasView.cpp
    src/UI/Canvas/DXCanvas.cpp

    # Tools
    src/Tools/ToolBase.cpp
    src/Tools/BrushTool.cpp

    # Filters
    src/Filters/FilterBase.cpp

    # Core Engine
    src/Core/Engine/ImageEngine.cpp
    src/Core/Engine/LayerManager.cpp
    src/Core/Engine/HistoryManager.cpp
    src/Core/Engine/ColorEngine.cpp
    src/Core/Engine/FilterEngine.cpp

    # Core Math
    src/Core/Math/Vector2D.cpp
    src/Core/Math/Matrix.cpp
    src/Core/Math/ColorSpace.cpp

    # Core Memory
    src/Core/Memory/MemoryPool.cpp
    src/Core/Memory/BufferManager.cpp
    src/Core/Memory/TileCache.cpp

    # Core Rendering
    src/Core/Rendering/Renderer.cpp
    src/Core/Rendering/GPURenderer.cpp
    src/Core/Rendering/CPURenderer.cpp
    src/Core/Rendering/ViewportManager.cpp

    # File IO
    src/Core/FileIO/ImageCodecs.cpp
    src/Core/FileIO/FileManager.cpp
    src/Core/FileIO/ExportManager.cpp
    src/Core/FileIO/PSDFormat.cpp

    # Utils
    src/Utils/Logger.cpp
    src/Utils/Config.cpp
    src/Utils/Threading.cpp
    src/Utils/Profiler.cpp
)

# Now that PhotoEditor target exists, we can add include dirs
target_include_directories(PhotoEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/UI
    ${CMAKE_SOURCE_DIR}/src/UI/Canvas
    ${CMAKE_SOURCE_DIR}/src/UI/Windows
    ${CMAKE_SOURCE_DIR}/src/Tools
)

# Link DirectX + Windows libs
target_link_libraries(PhotoEditor PRIVATE
    comctl32
    d3d11
    dxgi
    d3dcompiler
    windowscodecs
)
