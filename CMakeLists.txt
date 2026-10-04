cmake_minimum_required(VERSION 3.13)
project(DesktopCaptureApp VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

file(GLOB_RECURSE CPP_SRC "src/*.cpp")

add_executable(DesktopCaptureApp ${CPP_SRC})

# MSVC also picks these up from #pragma comment(lib, ...), GCC/MinGW does not.
target_link_libraries(DesktopCaptureApp gdi32 dxgi d3d11 d3dcompiler gdiplus
                      ws2_32 winmm rpcrt4)
if(MINGW)
  # Don't depend on libstdc++/libgcc/libwinpthread DLLs at runtime.
  target_link_options(DesktopCaptureApp PRIVATE -static)
endif()
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CPack)
