# joy jspine（Spine 骨骼动画）+ ECS 用法测试（独立可执行程序）

add_executable(spine_sdl3_test main.cpp)

# 引入 joy 引擎源码，并启用 Spine 模块（joy/jspine.c + spine-c）
include(${CMAKE_SOURCE_DIR}/joy/joy_add.cmake)
joy_add_sources(spine_sdl3_test SPINE)

# ECS 演示用 entt（单头文件，复用仓库内 project_proxy2d 的副本）
target_include_directories(spine_sdl3_test PRIVATE "${CMAKE_SOURCE_DIR}/project_proxy2d")

set_target_properties(spine_sdl3_test PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

if(MSVC)
    target_compile_options(spine_sdl3_test PRIVATE /utf-8)
endif()

# 资产：官方 spineboy（atlas/png/json）复制到可执行文件目录
set(TEST_ASSETS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(EXISTS "${TEST_ASSETS_ROOT}")
    file(GLOB_RECURSE TEST_ASSET_FILES "${TEST_ASSETS_ROOT}/*")
    foreach(asset_file ${TEST_ASSET_FILES})
        file(RELATIVE_PATH rel_path "${TEST_ASSETS_ROOT}" "${asset_file}")
        add_custom_command(TARGET spine_sdl3_test POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${asset_file}" "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${rel_path}"
        )
    endforeach()
endif()

set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT spine_sdl3_test)
