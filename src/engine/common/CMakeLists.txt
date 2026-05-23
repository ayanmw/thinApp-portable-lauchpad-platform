# src/engine/common/CMakeLists.txt
# 引擎公共工具模块

# 创建公共工具静态库
add_library(engine_common STATIC
    common.cpp
)

# 设置目标属性
set_target_properties(engine_common PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "engine_common"
)

# 设置包含目录
target_include_directories(engine_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# 链接依赖库（如果需要）
# target_link_libraries(engine_common
#     PRIVATE
#         # 添加需要的库
# )

# 编译选项
if(MSVC)
    target_compile_options(engine_common
        PRIVATE
            /W4
            /WX-
            /O2
    )
    
    # 预处理器定义
    target_compile_definitions(engine_common
        PRIVATE
            _WIN32
            _WINDOWS
            _CRT_SECURE_NO_WARNINGS
    )
endif()

# 安装规则
install(TARGETS engine_common
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

message(STATUS "配置完成: engine_common (公共工具模块)")
