@echo off
setlocal

cd /d "%~dp0\.."

set "PROFILE=%~1"
if "%PROFILE%"=="" (
    set "PROFILE=Windows-Release"
)

echo.
echo ========================================
echo Boon Installer Pipeline
echo ========================================
echo Profile : %PROFILE%
echo ========================================
echo.

echo [1/3] Creating portable install...
call scripts\MakeInstall.bat "%PROFILE%"
if errorlevel 1 goto :error

echo.
echo [2/3] Building NSIS installer...

where makensis >nul 2>nul
if errorlevel 1 (
    echo ERROR: makensis not found.
    echo Install NSIS and make sure makensis.exe is in PATH.
    goto :error
)

makensis installer\BoonInstaller.nsi
if errorlevel 1 goto :error

echo.
echo [3/3] Done
echo Installer:
echo   installer\BoonEngineSetup.exe
echo.
pause
exit /b 0

:error
echo.
echo ========================================
echo INSTALLER BUILD FAILED
echo ========================================
pause
exit /b 1