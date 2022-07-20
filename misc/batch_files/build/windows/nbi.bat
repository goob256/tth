@echo off
pushd .
setlocal
if "%1"=="t" goto tgui5
if "%1"=="s" goto shim3
if "%1"=="w" goto wedge3
if "%1"=="g" goto tth
:tgui5
call nb.bat t %2
call ni.bat t %2 %3 %4
goto done
:shim3
call nb.bat s %2
call ni.bat s %2 %3 %4
goto done
:wedge3
call nb.bat w %2
call ni.bat w %2 %3 %4
goto done
:tth
call nb.bat g %2
call ni.bat g %2 %3 %4
goto done
:done
endlocal
popd
