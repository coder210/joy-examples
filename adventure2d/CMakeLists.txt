# 可执行文件名（单二进制：单机/Host/客户端/独立服务端，运行时 --mode 切换）
set(EXECUTABLE_NAME "adventure2d")
message(STATUS "EXECUTABLE_NAME = '${EXECUTABLE_NAME}'")


# 如果是 Android，必须命名为 main
if (ANDROID)
    set(MOBILE_ASSETS_DIR "${MOBILE_MAIN_DIR}/assets")
    message(STATUS "MOBILE_ASSETS_DIR = '${MOBILE_ASSETS_DIR}'")
    set(EXECUTABLE_NAME main)
    add_library(${EXECUTABLE_NAME} SHARED)
else()
    add_executable(${EXECUTABLE_NAME})
endif()

file(GLOB SRC_FILES  "*.h"  "*.c" "*.hpp" "*.cpp" "scenes/*.h" "scenes/*.cpp" "layers/*.h" "layers/*.cpp" "systems/*.h" "systems/*.cpp" "components/*.h" "components/*.cpp" "network/*.h" "network/*.cpp" "factory/*.h" "factory/*.cpp")

# 合并所有源文件
set(PROJECT_FILES
    ${SRC_FILES}
)

# 添加源文件到目标
target_sources(${EXECUTABLE_NAME}
    PRIVATE
        ${PROJECT_FILES}
)

# Apple 平台额外图标
if(APPLE)
    add_definitions(-DLUA_USE_IOS)
endif()

# 设置语言标准
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# MSVC 下源文件按 UTF-8 解析（项目文件为 UTF-8 无 BOM）
if(MSVC)
    target_compile_options(${EXECUTABLE_NAME} PRIVATE /utf-8)
endif()

# Web 平台输出 HTML
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

# 引入 joy 框架源码（纯文件引入，不再构建独立库 target）
include(${CMAKE_SOURCE_DIR}/joy/joy_add.cmake)
joy_add_sources(${EXECUTABLE_NAME})

# 添加项目自身目录为头文件搜索路径
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# ------------------- 资产处理 -------------------
set(ASSET_ROOTS
    "${CMAKE_CURRENT_SOURCE_DIR}/../${EXECUTABLE_NAME}/assets|${EXECUTABLE_NAME}"
)

set(ASSET_FILES "")
foreach(ASSET_ROOT_ENTRY ${ASSET_ROOTS})
    string(REPLACE "|" ";" root_parts "${ASSET_ROOT_ENTRY}")
    list(GET root_parts 0 ASSETS_ROOT)
    list(GET root_parts 1 NAMESPACE)
    file(GLOB_RECURSE ALL_ASSET_FILES "${ASSETS_ROOT}/${NAMESPACE}/*")
    foreach(file ${ALL_ASSET_FILES})
        if(NOT file MATCHES "\\.svn/" AND NOT file MATCHES "\\.git/")
            file(RELATIVE_PATH rel_path "${ASSETS_ROOT}/${NAMESPACE}" "${file}")
            list(APPEND ASSET_FILES "${file}|${EXECUTABLE_NAME}/${rel_path}")
        endif()
    endforeach()
endforeach()

if (APPLE)
    # macOS/iOS: 放入 Resources 目录
    macro(add_resource FILE)
        file(RELATIVE_PATH relpath "${ASSETS_ROOT}" "${FILE}")
        get_filename_component(dir "${relpath}" DIRECTORY)
        target_sources(${EXECUTABLE_NAME} PRIVATE "${FILE}")
        set_property(SOURCE "${FILE}" PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${dir}")
    endmacro()
    foreach(entry ${ASSET_FILES})
        string(REPLACE "|" ";" parts "${entry}")
        list(GET parts 0 src)
        list(GET parts 1 relpath)
        target_sources(${EXECUTABLE_NAME} PRIVATE "${src}")
        get_filename_component(dir "${relpath}" DIRECTORY)
        set_property(SOURCE "${src}" PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${dir}")
    endforeach()

elseif(EMSCRIPTEN)
    # Emscripten: 使用 --preload-file
    string(REPLACE ";" " " PRELOAD_STR "${ASSET_FILES}")
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${PRELOAD_STR})

else()
    # 其他平台（Windows, Linux, Android 等）
    if(UNIX AND NOT APPLE)
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE m)
    endif()

    macro(copy_helper filename outname)
        add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${filename}" "${outname}"
        )
    endmacro()

    foreach(entry ${ASSET_FILES})
        string(REPLACE "|" ";" parts "${entry}")
        list(GET parts 0 src)
        list(GET parts 1 relpath)
        if (ANDROID)
            set(outname "${MOBILE_ASSETS_DIR}/${relpath}")
        else()
            set(outname "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${relpath}")
        endif()
        message(STATUS "Copying asset: ${relpath}")
        copy_helper("${src}" "${outname}")
    endforeach()
endif()

# ------------------- 平台特定属性 -------------------
if(MSVC)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
    )
endif()
# Visual Studio 启动项目设置（作用于当前目录）
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

# 源文件分组（便于 IDE 显示）
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PROJECT_FILES})

# 调试信息（可选）
message(STATUS "Project current dir: ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "Project source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Project binary dir: ${CMAKE_CURRENT_BINARY_DIR}")