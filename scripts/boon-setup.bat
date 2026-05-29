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
set "GENERATE_SCRIPT=%BOON_ENGINE_ROOT%\Scripts\GenerateProject.bat"

echo.
echo ========================================
echo Boon Engine Setup
echo ========================================
echo Engine Root:
echo   %BOON_ENGINE_ROOT%
echo ========================================
echo.

if not exist "%BOON_ENGINE_ROOT%\Boon\CMakeLists.txt" (
    echo ERROR: Invalid engine root:
    echo   %BOON_ENGINE_ROOT%
    goto :error
)

if not exist "%ICON_PATH%" (
    echo ERROR: Icon not found:
    echo   %ICON_PATH%
    goto :error
)

rem --------------------------------------------------
rem Save engine root permanently for the user
rem --------------------------------------------------
echo [1/4] Saving BOON_ENGINE_ROOT...
setx BOON_ENGINE_ROOT "%BOON_ENGINE_ROOT%" >nul

if errorlevel 1 (
    echo ERROR: Failed to save BOON_ENGINE_ROOT.
    goto :error
)

rem --------------------------------------------------
rem Register .bproj file type under HKCU
rem --------------------------------------------------
echo [2/4] Registering .bproj file type...

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
rem Editor might not exist yet before first build.
rem --------------------------------------------------
echo [3/4] Registering editor association...

if exist "%EDITOR_EXE%" (
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
) else (
    echo Editor.exe not found yet:
    echo   %EDITOR_EXE%
    echo This is okay if the engine has not been built yet.
)

rem --------------------------------------------------
rem Refresh shell
rem --------------------------------------------------
echo [4/4] Refreshing shell...
ie4uinit.exe -show >nul 2>nul

echo.
echo ========================================
echo Setup Complete
echo ========================================
echo Saved:
echo   BOON_ENGINE_ROOT=%BOON_ENGINE_ROOT%
echo.
echo Registered:
echo   .bproj as %PROGID%
echo.
echo Editor:
echo   %EDITOR_EXE%
echo ========================================
echo.

if exist "%GENERATE_SCRIPT%" (
    choice /M "Build the engine now"

    if errorlevel 2 goto :done

    echo.
    echo ========================================
    echo Building engine
    echo ========================================
    echo.

    call "%GENERATE_SCRIPT%" Windows-Debug

    if errorlevel 1 (
        echo.
        echo Engine build failed.
        goto :error
    )
) else (
    echo Build script not found:
    echo   %GENERATE_SCRIPT%
    echo.
    echo Build manually after setup.
)

:done
echo.
echo Next step:
echo   Scripts\GenerateProject.bat Windows-Debug
echo.
echo If Windows does not show the icon immediately, restart Explorer or sign out/in.
echo.
pause
exit /b 0

:regfail
echo.
echo ERROR: Failed to write one or more registry entries.
goto :error

:error
echo.
echo ========================================
echo Setup Failed
echo ========================================
pause
exit /b 1