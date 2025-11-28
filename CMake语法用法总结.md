# CMake 语法、用法总结

## 三个关键词的定义

| 关键词 | 含义 | 使用场景 |
|--------|------|--------|
| **PRIVATE** | 仅当前目标使用 | 内部实现、不暴露给依赖者 |
| **PUBLIC** | 同时导出给依赖者 | 公共 API、需要暴露给依赖者 |
| **INTERFACE** | 仅导出给依赖者 | 头文件库、只定义接口 |

---

## 最简单的理解方式

### PRIVATE（私密的）
> "我用这个东西，但我不告诉依赖我的人"

### PUBLIC（公开的）
> "我用这个东西，我也告诉依赖我的人可以用"

### INTERFACE（只给别人用）
> "我不用这个东西，但我告诉依赖我的人可以用"

---

## 真实代码例子

### 项目结构
```
utils库 → mylib库 → app程序
```

### 第一步：utils 库

**utils/utils.h**
```cpp
#ifndef UTILS_H
#define UTILS_H

void print_hello();

#endif
```

**utils/utils.cpp**
```cpp
#include "utils.h"
#include <iostream>

void print_hello() {
    std::cout << "Hello from utils!" << std::endl;
}
```

**utils/CMakeLists.txt**
```cmake
add_library(utils utils.cpp)
target_include_directories(utils PRIVATE include/)
```

---

### 第二步：mylib 库

**mylib/mylib.h**
```cpp
#ifndef MYLIB_H
#define MYLIB_H

void mylib_do_something();

#endif
```

**mylib/mylib.cpp**
```cpp
#include "mylib.h"
#include "utils.h"  // mylib 需要用 utils

void mylib_do_something() {
    print_hello();  // 调用 utils 的函数
}
```

**mylib/CMakeLists.txt（PRIVATE 情况）**
```cmake
add_library(mylib mylib.cpp)
target_link_libraries(mylib PRIVATE utils)  # ← PRIVATE
target_include_directories(mylib PRIVATE include/)
```

---

### 第三步：app 程序

**app/main.cpp**
```cpp
#include "mylib.h"

int main() {
    mylib_do_something();  // ✅ 能调用
    return 0;
}
```

**app/CMakeLists.txt**
```cmake
add_executable(app main.cpp)
target_link_libraries(app PRIVATE mylib)
```

---

## 关键问题解答

### 问题 1：如果 app 想直接调用 utils？

**app/main.cpp**
```cpp
#include "mylib.h"
#include "utils.h"  // ← 想直接包含

int main() {
    mylib_do_something();
    print_hello();  // ← 想直接调用
    return 0;
}
```

**结果：❌ 编译错误！**
```
fatal error: utils.h: No such file or directory
```

**为什么？**
- mylib 用 PRIVATE 链接 utils
- CMake 不会把 utils 的包含路径传给 app
- 编译器找不到 `utils.h`

---

### 问题 2：改成 PUBLIC 会怎样？

**mylib/CMakeLists.txt 改成：**
```cmake
add_library(mylib mylib.cpp)
target_link_libraries(mylib PUBLIC utils)  # ← 改成 PUBLIC
target_include_directories(mylib PUBLIC include/)
```

**现在 app/main.cpp 可以这样写：**
```cpp
#include "mylib.h"
#include "utils.h"  // ✅ 现在能找到了！

int main() {
    mylib_do_something();
    print_hello();  // ✅ 现在能编译了！
    return 0;
}
```

**为什么能编译了？**
- mylib 用 PUBLIC 链接 utils
- CMake 会自动把 utils 的包含路径传给 app
- 编译器能找到 `utils.h`

---

## 总结对比表

| 情况 | mylib 能用 utils | app 能 #include utils.h |
|------|--------|--------|
| mylib 用 PRIVATE 链接 utils | ✅ 能 | ❌ 不能 |
| mylib 用 PUBLIC 链接 utils | ✅ 能 | ✅ 能 |

