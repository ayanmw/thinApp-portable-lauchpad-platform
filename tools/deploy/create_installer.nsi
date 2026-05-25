; NSIS 安装脚本 for AI ThinApp Portable Launchpad Platform
; 安装 launchpad.exe、hook_engine.dll、Qt6 DLLs
; 创建桌面快捷方式、开始菜单文件夹
; 注册卸载程序（控制面板 → 卸载程序）
; 支持静默安装（/SILENT 参数）

;================================================
; 定义常量和宏
;================================================

; 编译器选项
SetCompressor lzma
SetCompressorDictSize 32

; 基本信息
!define APP_NAME "AI ThinApp Portable Launchpad"
!define APP_VERSION "0.1.0"
!define APP_PUBLISHER "AI ThinApp Team"
!define APP_URL "https://example.com/thinapp"
!define APP_COPYRIGHT "Copyright (C) 2026 AI ThinApp Team"
!define APP_EXE "launchpad.exe"
!define APP_DLL "hook_engine.dll"

; 安装目录
InstallDir "$PROGRAMFILES64\AI ThinApp Portable Launchpad"
InstallDirRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "InstallLocation"

; 输出文件
!define OUTPUT_FILE "launchpad-v${APP_VERSION}-setup.exe"

;================================================
; 属性设置
;================================================

Name "${APP_NAME} ${APP_VERSION}"
OutFile "${OUTPUT_FILE}"
Unicode True
RequestExecutionLevel admin

; 界面设置
!include "MUI2.nsh"

; 现代 UI 设置
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; 卸载页面
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; 语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

; 图标（如果有的话）
; !define MUI_ICON "icon.ico"
; !define MUI_UNICON "icon.ico"

;================================================
; 版本信息
;================================================

VIProductVersion "${APP_VERSION}.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "ProductVersion" "${APP_VERSION}"
VIAddVersionKey "CompanyName" "${APP_PUBLISHER}"
VIAddVersionKey "LegalCopyright" "${APP_COPYRIGHT}"
VIAddVersionKey "FileDescription" "${APP_NAME} 安装程序"
VIAddVersionKey "FileVersion" "${APP_VERSION}"

;================================================
; 安装段
;================================================

Section "主程序" SecMain
    SectionIn RO  ; 只读，不可取消
    
    ; 设置输出目录
    SetOutPath "$INSTDIR"
    
    ; 复制主程序
    File "..\build\output\x64\Release\launchpad.exe"
    File "..\build\output\x64\Release\hook_engine.dll"
    
    ; 复制 Qt6 DLLs
    File "..\build\output\x64\Release\Qt6Core.dll"
    File "..\build\output\x64\Release\Qt6Gui.dll"
    File "..\build\output\x64\Release\Qt6Widgets.dll"
    File "..\build\output\x64\Release\Qt6Network.dll"
    
    ; 复制文档
    File "..\README.md"
    File "..\LICENSE"
    
    ; 创建卸载程序
    WriteUninstaller "$INSTDIR\uninstall.exe"
    
    ; 注册卸载信息（控制面板 → 卸载程序）
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "URLInfoAbout" "${APP_URL}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "DisplayIcon" "$INSTDIR\${APP_EXE}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "QuietUninstallString" '"$INSTDIR\uninstall.exe" /SILENT'
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "NoRepair" 1
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "InstallDate" "${APP_COPYRIGHT}"
    
    ; 计算卸载程序大小
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "EstimatedSize" "$0"
    
SectionEnd

Section "快捷方式" SecShortcuts
    
    ; 创建桌面快捷方式
    CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
    
    ; 创建开始菜单文件夹和快捷方式
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}"
    CreateShortcut "$SMPROGRAMS\${APP_NAME}\卸载 ${APP_NAME}.lnk" "$INSTDIR\uninstall.exe"
    
SectionEnd

;================================================
; 卸载段
;================================================

Section "Uninstall"
    
    ; 删除文件
    Delete "$INSTDIR\launchpad.exe"
    Delete "$INSTDIR\hook_engine.dll"
    Delete "$INSTDIR\Qt6Core.dll"
    Delete "$INSTDIR\Qt6Gui.dll"
    Delete "$INSTDIR\Qt6Widgets.dll"
    Delete "$INSTDIR\Qt6Network.dll"
    Delete "$INSTDIR\README.md"
    Delete "$INSTDIR\LICENSE"
    Delete "$INSTDIR\uninstall.exe"
    
    ; 删除快捷方式
    Delete "$DESKTOP\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\卸载 ${APP_NAME}.lnk"
    RMDir "$SMPROGRAMS\${APP_NAME}"
    
    ; 删除安装目录（如果为空）
    RMDir "$INSTDIR"
    
    ; 删除注册表项
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad"
    
SectionEnd

;================================================
; 描述字符串（鼠标悬停时显示）
;================================================

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "安装 ${APP_NAME} 主程序（必需）"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} "创建桌面和开始菜单快捷方式"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;================================================
; 静默安装支持
;================================================

; 如果用户传递了 /SILENT 参数，则不显示界面
SilentInstall silent

;================================================
; 完成
;================================================

; 显示完成页面
!insertmacro MUI_LANGUAGE "SimpChinese"

; 完成消息
LangString COMPLETED 1033 "安装完成！"
LangString COMPLETED 2052 "安装完成！"

; 运行程序选项
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "立即运行 ${APP_NAME}"
!insertmacro MUI_PAGE_FINISH

;================================================
; 结束
;================================================

; 安装程序初始化
Function .onInit
    ; 检查是否已有旧版本
    ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AIThinAppLaunchpad" "UninstallString"
    IfFileExists "$0" 0 +3
        MessageBox MB_OKCANCEL|MB_ICONINFORMATION "检测到已安装的版本，将继续安装并覆盖。" IDOK +2
        Abort
FunctionEnd

; 卸载程序初始化
Function un.onInit
    MessageBox MB_OKCANCEL|MB_ICONQUESTION "确定要卸载 ${APP_NAME} 吗？" IDOK +2
    Abort
FunctionEnd

;================================================
; 结束安装
;================================================

; 安装成功后的提示
Function .onInstSuccess
    MessageBox MB_OK|MB_ICONINFORMATION "$(COMPLETED)"
FunctionEnd
