# src/engine/process/CMakeLists.txt
# 进程隔离与子进程 Hook 继承模块

# 创建 Process 静态库
add_library(engine_process STATIC
    process_inherit.cpp
)

# 设置目标属性
set_target_properties(engine_process PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "engine_process"
)

# 设置包含目录
target_include_directories(engine_process
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
        ${CMAKE_CURRENT_SOURCE_DIR}/../hook
)

# 链接依赖库
if(TARGET engine_hook)
    target_link_libraries(engine_process
        PRIVATE
            engine_hook
    )
endif()

# 编译选项
if(MSVC)
    target_compile_options(engine_process
        PRIVATE
            /W4
            /WX-
            /O2
    )
    
    # 预处理器定义
    target_compile_definitions(engine_process
        PRIVATE
            _WIN32
            _WINDOWS
            _CRT_SECURE_NO_WARNINGS
    )
endif()

# 安装规则
install(TARGETS engine_process
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

message(STATUS "配置完成: engine_process (进程隔离模块)")
