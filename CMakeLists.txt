cmake_minimum_required(VERSION 3.20)
project(PhotoEditor)

set(CMAKE_CXX_STANDARD 20)

# FIX: include dirs
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/UI
    ${CMAKE_SOURCE_DIR}/src/UI/Canvas
    ${CMAKE_SOURCE_DIR}/src/UI/Windows
)

add_executable(PhotoEditor WIN32
    src/main.cpp
    src/UI/Windows/MainWindow.cpp
    src/UI/Windows/ToolbarLeft.cpp
    src/UI/Windows/RightPanels.cpp
    src/UI/Canvas/CanvasView.cpp
    src/UI/Canvas/DXCanvas.cpp
)

target_link_libraries(PhotoEditor PRIVATE
    comctl32
    d3d11
    dxgi
    
)
