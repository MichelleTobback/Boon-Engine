@echo off
setlocal

cd /d "%~dp0\.."

set "PROFILE=%~1"
if "%PROFILE%"=="" set "PROFILE=Windows-Debug"

set "CONFIG=Debug"
echo %PROFILE% | findstr /I "Release" >nul
if not errorlevel 1 set "CONFIG=Release"

set "INSTALL_DIR=%CD%\out\install\Boon"

echo.
echo ========================================
echo Boon Install Package
echo ========================================
echo Profile : %PROFILE%
echo Config  : %CONFIG%
echo Output  : %INSTALL_DIR%
echo ========================================
echo.

call "%CD%\scripts\GenerateProject.bat" "%PROFILE%"
if errorlevel 1 goto :error

echo [1/5] Cleaning install folder...
rmdir /s /q "%INSTALL_DIR%" 2>nul

mkdir "%INSTALL_DIR%\Tools"
mkdir "%INSTALL_DIR%\Editor"
mkdir "%INSTALL_DIR%\Runtime"
mkdir "%INSTALL_DIR%\SDK"

echo [2/5] Copying tools...
copy "%CD%\bin\%CONFIG%\Tools\BoonBuild.exe" "%INSTALL_DIR%\Tools\" /Y
copy "%CD%\bin\%CONFIG%\Tools\BClassGenerator.exe" "%INSTALL_DIR%\Tools\" /Y

echo [3/5] Copying editor/runtime...
xcopy "%CD%\bin\%CONFIG%\Editor" "%INSTALL_DIR%\Editor" /E /I /Y
xcopy "%CD%\bin\%CONFIG%\Runtime" "%INSTALL_DIR%\Runtime" /E /I /Y

echo [4/5] Copying SDK...
xcopy "%CD%\Boon" "%INSTALL_DIR%\SDK\Boon" /E /I /Y
copy "%CD%\CMakeLists.txt" "%INSTALL_DIR%\SDK\" /Y
copy "%CD%\BuildProfiles.json" "%INSTALL_DIR%\SDK\" /Y

echo [5/5] Writing setup script...

(
echo @echo off
echo setlocal
echo.
echo set "BOON_INSTALL_ROOT=%%~dp0"
echo for %%%%I in ^("%%BOON_INSTALL_ROOT%%."^) do set "BOON_INSTALL_ROOT=%%%%~fI"
echo set "BOON_SDK_ROOT=%%BOON_INSTALL_ROOT%%\SDK"
echo set "BOON_ENGINE_ROOT=%%BOON_SDK_ROOT%%"
echo.
echo setx BOON_INSTALL_ROOT "%%BOON_INSTALL_ROOT%%" ^>nul
echo setx BOON_SDK_ROOT "%%BOON_SDK_ROOT%%" ^>nul
echo setx BOON_ENGINE_ROOT "%%BOON_ENGINE_ROOT%%" ^>nul
echo.
echo echo Boon installed:
echo echo   BOON_INSTALL_ROOT=%%BOON_INSTALL_ROOT%%
echo echo   BOON_SDK_ROOT=%%BOON_SDK_ROOT%%
echo echo.
echo pause
) > "%INSTALL_DIR%\SetupBoon.bat"

echo.
echo ========================================
echo Install package created
echo ========================================
echo %INSTALL_DIR%
echo ========================================
pause
exit /b 0

:error
echo.
echo ========================================
echo INSTALL PACKAGE FAILED
echo ========================================
pause
exit /b 1