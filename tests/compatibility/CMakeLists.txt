# tests/compatibility/CMakeLists.txt
# 兼容性测试模块

# POC 阶段：创建占位符
# 实际测试将在 MVP 阶段实现

if(BUILD_TESTING)
    message(STATUS "兼容性测试模块配置（占位符）")
    
    # 添加自定义目标
    add_custom_target(test_compatibility
        COMMAND ${CMAKE_COMMAND} -E echo "兼容性测试（待实现）"
        COMMENT "运行兼容性测试"
    )
endif()

message(STATUS "配置完成: tests/compatibility (兼容性测试 - 占位)")
