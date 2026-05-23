# src/engine/vfs/CMakeLists.txt
# 虚拟文件系统（VFS）模块

# 创建 VFS 静态库
add_library(engine_vfs STATIC
    vfs.cpp
)

# 设置目标属性
set_target_properties(engine_vfs PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "engine_vfs"
)

# 设置包含目录
target_include_directories(engine_vfs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

# 链接依赖库（如果需要）
# target_link_libraries(engine_vfs
#     PRIVATE
#         engine_common
# )

# 编译选项
if(MSVC)
    target_compile_options(engine_vfs
        PRIVATE
            /W4
            /WX-
            /O2
    )
    
    # 预处理器定义
    target_compile_definitions(engine_vfs
        PRIVATE
            _WIN32
            _WINDOWS
            _CRT_SECURE_NO_WARNINGS
    )
endif()

# 安装规则（可选）
install(TARGETS engine_vfs
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

message(STATUS "配置完成: engine_vfs (虚拟文件系统模块)")
