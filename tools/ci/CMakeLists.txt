# tools/ci/CMakeLists.txt
# CI/CD 工具脚本

# POC 阶段：创建占位符
# 实际 CI 配置将在 MVP 阶段实现

# 添加自定义目标（示例）
add_custom_target(ci_placeholder
    COMMAND ${CMAKE_COMMAND} -E echo "CI/CD 工具（待实现）"
    COMMENT "CI 工具占位符"
)

message(STATUS "配置完成: tools/ci (CI/CD 工具 - 占位)")
