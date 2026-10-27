# model3d_gpu_test —— SDL3 GPU API 加载 / 渲染 3D 模型测试（独立可执行程序）

add_executable(model3d_gpu_test main.cpp)

# 仅依赖 SDL3（无需 joy 引擎；GPU 路径直接使用 SDL_gpu.h 的 SDL_GPUDevice）
target_link_libraries(model3d_gpu_test PRIVATE SDL3::SDL3)

# 复用 SDL 官方 testgpu 预编译着色器头（cube.vert/frag 的 DXIL/SPIR-V/MSL 三格式，
# 运行时按设备支持的格式选择加载，本机无需 shader 编译工具链）
target_include_directories(model3d_gpu_test PRIVATE "${CMAKE_SOURCE_DIR}/SDL/test")

# glTF(.glb/.gltf) 解析：assimp（FetchContent 拉取，只启用 glTF 导入器，静态库）
include(FetchContent)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)  # 本机无系统 zlib，用 assimp 自带源码编译
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(assimp
        URL https://github.com/assimp/assimp/archive/refs/tags/v5.4.3.zip
        DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(assimp)
target_link_libraries(model3d_gpu_test PRIVATE assimp::assimp)

set_target_properties(model3d_gpu_test PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        # VS 调试默认工作目录是源码目录，模型资产复制在 $(OutDir)/models 下，
        # 显式指定工作目录，调试时才能找到 models/robot.glb
        VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
)

if(MSVC)
    target_compile_options(model3d_gpu_test PRIVATE /utf-8)
endif()

# 资产：OBJ 模型复制到可执行文件目录
set(TEST_ASSETS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(EXISTS "${TEST_ASSETS_ROOT}")
    file(GLOB_RECURSE TEST_ASSET_FILES "${TEST_ASSETS_ROOT}/*")
    foreach(asset_file ${TEST_ASSET_FILES})
        file(RELATIVE_PATH rel_path "${TEST_ASSETS_ROOT}" "${asset_file}")
        add_custom_command(TARGET model3d_gpu_test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${asset_file}" "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${rel_path}"
        )
    endforeach()
endif()

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT model3d_gpu_test)
