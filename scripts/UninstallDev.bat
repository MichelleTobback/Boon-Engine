@echo off
setlocal

echo Removing BOON_INSTALL_ROOT...
reg delete "HKCU\Environment" /v BOON_INSTALL_ROOT /f
reg delete "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v BOON_INSTALL_ROOT /f

echo Removing BOON_ENGINE_ROOT...
reg delete "HKCU\Environment" /v BOON_ENGINE_ROOT /f
reg delete "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v BOON_ENGINE_ROOT /f

echo Removing BOON_SDK_ROOT...
reg delete "HKCU\Environment" /v BOON_SDK_ROOT /f
reg delete "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v BOON_SDK_ROOT /f

echo Removing C:\Program Files\Boon...
if exist "C:\Program Files\Boon" rmdir /s /q "C:\Program Files\Boon"

echo Done.
pause