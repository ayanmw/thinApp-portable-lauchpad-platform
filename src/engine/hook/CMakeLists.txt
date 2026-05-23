# src/engine/hook/CMakeLists.txt
# Hook 引擎模块 - 基于 MinHook 的 API Hook 实现

# 创建 Hook 引擎静态库
add_library(engine_hook STATIC
    hook_engine.cpp
)

# 设置目标属性
set_target_properties(engine_hook PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "engine_hook"
)

# 设置包含目录
target_include_directories(engine_hook
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# 链接依赖库
if(TARGET minhook)
    target_link_libraries(engine_hook
        PUBLIC
            minhook
    )
    message(STATUS "engine_hook: 链接 MinHook 库")
else()
    message(WARNING "engine_hook: MinHook 库未找到，编译可能失败")
endif()

# 编译选项
if(MSVC)
    target_compile_options(engine_hook
        PRIVATE
            /W4     # 警告等级 4
            /WX-    # 不将警告视为错误
            /O2     # 最大化速度优化
    )
    
    # 预处理器定义
    target_compile_definitions(engine_hook
        PRIVATE
            _WIN32
            _WINDOWS
            _CRT_SECURE_NO_WARNINGS
    )
endif()

# 安装规则（可选）
install(TARGETS engine_hook
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

message(STATUS "配置完成: engine_hook (Hook 引擎模块)")
