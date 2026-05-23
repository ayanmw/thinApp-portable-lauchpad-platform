# src/engine/CMakeLists.txt
# Hook 引擎根目录

# 添加引擎子模块
add_subdirectory(common)
add_subdirectory(hook)
add_subdirectory(vfs)
add_subdirectory(vreg)
add_subdirectory(process)

# 创建引擎主库（可选，用于整合所有引擎模块）
# 暂时注释掉，让各模块独立编译
# add_library(engine INTERFACE)
# target_link_libraries(engine
#     INTERFACE
#         engine_hook
#         engine_vfs
#         engine_vreg
#         engine_process
#         engine_common
# )
