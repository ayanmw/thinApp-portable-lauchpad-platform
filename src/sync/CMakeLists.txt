# src/sync/CMakeLists.txt
# 云端同步模块（V2 阶段实现）

# POC 和 MVP 阶段暂不实现
# 此模块为 V2 阶段的云端同步功能预留

# 占位目标
add_custom_target(sync_placeholder
    COMMAND ${CMAKE_COMMAND} -E echo "云端同步模块（V2 阶段实现）"
    COMMENT "同步模块占位符"
)

message(STATUS "配置完成: sync (云端同步模块 - 占位)")