---

## 你的项目中应该怎么用？

### cursor_cmake 项目（可执行文件）
```cmake
add_executable(app
    src/main.cpp
    src/add.cpp
)

target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/include)
# ↑ 用 PRIVATE，因为没人依赖 app
```

### qt_tool_template 项目（可执行文件）
```cmake
add_executable(qt_tool_template ${PROJECT_SOURCES})
target_include_directories(qt_tool_template PRIVATE ${CMAKE_SOURCE_DIR}/include)
# ↑ 用 PRIVATE，因为没人依赖它
```

### 如果以后要做成库
```cmake
add_library(qt_tool_lib ${PROJECT_SOURCES})
target_include_directories(qt_tool_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
# ↑ 改成 PUBLIC，因为别人要用你的库
```

---

## 快速判断法

**问自己一个问题：**

> **别人会直接 `#include` 我的头文件吗？**

- **是** → 用 **PUBLIC** ✅
- **否** → 用 **PRIVATE** ✅

---

# CMake 变量详解

## CMAKE_SOURCE_DIR vs CMAKE_CURRENT_SOURCE_DIR vs PROJECT_SOURCE_DIR

### 三个变量的定义

| 变量 | 含义 | 是否会变 |
|------|------|--------|
| **CMAKE_SOURCE_DIR** | 项目根目录（最顶层） | ❌ 永不改变 |
| **CMAKE_CURRENT_SOURCE_DIR** | 当前 CMakeLists.txt 所在目录 | ✅ 每个子目录都改变 |
| **PROJECT_SOURCE_DIR** | 最近一次 project() 命令的目录 | ✅ 有新 project() 时改变 |

---

## 具体例子

### 简单项目（没有子目录）

```
d:/project/
├── CMakeLists.txt
├── src/
│   └── main.cpp
└── include/
    └── add.h
```

**d:/project/CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.16)
project(myproject)

message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
# 输出：CMAKE_SOURCE_DIR = d:/project

