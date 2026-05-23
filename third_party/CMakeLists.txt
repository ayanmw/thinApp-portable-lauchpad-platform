# third_party/CMakeLists.txt
# 第三方依赖：MinHook 库

# MinHook 是一个轻量级的 x86/x64 API Hook 库
# 项目地址：https://github.com/TsudaKageyu/minhook

# 检查 MinHook 源码是否存在
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/minhook")
    message(STATUS "MinHook 源码未找到，正在解压...")
    
    # 解压 minhook.zip（如果存在）
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/minhook.zip")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xvf minhook.zip
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE UNZIP_RESULT
        )
        if(UNZIP_RESULT EQUAL 0)
            message(STATUS "MinHook 解压成功")
            # 重命名解压后的目录为 minhook
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/minhook-master")
                file(RENAME 
                    "${CMAKE_CURRENT_SOURCE_DIR}/minhook-master"
                    "${CMAKE_CURRENT_SOURCE_DIR}/minhook"
                )
            endif()
        else()
            message(WARNING "MinHook 解压失败，请手动解压 minhook.zip")
        endif()
    else()
        message(WARNING "MinHook 源码包 (minhook.zip) 未找到")
        message(WARNING "请下载 MinHook 源码并放置到 third_party/minhook 目录")
        message(WARNING "下载地址：https://github.com/TsudaKageyu/minhook/releases")
    endif()
endif()

# 如果 MinHook 源码存在，添加为子目录
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/minhook/CMakeLists.txt")
    message(STATUS "找到 MinHook 源码，添加到构建")
    add_subdirectory(minhook)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/minhook/include/MinHook.h")
    # 如果只有头文件和源文件，手动编译
    message(STATUS "手动编译 MinHook 库")
    
    # 收集 MinHook 源文件
    set(MINHOOK_SRC
        minhook/src/buffer.c
        minhook/src/hook.c
        minhook/src/trampoline.c
        minhook/src/hde/hde32.c
        minhook/src/hde/hde64.c
        minhook/src/hde/table32.h
        minhook/src/hde/table64.h
    )
    
    # 创建静态库
    add_library(minhook STATIC ${MINHOOK_SRC})
    
    # 设置包含目录
    target_include_directories(minhook
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/minhook/include
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/minhook/src
    )
    
    # 设置编译选项
    if(MSVC)
        target_compile_options(minhook PRIVATE /W3 /WX-)
    endif()
    
    message(STATUS "MinHook 静态库配置完成")
else()
    # 创建占位目标（用于编译测试）
    message(WARNING "MinHook 未找到，创建占位目标")
    add_library(minhook INTERFACE)
    message(WARNING "请安装 MinHook 后再进行完整构建")
    message(WARNING "或使用 vcpkg: vcpkg install minhook")
endif()

# 导出 MinHook 目标供其他模块使用
if(TARGET minhook)
    set(MINHOOK_FOUND TRUE CACHE BOOL "MinHook 库是否找到")
    set(MINHOOK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/minhook/include" CACHE PATH "MinHook 头文件目录")
    set(MINHOOK_LIBRARY minhook CACHE STRING "MinHook 库目标")
endif()
