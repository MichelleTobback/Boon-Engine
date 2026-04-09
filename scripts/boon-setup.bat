@echo off
setlocal

rem --------------------------------------------------
rem Resolve engine root from this script's location
rem Assumes script lives in: <EngineRoot>\scripts\
rem --------------------------------------------------
set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "BOON_ENGINE_ROOT=%%~fI"

set "PROGID=Boon.Project"
set "EDITOR_RELATIVE_PATH=bin\Debug\Editor\Editor.exe"
set "ICON_RELATIVE_PATH=Boon\Assets\Resources\BoonEngine.ico"

set "EDITOR_EXE=%BOON_ENGINE_ROOT%\%EDITOR_RELATIVE_PATH%"
set "ICON_PATH=%BOON_ENGINE_ROOT%\%ICON_RELATIVE_PATH%"

if not exist "%EDITOR_EXE%" (
    echo ERROR: Editor.exe not found:
    echo   %EDITOR_EXE%
    exit /b 1
)

if not exist "%ICON_PATH%" (
    echo ERROR: Icon not found:
    echo   %ICON_PATH%
    exit /b 1
)

if not exist "%BOON_ENGINE_ROOT%\Boon\CMakeLists.txt" (
    echo ERROR: Invalid engine root:
    echo   %BOON_ENGINE_ROOT%
    exit /b 1
)

rem --------------------------------------------------
rem Save BOON_ENGINE_ROOT permanently for the user
rem --------------------------------------------------
setx BOON_ENGINE_ROOT "%BOON_ENGINE_ROOT%" >nul
if errorlevel 1 (
    echo ERROR: Failed to save BOON_ENGINE_ROOT
    exit /b 1
)

rem --------------------------------------------------
rem Register .bproj file type under HKCU
rem --------------------------------------------------
reg add "HKCU\Software\Classes\.bproj" /ve /d "%PROGID%" /f >nul
if errorlevel 1 goto :regfail

reg add "HKCU\Software\Classes\%PROGID%" /ve /d "Boon Project" /f >nul
if errorlevel 1 goto :regfail

reg add "HKCU\Software\Classes\%PROGID%\DefaultIcon" ^
    /ve /t REG_SZ ^
    /d "\"%ICON_PATH%\"" /f >nul
if errorlevel 1 goto :regfail

reg add "HKCU\Software\Classes\%PROGID%\shell" /ve /d "open" /f >nul
if errorlevel 1 goto :regfail

reg add "HKCU\Software\Classes\%PROGID%\shell\open" /ve /d "Open" /f >nul
if errorlevel 1 goto :regfail

reg add "HKCU\Software\Classes\%PROGID%\shell\open\command" ^
    /ve /t REG_SZ ^
    /d "\"%EDITOR_EXE%\" \"%%1\"" /f >nul
if errorlevel 1 goto :regfail

rem --------------------------------------------------
rem Register Editor.exe in "Open with"
rem --------------------------------------------------
reg add "HKCU\Software\Classes\Applications\Editor.exe\shell\open\command" ^
    /ve /t REG_SZ ^
    /d "\"%EDITOR_EXE%\" \"%%1\"" /f >nul
if errorlevel 1 goto :regfail

reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.bproj\OpenWithProgids" ^
    /v "%PROGID%" /t REG_NONE /d "" /f >nul
if errorlevel 1 goto :regfail

reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.bproj\OpenWithList" ^
    /v "a" /d "Editor.exe" /f >nul
if errorlevel 1 goto :regfail

reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.bproj\OpenWithList" ^
    /v "MRUList" /d "a" /f >nul
if errorlevel 1 goto :regfail

rem --------------------------------------------------
rem Refresh shell
rem --------------------------------------------------
ie4uinit.exe -show >nul 2>nul

echo.
echo Saved BOON_ENGINE_ROOT=%BOON_ENGINE_ROOT%
echo Registered .bproj as %PROGID%
echo Open command:
echo   "%EDITOR_EXE%" "%%1"
echo Icon:
echo   %ICON_PATH%
echo.
echo IMPORTANT:
echo Windows may still require you to choose the default app once.
echo To finish:
echo   1. Right-click a .bproj file
echo   2. Open with
echo   3. Choose another app
echo   4. Select Editor.exe
echo   5. Check "Always use this app"
echo.
echo You may need to restart Explorer or sign out/in before the icon updates.
exit /b 0

:regfail
echo ERROR: Failed to write one or more registry entries.
exit /b 1