# src/engine/vreg/CMakeLists.txt
# 虚拟注册表（VReg）模块

# 创建 VReg 静态库
add_library(engine_vreg STATIC
    vreg.cpp
)

# 设置目标属性
set_target_properties(engine_vreg PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "engine_vreg"
)

# 设置包含目录
target_include_directories(engine_vreg
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

# 链接依赖库（如果需要）
# target_link_libraries(engine_vreg
#     PRIVATE
#         engine_common
# )

# 编译选项
if(MSVC)
    target_compile_options(engine_vreg
        PRIVATE
            /W4
            /WX-
            /O2
    )
    
    # 预处理器定义
    target_compile_definitions(engine_vreg
        PRIVATE
            _WIN32
            _WINDOWS
            _CRT_SECURE_NO_WARNINGS
    )
endif()

# 安装规则
install(TARGETS engine_vreg
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

message(STATUS "配置完成: engine_vreg (虚拟注册表模块)")
