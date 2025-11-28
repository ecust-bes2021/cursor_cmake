#[[
这是一个用于cmake学习的脚本--（块注释）
]]
cmake_miniMum_required(VERSION 3.16)

project(cursor_cmake_test VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================================
#设置输出目录 -区分 Debug 和 Release
# ========================================
# 设置可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
# 设置库文件输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
# 设置归档文件输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

# 确保多配置生成器（如 Visual Studio）也使用相同的输出目录
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG})
endforeach()

add_executable(app
    src/main.cpp
    src/add.cpp
)
# 添加 include 目录
target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 添加生成静态库方法
add_library(add_lib STATIC src/add.cpp)
target_include_directories(add_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

#添加生成动态库方法
add_library(app_lib_shared SHARED src/add.cpp)
target_include_directories(app_lib_shared PUBLIC ${CMAKE_SOURCE_DIR}/include)



