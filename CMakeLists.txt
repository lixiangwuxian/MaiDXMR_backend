cmake_minimum_required(VERSION 3.0.0)
project(DesktopCaptureApp VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(DesktopCaptureApp src/main.cpp src/nvenc/AppEncD3D11/NvEncoder/NvEncoder.cpp  src/nvenc/AppEncD3D11/NvEncoder/NvEncoderD3D11.cpp)
# # 设置CMake的最小版本要求
# cmake_minimum_required(VERSION 3.10)

# # 设置你的项目名称
# project(DesktopCaptureApp)

# # 指定C++标准
# set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_CXX_STANDARD_REQUIRED True)

# # 设置使用Clang作为编译器
# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)

# # 添加用于编译的源文件
# add_executable(DesktopCaptureApp src/main.cpp src/nvenc/AppEncD3D11/NvEncoder/NvEncoder.cpp  src/nvenc/AppEncD3D11/NvEncoder/NvEncoderD3D11.cpp) # 假设你的源文件名为main.cpp
# include_directories(C:/MinGW/include)

# # 添加编译器标志
# # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xlinker /libpath:C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.19041.0\\um\\x86")

# # 链接Windows系统库，如果有需要
target_link_libraries(DesktopCaptureApp gdi32 dxgi d3d11 d3dcompiler)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
