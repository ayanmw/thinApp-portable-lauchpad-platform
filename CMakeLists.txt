# CMakeLists.txt - Hook DLL 编译配置
# 作者: AI ThinApp Team
# 日期: 2026-05-23
# 
# 功能: 编译 Hook 引擎 DLL（含文件系统、注册表、进程继承 Hook）
# 依赖: MinHook (已放在 third_party/minhook/)

cmake_minimum_required(VERSION 3.20)
project(HookEngine LANGUAGES CXX)

# ============================================================================
# 配置项
# ============================================================================

# 编译为 DLL
add_library(hook_engine SHARED)

# C++ 标准
target_compile_features(hook_engine PRIVATE cxx_std_17)

# ============================================================================
# 源文件
# ============================================================================

target_sources(hook_engine PRIVATE
    src/engine/hook/hook_engine.cpp
    src/engine/hook/hook_engine.h
    src/engine/hook/vreg_engine.cpp
    src/engine/hook/vreg_engine.h
    src/engine/hook/process_inherit.cpp
    src/engine/hook/process_inherit.h
)

# ============================================================================
# 头文件搜索路径
# ============================================================================

target_include_directories(hook_engine PRIVATE
    src/engine/hook
    third_party/minhook/include
)

# ============================================================================
# 库文件链接
# ============================================================================

# 根据编译架构选择 MinHook 库
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # x64 架构
    target_link_libraries(hook_engine PRIVATE
        third_party/minhook/lib/libMinHook-x64-v141-md.lib
    )
else()
    # x86 架构
    target_link_libraries(hook_engine PRIVATE
        third_party/minhook/lib/libMinHook-x86-v141-md.lib
    )
endif()

# 链接 Windows 系统库
target_link_libraries(hook_engine PRIVATE
    ntdll.lib
    kernel32.lib
    user32.lib
    advapi32.lib
    Shlwapi.lib
)

# ============================================================================
# 编译选项
# ============================================================================

# MSVC 编译选项
if(MSVC)
    # 禁用 RTTI（减小体积）
    target_compile_options(hook_engine PRIVATE /GR-)
    
    # 禁用异常处理（减小体积）
    target_compile_options(hook_engine PRIVATE /EHsc-)
    
    # 使用多字节字符集
    target_compile_definitions(hook_engine PRIVATE _MBCS)
    
    # 去除默认库（减小体积）
    target_link_options(hook_engine PRIVATE /NODEFAULTLIB:libcmt.lib)
    
    # 设置输出文件名
    set_target_properties(hook_engine PROPERTIES
        OUTPUT_NAME "HookEngine"
        PREFIX ""
        SUFFIX ".dll"
    )
endif()

# ============================================================================
# 安装规则（可选）
# ============================================================================

install(TARGETS hook_engine
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
