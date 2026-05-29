@echo off
setlocal

cd /d "%~dp0\.."

set PROFILE=%~1
if "%PROFILE%"=="" (
    set PROFILE=Windows-Debug
)

echo.
echo ========================================
echo Boon Engine Build Pipeline
echo ========================================
echo Profile : %PROFILE%
echo Root    : %CD%
echo ========================================
echo.

echo [Setup] Finding Visual Studio...

set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe

if not exist "%VSWHERE%" (
    echo Could not find vswhere.exe.
    echo Install Visual Studio Build Tools or Visual Studio Community.
    goto :error
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set VS_INSTALL=%%i
)

if "%VS_INSTALL%"=="" (
    echo Could not find Visual Studio C++ tools.
    echo Install "Desktop development with C++".
    goto :error
)

set VS_DEV_CMD=%VS_INSTALL%\Common7\Tools\VsDevCmd.bat

if not exist "%VS_DEV_CMD%" (
    echo Could not find VsDevCmd.bat:
    echo %VS_DEV_CMD%
    goto :error
)

call "%VS_DEV_CMD%" -arch=x64 -host_arch=x64

echo.
echo [1/5] Configuring BoonBuild...

cmake -S Boon/tools/BoonBuild ^
      -B out/build/BoonBuild-Debug ^
      -G Ninja ^
      -DCMAKE_BUILD_TYPE=Debug

if errorlevel 1 goto :error

echo.
echo [2/5] Building BoonBuild...

cmake --build out/build/BoonBuild-Debug

if errorlevel 1 goto :error

echo.
echo [3/5] Running BoonBuild...

bin\Debug\Tools\BoonBuild.exe . --profile %PROFILE%

if errorlevel 1 goto :error

echo.
echo [4/5] Configuring CMake preset...

cmake --preset %PROFILE%

if errorlevel 1 goto :error

echo.
echo [5/5] Building engine...

cmake --build --preset %PROFILE%

if errorlevel 1 goto :error

echo.
echo ========================================
echo Build Complete
echo ========================================
pause
exit /b 0

:error
echo.
echo ========================================
echo BUILD FAILED
echo ========================================
pause
exit /b 1