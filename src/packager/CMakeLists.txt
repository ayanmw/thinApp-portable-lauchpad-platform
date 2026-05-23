# src/packager/CMakeLists.txt
# 应用打包工具模块

# 添加子模块
add_subdirectory(capture)
add_subdirectory(editor)
add_subdirectory(vapp)

# 创建打包工具主库（可选）
# add_library(packager INTERFACE)
# target_link_libraries(packager
#     INTERFACE
#         packager_capture
#         packager_editor
#         packager_vapp
# )

message(STATUS "配置完成: packager (应用打包模块)")
