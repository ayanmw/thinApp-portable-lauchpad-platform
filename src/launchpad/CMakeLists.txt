# src/launchpad/CMakeLists.txt
# Launchpad 桌面客户端模块

# 添加子模块
add_subdirectory(app-manager)
add_subdirectory(store)
add_subdirectory(ui)

# 创建 Launchpad 可执行文件（MVP 阶段）
# POC 阶段暂不创建完整可执行文件

# 占位目标
add_custom_target(launchpad_placeholder
    COMMAND ${CMAKE_COMMAND} -E echo "Launchpad 客户端（待实现）"
    COMMENT "Launchpad 客户端模块占位符"
)

message(STATUS "配置完成: launchpad (Launchpad 客户端模块)")
