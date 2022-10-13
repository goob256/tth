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
if "%1"=="d" goto data_flag
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
:data_flag
set TARGET="d"
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

if %TARGET%=="t" goto tgui5
if %TARGET%=="s" goto shim3
if %TARGET%=="w" goto wedge3
if %TARGET%=="g" goto tth
if %TARGET%=="d" goto data

echo Invalid target: %TARGET%
goto done

:tgui5
cd c:\users\trent\code\tth\tgui5\build
if %CFG%=="release" goto tgui5_release
rem copy relwithdebinfo\tgui5.dll ..\..\t
goto done
:tgui5_release
rem copy release\tgui5.dll ..\..\t
goto done
:shim3
cd c:\users\trent\code\tth\shim3\build
if %CFG%=="release" goto shim3_release
rem copy relwithdebinfo\shim3.dll ..\..\t
goto done
:shim3_release
rem copy release\shim3.dll ..\..\t
goto done
:wedge3
cd c:\users\trent\code\wedge3\build
if %CFG%=="release" goto wedge3_release
rem copy relwithdebinfo\wedge3.dll ..\..\t
goto done
:wedge3_release
rem copy release\wedge3.dll ..\..\t
goto done
:tth
cd c:\users\trent\code\tth\build
if %CFG%=="release" goto tth_release
copy "relwithdebinfo\tth.exe" ..\..\t
goto done
:tth_release
copy "release\tth.exe" ..\..\t
goto done
:data
if %CFG%=="release" goto data_release
cd c:\users\trent\code\t
xcopy /q /e /y ..\tth\data data\
copy ..\tth\docs\3rd_party.html .
goto done
:data_release
cd c:\users\trent\code\tth\data
c:\users\trent\code\compress_dir\compress_dir.exe > nul
move ..\data.cpa c:\users\trent\code\t
copy ..\docs\3rd_party.html c:\users\trent\code\t
goto done
:done
endlocal
popd
