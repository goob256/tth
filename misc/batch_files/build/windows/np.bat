@echo off
pushd .
setlocal
if "%1"=="t" goto tgui5
if "%1"=="s" goto shim3
if "%1"=="w" goto wedge3
if "%1"=="g" goto tth
:tgui5
cd c:\users\trent\code\tth\tgui5
git pull --rebase
goto done
:shim3
cd c:\users\trent\code\tth\shim3
git pull --rebase
goto done
:wedge3
cd c:\users\trent\code\tth\wedge3
git pull --rebase
goto done
:tth
cd c:\users\trent\code\tth
git pull --rebase
goto done
:done
endlocal
popd
