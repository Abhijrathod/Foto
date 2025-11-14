cmake_minimum_required(VERSION 3.20)
project(PhotoEditor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PhotoEditor WIN32
    src/main.cpp
    src/UI/Windows/MainWindow.cpp
    src/UI/Canvas/CanvasView.cpp
    src/UI/Windows/ToolbarLeft.cpp
    src/UI/Windows/RightPanels.cpp
)

# ⭐ ADD THIS LINE:
target_link_libraries(PhotoEditor PRIVATE comctl32)
