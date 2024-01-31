cmake_minimum_required(VERSION 3.0.0)
project(DesktopCaptureApp VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

file(GLOB_RECURSE CPP_SRC "src/*.cpp")

add_executable(DesktopCaptureApp ${CPP_SRC})

target_link_libraries(DesktopCaptureApp gdi32 dxgi d3d11 d3dcompiler)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
