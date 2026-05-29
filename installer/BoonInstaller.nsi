!define APP_NAME "Boon Engine"
!define APP_VERSION "0.1.0"
!define APP_PUBLISHER "Boon"
!define INSTALL_DIR "$PROGRAMFILES64\Boon"

Name "${APP_NAME}"
OutFile "BoonEngineSetup.exe"
InstallDir "${INSTALL_DIR}"
RequestExecutionLevel admin

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Install"

    SetOutPath "$INSTDIR"

    ; Clean previous install
    RMDir /r "$INSTDIR"

    SetOutPath "$INSTDIR"

    File /r "..\out\install\Boon\*.*"

    ; Environment variables
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "BOON_INSTALL_ROOT" "$INSTDIR"
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "BOON_SDK_ROOT" "$INSTDIR\SDK"
    WriteRegExpandStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "BOON_ENGINE_ROOT" "$INSTDIR\SDK"

    ; .bproj file association
    WriteRegStr HKCR ".bproj" "" "Boon.Project"
    WriteRegStr HKCR "Boon.Project" "" "Boon Project"
    WriteRegStr HKCR "Boon.Project\DefaultIcon" "" "$INSTDIR\SDK\Boon\Assets\Resources\BoonEngine.ico"
    WriteRegStr HKCR "Boon.Project\shell\open\command" "" '"$INSTDIR\Editor\Editor.exe" "%1"'

    ; Uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Uninstall"

    DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "BOON_INSTALL_ROOT"
    DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "BOON_SDK_ROOT"
    DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "BOON_ENGINE_ROOT"

    DeleteRegKey HKCR ".bproj"
    DeleteRegKey HKCR "Boon.Project"

    RMDir /r "$INSTDIR"

SectionEnd