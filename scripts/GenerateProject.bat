@echo off
setlocal

cd /d "%~dp0\.."

set VS_DEV_CMD=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat

if not exist "%VS_DEV_CMD%" (
    echo Could not find VsDevCmd.bat:
    echo %VS_DEV_CMD%
    exit /b 1
)

call "%VS_DEV_CMD%" -arch=x64 -host_arch=x64

echo ========================================
echo Building BoonBuild
echo ========================================

cmake -S Boon/Tools/BoonBuild ^
      -B out/build/BoonBuild-Debug ^
      -G Ninja ^
      -DCMAKE_BUILD_TYPE=Debug

if errorlevel 1 exit /b 1

cmake --build out/build/BoonBuild-Debug

if errorlevel 1 goto :error

echo ========================================
echo Generating modules
echo ========================================

bin\Debug\Tools\BoonBuild.exe .

if errorlevel 1 goto :error

echo ========================================
echo Generating engine project
echo ========================================

cmake -S . ^
      -B out/build/x64-Debug ^
      -G Ninja ^
      -DCMAKE_BUILD_TYPE=Debug

if errorlevel 1 goto :error

echo ========================================
echo Done
echo ========================================

echo.
echo ========================================
echo Press any key to close...
echo ========================================

pause >nul
exit /b 0

:error
echo.
echo ========================================
echo Generation FAILED
echo ========================================

pause >nul
exit /b 1