message("CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
# 输出：CMAKE_CURRENT_SOURCE_DIR = d:/project

message("PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
# 输出：PROJECT_SOURCE_DIR = d:/project
```

**输出：**
```
CMAKE_SOURCE_DIR = d:/project
CMAKE_CURRENT_SOURCE_DIR = d:/project
PROJECT_SOURCE_DIR = d:/project
```

---

### 复杂项目（有子目录）

```
d:/project/
├── CMakeLists.txt              ← project(main_project)
├── src/
│   └── main.cpp
└── subproject/
    └── CMakeLists.txt          ← project(sub_project)
```

**d:/project/CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.16)
project(main_project)

message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
# 输出：CMAKE_SOURCE_DIR = d:/project

message("PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
# 输出：PROJECT_SOURCE_DIR = d:/project

add_subdirectory(subproject)
```

**d:/project/subproject/CMakeLists.txt**
```cmake
project(sub_project)

message("CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
# 输出：CMAKE_SOURCE_DIR = d:/project（不变！）

message("PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
# 输出：PROJECT_SOURCE_DIR = d:/project/subproject（变了！）

message("CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
# 输出：CMAKE_CURRENT_SOURCE_DIR = d:/project/subproject
```

---

## 三个变量的对比表

| 变量 | 顶层 CMakeLists.txt | 子目录（无 project） | 子目录（有 project） |
|------|--------|--------|--------|
| **CMAKE_SOURCE_DIR** | `d:/project` | `d:/project` | `d:/project` |
| **CMAKE_CURRENT_SOURCE_DIR** | `d:/project` | `d:/project/subdir` | `d:/project/subdir` |
| **PROJECT_SOURCE_DIR** | `d:/project` | `d:/project` | `d:/project/subdir` |

---

## 什么时候用哪个？

### 用 CMAKE_SOURCE_DIR

**当你需要引用项目根目录的文件时：**

```cmake
# 在任何 CMakeLists.txt 中都能这样写
target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/include)
```

这样无论在哪个子目录，都能正确指向项目根目录的 `include` 文件夹。

### 用 CMAKE_CURRENT_SOURCE_DIR

**当你需要引用当前目录的文件时：**

```cmake
# 在 src/CMakeLists.txt 中
add_executable(app
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/add.cpp
)
```

这样只需要写一次，就能在任何子目录中正确引用该目录的文件。

---

# 输出目录配置

## 三个输出目录变量

```cmake
# 可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

# 库文件输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

# 静态库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
```

### 各变量的用途

| 变量 | 用途 | 输出物 |
|------|------|--------|
| **CMAKE_RUNTIME_OUTPUT_DIRECTORY** | 可执行文件和动态库 | `.exe`、`.dll` |
| **CMAKE_LIBRARY_OUTPUT_DIRECTORY** | 共享库（Linux/Mac） | `.so`、`.dylib` |
| **CMAKE_ARCHIVE_OUTPUT_DIRECTORY** | 静态库 | `.a`、`.lib` |

---

## 多配置生成器兼容性

对于 Visual Studio 等多配置生成器，需要额外设置：

```cmake
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG})
endforeach()
```

这样可以确保 Debug 和 Release 版本分别输出到不同的目录。

---

## 生成器表达式 $<CONFIG>

```cmake
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
```

- `${CMAKE_SOURCE_DIR}` - 在配置阶段展开
- `$<CONFIG>` - 在生成阶段展开，会被替换为 `Debug` 或 `Release`

**最终路径：**
- Debug: `d:/project/bin/Debug/app.exe`
- Release: `d:/project/bin/Release/app.exe`

---

# 库文件的生成与链接

## 一、生成库文件

### 1. 静态库（STATIC）

```cmake
add_library(add_lib STATIC src/add.cpp)
target_include_directories(add_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
```

**输出文件：**

| 编译器 | 输出文件 | 说明 |
|--------|---------|------|
| Visual Studio | `add_lib.lib` | MSVC格式静态库 |
| MinGW/GCC | `libadd_lib.a` | GNU格式静态库 |

**特点：**
- ✅ 编译时链接，无运行时依赖
- ✅ 部署简单，只需要一个exe
- ❌ 体积较大，不能热更新

---

### 2. 动态库（SHARED）

```cmake
add_library(app_lib_shared SHARED src/add.cpp)
target_include_directories(app_lib_shared PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 自动导出所有符号（推荐）
set_target_properties(app_lib_shared PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
```

**输出文件：**

| 编译器 | 动态库本体 | 导入库 | 说明 |
|--------|-----------|--------|------|
| Visual Studio | `app_lib_shared.dll` | `app_lib_shared.lib` | MSVC格式 |
| MinGW/GCC | `app_lib_shared.dll` | `libapp_lib_shared.dll.a` | GNU格式 |

**特点：**
- ✅ 体积小，可热更新
- ✅ 多程序共享，节省内存
- ❌ 运行时需要dll文件
- ❌ 版本管理复杂

---

### 3. 同时生成静态库和动态库

```cmake
# 定义源文件列表
set(LIB_SOURCES src/add.cpp)

# 静态库
add_library(add_lib_static STATIC ${LIB_SOURCES})
target_include_directories(add_lib_static PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 动态库
add_library(add_lib_shared SHARED ${LIB_SOURCES})
target_include_directories(add_lib_shared PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(add_lib_shared PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# exe 链接静态库
add_executable(app src/main.cpp)
target_link_libraries(app PRIVATE add_lib_static)
```

---

### 4. 静态库 vs 动态库 对比

| 特性 | 静态库 (STATIC) | 动态库 (SHARED) |
|------|----------------|-----------------|
| **文件格式** | `.lib` / `.a` | `.dll` + `.lib` / `.so` |
| **链接时机** | 编译时 | 运行时 |
| **exe体积** | 较大 | 较小 |
| **运行依赖** | 无 | 需要dll |
| **更新方式** | 重新编译exe | 替换dll |
| **内存占用** | 每个exe一份 | 多exe共享一份 |

---

## 二、链接库文件

### 方式1：link_directories + target_link_libraries（❌ 不推荐）

```cmake
# 指定库文件搜索目录（全局）
link_directories(${CMAKE_SOURCE_DIR}/../cursor_cmake/bin/Debug)

add_executable(my_app main.cpp)

# 添加头文件目录
target_include_directories(my_app PRIVATE ${CMAKE_SOURCE_DIR}/../cursor_cmake/include)

# 只指定库名，链接器在 link_directories 目录中查找
target_link_libraries(my_app add_lib)
```

**问题：**
- ❌ 全局设置，影响所有后续目标
- ❌ 多个同名库时容易链接错误
- ❌ 代码不清晰，维护困难

---

### 方式2：target_link_libraries 完整路径（✅ 推荐）

```cmake
set(LIB_PATH ${CMAKE_SOURCE_DIR}/../cursor_cmake)

add_executable(my_app main.cpp)

# 添加头文件目录
target_include_directories(my_app PRIVATE ${LIB_PATH}/include)

# 直接指定完整路径
target_link_libraries(my_app ${LIB_PATH}/bin/Debug/add_lib.lib)
```

**优点：**
- ✅ 目标级别，不影响其他目标
- ✅ 明确指定库文件位置
- ✅ 易于维护和理解

---

### 方式3：add_library(IMPORTED) + target_link_libraries（✅✅ 最推荐）

```cmake
set(LIB_PATH ${CMAKE_SOURCE_DIR}/../cursor_cmake)

# 创建导入库目标（不编译，只是给已存在的库起个别名）
add_library(add_lib STATIC IMPORTED)
set_target_properties(add_lib PROPERTIES
    IMPORTED_LOCATION ${LIB_PATH}/bin/Debug/add_lib.lib
    INTERFACE_INCLUDE_DIRECTORIES ${LIB_PATH}/include
)

add_executable(my_app main.cpp)

# 链接导入库目标（自动包含头文件目录）
target_link_libraries(my_app add_lib)
```

**优点：**
- ✅ 可以设置更多属性（头文件、编译选项等）
- ✅ 链接时自动包含头文件目录
- ✅ 符合现代CMake的目标导向风格

---

### 三种链接方式对比

| 方式 | 代码 | 清晰度 | 推荐度 |
|------|------|--------|--------|
| `link_directories` + 库名 | 简洁 | ⭐ 低 | ❌ 不推荐 |
| `target_link_libraries` 完整路径 | 中等 | ⭐⭐⭐ 高 | ✅ 推荐 |
| `add_library(IMPORTED)` | 较多 | ⭐⭐⭐⭐ 很高 | ✅✅ 最推荐 |

---

## 三、add_library 的两种用法

### 用法1：编译创建库

```cmake
# 从源代码编译，生成新的库文件
add_library(add_lib STATIC src/add.cpp)
```

```
源代码 add.cpp  →  编译  →  生成 add_lib.lib（新文件）
```

### 用法2：导入已存在的库（IMPORTED）

```cmake
# 不编译！只是给已存在的库文件起个别名
add_library(add_lib STATIC IMPORTED)
set_target_properties(add_lib PROPERTIES
    IMPORTED_LOCATION /path/to/add_lib.lib
)
```

```
已存在的 add_lib.lib  →  CMake给它起个别名  →  后续用别名链接
```

### 对比

| | 编译创建 | 导入已存在 |
|---|---------|-----------|
| **关键字** | `add_library(name STATIC src.cpp)` | `add_library(name STATIC IMPORTED)` |
| **会编译吗？** | ✅ 会 | ❌ 不会 |
| **生成新文件？** | ✅ 生成新的.lib | ❌ 不生成 |
| **库文件来源** | 源代码编译 | 已经存在的文件 |

---

## 四、link_directories vs target_link_libraries

| 特性 | link_directories | target_link_libraries |
|------|-----------------|----------------------|
| **作用** | 指定库文件**搜索目录** | 指定要链接的**具体库** |
| **范围** | 全局（影响所有后续目标） | 目标级别（只影响该目标） |
| **推荐度** | ❌ 已过时 | ✅ 现代CMake推荐 |

**简单记忆：**
- 🚫 `link_directories` = 旧方法，全局，容易出错
- ✅ `target_link_libraries` = 新方法，目标级别，推荐使用

---

## 五、跨编译器兼容性

### MinGW 生成的库能在 VS 中使用吗？

| 库类型 | MinGW → VS | 说明 |
|--------|-----------|------|
| 静态库 `.a` | ❌ 不能 | 目标文件格式不兼容 |
| 动态库 `.dll` | ✅ 能 | DLL是标准Windows格式 |

**解决方案：**
1. 用相同编译器重新编译源代码
2. 使用DLL（动态库是二进制兼容的）

---

## 六、完整示例

### 库项目（cursor_cmake）

```cmake
cmake_minimum_required(VERSION 3.16)
project(cursor_cmake_test)

set(CMAKE_CXX_STANDARD 17)

# 输出目录配置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

# 生成静态库
add_library(add_lib STATIC src/add.cpp)
target_include_directories(add_lib PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 生成动态库
add_library(add_lib_shared SHARED src/add.cpp)
target_include_directories(add_lib_shared PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(add_lib_shared PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

# 生成可执行文件
add_executable(app src/main.cpp)
target_link_libraries(app PRIVATE add_lib)
target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/include)
```

### 使用库的项目（my_app）

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_app)

set(CMAKE_CXX_STANDARD 17)
set(LIB_PATH ${CMAKE_SOURCE_DIR}/../cursor_cmake)

# 方式1：直接链接（简单场景）
add_executable(my_app main.cpp)
target_include_directories(my_app PRIVATE ${LIB_PATH}/include)
target_link_libraries(my_app ${LIB_PATH}/bin/Debug/add_lib.lib)

# 方式2：使用导入库目标（推荐）
add_library(add_lib STATIC IMPORTED)
set_target_properties(add_lib PROPERTIES
    IMPORTED_LOCATION ${LIB_PATH}/bin/Debug/add_lib.lib
    INTERFACE_INCLUDE_DIRECTORIES ${LIB_PATH}/include
)
add_executable(my_app2 main.cpp)
target_link_libraries(my_app2 add_lib)
```

---

# find 系列命令

CMake 提供了一系列 `find_xxx` 命令，用于查找文件、库、程序等。

## 命令总览

| 命令 | 作用 | 查找目标 |
|------|------|---------|
| **find_package** | 查找整个库/包 | 库 + 头文件 + 配置 |
| **find_library** | 查找库文件 | `.lib` / `.a` / `.so` / `.dll` |
| **find_path** | 查找包含某文件的目录 | 头文件目录 |
| **find_file** | 查找具体文件 | 任意文件 |
| **find_program** | 查找可执行程序 | `.exe` / 可执行文件 |

---

## 一、find_package（最常用）

查找并加载外部库的配置，自动设置头文件路径、库文件等。

### 基本用法

```cmake
# 查找 OpenCV 库
find_package(OpenCV REQUIRED)

# 使用
add_executable(app main.cpp)
target_include_directories(app PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(app ${OpenCV_LIBS})
```

### 工作原理

```
find_package(OpenCV)
        ↓
CMake 在以下位置查找 OpenCVConfig.cmake 或 FindOpenCV.cmake：
  - CMAKE_PREFIX_PATH
  - 系统路径
  - 环境变量
        ↓
找到后设置变量：
  - OpenCV_FOUND = TRUE
  - OpenCV_INCLUDE_DIRS = /path/to/opencv/include
  - OpenCV_LIBS = opencv_core;opencv_imgproc;...
```

### 常用参数

```cmake
# REQUIRED：必须找到，否则报错
find_package(OpenCV REQUIRED)

# COMPONENTS：指定需要的组件
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# 指定版本
find_package(OpenCV 4.5 REQUIRED)

# QUIET：不输出查找信息
find_package(OpenCV QUIET)
```

### 现代CMake的目标用法

```cmake
# 查找 Qt5
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_executable(app main.cpp)
# 使用导入目标，自动包含头文件和库
target_link_libraries(app Qt5::Core Qt5::Widgets)
```

---

## 二、find_library

查找库文件，返回库文件的完整路径。

### 基本用法

```cmake
find_library(CALC_LIB
    NAMES calc libcalc        # 库名（可以指定多个名字）
    PATHS ${PROJECT_SOURCE_DIR}/lib  # 查找路径
)

# 检查是否找到
if(CALC_LIB)
    message("找到库：${CALC_LIB}")
    target_link_libraries(app ${CALC_LIB})
else()
    message(FATAL_ERROR "未找到 calc 库")
endif()
```

### 常用参数

```cmake
find_library(MY_LIB
    NAMES mylib                    # 库名
    PATHS /usr/lib /usr/local/lib  # 查找路径
    PATH_SUFFIXES lib lib64        # 路径后缀
    NO_DEFAULT_PATH                # 不搜索默认路径
)
```

### 输出示例

```
CALC_LIB = /path/to/lib/libcalc.so
```

---

## 三、find_path

查找包含某个文件的目录路径。

### 基本用法

```cmake
# 查找包含 add.h 的目录
find_path(ADD_INCLUDE_DIR
    NAMES add.h               # 要查找的文件名
    PATHS ${PROJECT_SOURCE_DIR}/include  # 查找路径
)

# 使用
if(ADD_INCLUDE_DIR)
    message("找到头文件目录：${ADD_INCLUDE_DIR}")
    target_include_directories(app PRIVATE ${ADD_INCLUDE_DIR})
endif()
```

### 输出示例

```
ADD_INCLUDE_DIR = /path/to/include
# 注意：返回的是目录，不是文件路径
```

---

## 四、find_file

查找具体文件的完整路径。

### 基本用法

```cmake
find_file(CONFIG_FILE
    NAMES config.json
    PATHS ${PROJECT_SOURCE_DIR}/config
)

if(CONFIG_FILE)
    message("找到配置文件：${CONFIG_FILE}")
endif()
```

### 与 find_path 的区别

| 命令 | 返回值 | 示例 |
|------|--------|------|
| `find_path` | 目录路径 | `/path/to/include` |
| `find_file` | 文件完整路径 | `/path/to/include/add.h` |

---

## 五、find_program

查找可执行程序。

### 基本用法

```cmake
# 查找 Python 解释器
find_program(PYTHON_EXE
    NAMES python python3 python.exe
    PATHS "C:/Python39" "/usr/bin"
)

if(PYTHON_EXE)
    message("找到 Python：${PYTHON_EXE}")
    # 可以用来执行脚本
    execute_process(COMMAND ${PYTHON_EXE} --version)
endif()
```

### 常见用途

```cmake
# 查找 Git
find_program(GIT_EXE git)

# 查找 Doxygen（文档生成器）
find_program(DOXYGEN_EXE doxygen)

# 查找编译器
find_program(CLANG_EXE clang++)
```

---

## 六、find 命令对比

| 命令 | 返回值 | 典型用途 |
|------|--------|---------|
| `find_package` | 设置多个变量 | 查找第三方库（OpenCV、Qt、Boost等） |
| `find_library` | 库文件完整路径 | 查找自己的或系统的库文件 |
| `find_path` | 目录路径 | 查找头文件所在目录 |
| `find_file` | 文件完整路径 | 查找配置文件等 |
| `find_program` | 程序完整路径 | 查找编译器、解释器等 |

---

## 七、用 find 命令代替 link_directories

### 旧写法（不推荐）

```cmake
include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_executable(app main.cpp)
target_link_libraries(app calc)
```

### 新写法（推荐）

```cmake
# 用 find 命令查找
find_path(CALC_INCLUDE_DIR NAMES calc.h PATHS ${PROJECT_SOURCE_DIR}/include)
find_library(CALC_LIB NAMES calc PATHS ${PROJECT_SOURCE_DIR}/lib)

add_executable(app main.cpp)
target_include_directories(app PRIVATE ${CALC_INCLUDE_DIR})
target_link_libraries(app ${CALC_LIB})
```

---

## 八、推荐使用优先级

```
1. find_package     ← 首选，最完整
2. find_library + find_path  ← 次选，手动组合
3. 直接指定完整路径  ← 简单场景
4. link_directories  ← 不推荐
```

---

# file 命令

`file` 命令用于文件和目录操作，包括查找文件、读写文件、复制删除等。

## 命令总览

| 命令 | 作用 |
|------|------|
| `file(GLOB ...)` | 查找匹配的文件 |
| `file(GLOB_RECURSE ...)` | 递归查找匹配的文件 |
| `file(READ ...)` | 读取文件内容 |
| `file(WRITE ...)` | 写入文件 |
| `file(APPEND ...)` | 追加写入文件 |
| `file(COPY ...)` | 复制文件 |
| `file(REMOVE ...)` | 删除文件 |
| `file(MAKE_DIRECTORY ...)` | 创建目录 |
| `file(RENAME ...)` | 重命名文件 |
| `file(DOWNLOAD ...)` | 下载文件 |

---

## 一、file(GLOB) - 查找文件

查找匹配模式的文件，将结果存入变量。

### 基本用法

```cmake
# 查找所有 .cpp 文件
file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 查找多种类型
file(GLOB SRC_FILES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.c
)

# 使用
add_executable(app ${SRC_FILES})
```

### 示例输出

```cmake
# SRC_FILES 的值：
# /path/to/src/main.cpp;/path/to/src/add.cpp;/path/to/src/utils.cpp
```

---

## 二、file(GLOB_RECURSE) - 递归查找

递归查找所有子目录中匹配的文件。

### 基本用法

```cmake
# 递归查找 src 目录及其子目录下的所有 .cpp 文件
file(GLOB_RECURSE ALL_SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 递归查找所有头文件
file(GLOB_RECURSE ALL_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h)
```

### 目录结构示例

```
src/
├── main.cpp
├── utils/
│   └── helper.cpp
└── core/
    └── engine.cpp

# GLOB 只会找到：main.cpp
# GLOB_RECURSE 会找到：main.cpp, helper.cpp, engine.cpp
```

---

## 三、GLOB vs GLOB_RECURSE 对比

| 命令 | 搜索范围 | 适用场景 |
|------|---------|---------|
| `GLOB` | 仅当前目录 | 扁平目录结构 |
| `GLOB_RECURSE` | 当前目录及所有子目录 | 多层目录结构 |

---

## 四、file(GLOB) 的注意事项

### ⚠️ 官方不推荐用于源文件

CMake 官方不推荐用 `GLOB` 收集源文件，原因：

```cmake
# 问题：添加新文件后，CMake 不会自动重新配置
file(GLOB SRC_FILES src/*.cpp)  # 添加新的 .cpp 文件后需要手动重新运行 cmake
```

### ✅ 推荐做法

```cmake
# 显式列出所有源文件
set(SRC_FILES
    src/main.cpp
    src/add.cpp
    src/utils.cpp
)
add_executable(app ${SRC_FILES})
```

### 💡 折中方案

```cmake
# 使用 CONFIGURE_DEPENDS（CMake 3.12+）
# 会在每次构建时检查文件变化
file(GLOB SRC_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
```

---

## 五、file(READ) - 读取文件

```cmake
# 读取整个文件内容
file(READ ${CMAKE_SOURCE_DIR}/version.txt VERSION_CONTENT)
message("版本信息：${VERSION_CONTENT}")

# 读取为十六进制
file(READ ${CMAKE_SOURCE_DIR}/data.bin HEX_CONTENT HEX)
```

---

## 六、file(WRITE) / file(APPEND) - 写入文件

```cmake
# 写入文件（覆盖）
file(WRITE ${CMAKE_BINARY_DIR}/config.h
    "#define VERSION \"1.0.0\"\n"
    "#define BUILD_DATE \"2024-01-01\"\n"
)

# 追加写入
file(APPEND ${CMAKE_BINARY_DIR}/config.h
    "#define DEBUG_MODE 1\n"
)
```

---

## 七、file(COPY) - 复制文件

```cmake
# 复制单个文件
file(COPY ${CMAKE_SOURCE_DIR}/config.json
    DESTINATION ${CMAKE_BINARY_DIR}
)

# 复制整个目录
file(COPY ${CMAKE_SOURCE_DIR}/resources
    DESTINATION ${CMAKE_BINARY_DIR}
)

# 复制并设置权限
file(COPY ${CMAKE_SOURCE_DIR}/script.sh
    DESTINATION ${CMAKE_BINARY_DIR}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
)
```

---

## 八、file(REMOVE) - 删除文件

```cmake
# 删除单个文件
file(REMOVE ${CMAKE_BINARY_DIR}/temp.txt)

# 删除多个文件
file(REMOVE
    ${CMAKE_BINARY_DIR}/temp1.txt
    ${CMAKE_BINARY_DIR}/temp2.txt
)

# 递归删除目录
file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/temp_dir)
```

---

## 九、file(MAKE_DIRECTORY) - 创建目录

```cmake
# 创建目录
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# 创建多层目录
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/output/logs/debug)
```

---

## 十、file(RENAME) - 重命名文件

```cmake
file(RENAME
    ${CMAKE_BINARY_DIR}/old_name.txt
    ${CMAKE_BINARY_DIR}/new_name.txt
)
```

---

## 十一、file(DOWNLOAD) - 下载文件

```cmake
# 下载文件
file(DOWNLOAD
    "https://example.com/file.zip"
    ${CMAKE_BINARY_DIR}/file.zip
    SHOW_PROGRESS
    STATUS DOWNLOAD_STATUS
)

# 检查下载状态
list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
if(NOT STATUS_CODE EQUAL 0)
    message(FATAL_ERROR "下载失败")
endif()
```

---

## 十二、file 命令常用场景

### 场景1：收集源文件

```cmake
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.c
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/include/*.h
    ${CMAKE_SOURCE_DIR}/include/*.hpp
)
add_executable(app ${SOURCES} ${HEADERS})
```

### 场景2：生成配置头文件

```cmake
file(WRITE ${CMAKE_BINARY_DIR}/config.h
    "#pragma once\n"
    "#define PROJECT_NAME \"${PROJECT_NAME}\"\n"
    "#define PROJECT_VERSION \"${PROJECT_VERSION}\"\n"
)
```

### 场景3：复制运行时资源

```cmake
# 复制资源文件到输出目录
file(COPY ${CMAKE_SOURCE_DIR}/resources
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
```

### 场景4：清理临时文件

```cmake
# 在配置时清理旧的临时文件
file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/temp)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/temp)
```

---

## 十三、file vs find 对比

| 特性 | file(GLOB) | find_xxx |
|------|-----------|----------|
| **用途** | 收集文件列表 | 查找特定文件/库 |
| **返回值** | 文件列表 | 单个路径 |
| **搜索范围** | 指定目录 | 系统路径 + 指定路径 |
| **典型场景** | 收集源文件 | 查找第三方库 |

```cmake
# file(GLOB)：收集多个文件
file(GLOB SOURCES src/*.cpp)  # 返回：main.cpp;add.cpp;utils.cpp

# find_library：查找一个库
find_library(CALC_LIB calc)   # 返回：/usr/lib/libcalc.so
```

