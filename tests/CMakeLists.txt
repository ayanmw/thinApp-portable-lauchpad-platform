# tests/CMakeLists.txt
# 测试根目录

# 添加测试子模块
add_subdirectory(engine)
add_subdirectory(compatibility)
add_subdirectory(performance)

message(STATUS "配置完成: tests (测试模块)")
