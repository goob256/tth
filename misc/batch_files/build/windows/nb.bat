@echo off
pushd .
setlocal

set CFG="relwithdebinfo"
set TARGET="x"

:beginloop
if "%1"=="debug" goto debug
if "%1"=="release" goto release
if "%1"=="demo" goto endloop rem not supported in this script
if "%1"=="steam" goto endloop rem not supported in this script
if "%1"=="t" goto tgui5_flag
if "%1"=="s" goto shim3_flag
if "%1"=="w" goto wedge3_flag
if "%1"=="g" goto tth_flag
if "%1"=="d" goto endloop rem not supported by this script
goto doneloop
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
:release
set CFG="release"
goto endloop
:debug
set CFG="relwithdebinfo"
goto endloop
:endloop
shift
goto beginloop
:doneloop

if %TARGET%=="t" goto tgui5_type
if %TARGET%=="s" goto shim3_type
if %TARGET%=="w" goto wedge3_type
if %TARGET%=="g" goto tth_type

:tgui5_type
if %CFG%=="release" goto tgui5_release
goto tgui5

:shim3_type
if %CFG%=="release" goto shim3_release
goto shim3

:wedge3_type
if %CFG%=="release" goto wedge3_release
goto wedge3

:tth_type
if %CFG%=="release" goto tth_release
goto tth

echo Invalid target: %TARGET%
goto done

:tgui5
if "%2"=="r" goto tgui5_release
cd c:\users\trent\code\tth\tgui5\build
msbuild /p:configuration=relwithdebinfo tgui5.sln
goto done
:tgui5_release
cd c:\users\trent\code\tth\tgui5\build
msbuild /p:configuration=release tgui5.sln
goto done
:shim3
if "%2"=="r" goto shim3_release
cd c:\users\trent\code\tth\shim3\build
msbuild /p:configuration=relwithdebinfo shim3.sln
goto done
:shim3_release
cd c:\users\trent\code\tth\shim3\build
msbuild /p:configuration=release shim3.sln
goto done
:wedge3
if "%2"=="r" goto wedge3_release
cd c:\users\trent\code\wedge3\build
msbuild /p:configuration=relwithdebinfo wedge3.sln
goto done
:wedge3_release
cd c:\users\trent\code\wedge3\build
msbuild /p:configuration=release wedge3.sln
goto done
:tth
if "%2"=="r" goto tth_release
cd c:\users\trent\code\tth\build
msbuild /p:configuration=relwithdebinfo "tth.sln"
goto done
:tth_release
cd c:\users\trent\code\tth\build
msbuild /p:configuration=release "tth.sln"
goto done
:done
endlocal
popd
