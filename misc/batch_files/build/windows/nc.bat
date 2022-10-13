@echo off
pushd .
setlocal

set TARGET="x"
set CFG_FLAGS=
set STEAMWORKS_FLAGS=
set DEMO_FLAGS=

:beginloop
if "%1"=="debug" goto debug
if "%1"=="release" goto endloop rem not supported in this script
if "%1"=="demo" goto demo
if "%1"=="steam" goto steam
if "%1"=="t" goto tgui5_flag
if "%1"=="s" goto shim3_flag
if "%1"=="w" goto wedge3_flag
if "%1"=="g" goto tth_flag
if "%1"=="d" goto data_flag
goto doneloop
:debug
set CFG_FLAGS="-DDEBUG=on"
goto endloop
:steam
set STEAMWORKS_FLAGS="-DSTEAMWORKS=on"
goto endloop
:demo
set DEMO_FLAGS="-DDEMO=on"
goto endloop
:tgui5_flag
set TARGET="t"
goto endloop
:shim3_flag
set TARGET="s"
goto endloop
:wedge3_flag
set TARGET="w"
goto endloop
:tth_flag
set TARGET="g"
goto endloop
:data_flag
set TARGET="d"
goto endloop
:endloop
shift
goto beginloop
:doneloop

if %TARGET%=="t" goto tgui5
if %TARGET%=="s" goto shim3
if %TARGET%=="w" goto wedge3
if %TARGET%=="g" goto tth
if %TARGET%=="d" goto data

echo Invalid target: %TARGET%
goto done

:tgui5
del c:\users\trent\code\t\tgui5.dll
cd c:\users\trent\code\tth\tgui5
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\tth\misc\batch_files\cmake\windows\tgui5.bat %CFG_FLAGS% %STEAMWORKS_FLAGS%
goto done
:shim3
del c:\users\trent\code\t\shim3.dll
cd c:\users\trent\code\tth\shim3
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\tth\misc\batch_files\cmake\windows\shim3.bat %CFG_FLAGS% %STEAMWORKS_FLAGS%
goto done
:wedge3
del c:\users\trent\code\t\wedge3.dll
cd c:\users\trent\code\tth\shim3
cd c:\users\trent\code\wedge3
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\tth\misc\batch_files\cmake\windows\wedge3.bat %CFG_FLAGS% %STEAMWORKS_FLAGS%
goto done
:tth
del "c:\users\trent\code\t\tth.exe"
cd c:\users\trent\code\tth
rmdir /s /q build
mkdir build
cd build
c:\users\trent\code\tth\misc\batch_files\cmake\windows\tth.bat %CFG_FLAGS% %STEAMWORKS_FLAGS% %DEMO_FLAGS%
goto done
:data
rmdir /s /q c:\users\trent\code\t\data
del c:\users\trent\code\t\data.cpa
cd c:\users\trent\code\t
rmdir /s /q data
goto done
:done
endlocal
popd
