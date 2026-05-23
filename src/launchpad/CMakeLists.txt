# CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 项目名称和版本
project(ai-thinapp-launchpad VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准（C++17）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 自动处理 Qt 的 moc、uic、rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找 Qt 6 包（需要 Widgets 模块）
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# 启用 Win32 子系统（Windows 下隐藏控制台窗口）
if (WIN32)
    set(CMAKE_WIN32_EXECUTABLE ON)
endif()

# ===== 源文件 =====
set(SOURCES
    main.cpp
    ui/main_window.cpp
    app_manager.cpp
)

# ===== 头文件 =====
set(HEADERS
    ui/main_window.h
    app_manager.h
)

# ===== 资源文件（可选，POC 阶段不使用）=====
# set(RESOURCES
#     resources/resources.qrc
# )

# ===== 可执行文件 =====
add_executable(launchpad ${SOURCES} ${HEADERS})

# ===== 包含目录 =====
target_include_directories(launchpad PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ===== 链接库 =====
target_link_libraries(launchpad
    Qt6::Widgets
)

# ===== 编译选项 =====
if (MSVC)
    # MSVC 编译器选项
    target_compile_options(launchpad PRIVATE
        /W4     # 警告等级 4
        /wd4251 # 禁用 DLL 接口警告（Qt 需要）
        /wd4996 # 禁用弃用函数警告
    )
else()
    # GCC/Clang 编译器选项
    target_compile_options(launchpad PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# ===== 安装规则（可选）=====
# install(TARGETS launchpad
#     RUNTIME DESTINATION bin
#     BUNDLE DESTINATION .
# )

# ===== 复制 Qt DLL（Windows 部署，可选）=====
# if (WIN32 AND QT6_DEPLOY_QT)
#     find_package(Qt6 REQUIRED COMPONENTS WinDeploymentSupport)
#     include(GNUInstallDirs)
#     qt_generate_deploy_app_script(
#         TARGET launchpad
#         OUTPUT_SCRIPT deploy_script
#         NO_UNSUPPORTED_PLATFORM_ERROR
#     )
#     install(SCRIPT ${deploy_script})
# endif()

# ===== 打印配置信息 =====
message(STATUS "========================================")
message(STATUS "项目: ${PROJECT_NAME}")
message(STATUS "版本: ${PROJECT_VERSION}")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt 版本: ${Qt6_VERSION}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "========================================")
