@echo off

rem The script is invoked from the gitlab-ci.yml job without arguments.
rem The script assumes that the following environment variables are set:
rem 
rem   - src_dir: points to the NifTK source folder
rem   - ep_dir: points to the folder where the external projects are (to be) built
rem   - sb_dir: points to the folder where NifTK is to be built (superbuild folder)
rem 
rem The script also assumes that the current directory is %sb_dir%
rem 
rem The script runs the test for a NifTK release mode build.

rem Enable command extensions to allow checking exit status of commands.
setlocal EnableExtensions

set "CMAKE_DIR=c:/CMake/CMake-3.7.2"

echo CMake folder:           %CMAKE_DIR%
echo Project build folder:   %pb_dir%

set BCONF=Release

cd /d "%pb_dir%"
set NIFTK_DRC_ANALYZE=ON

setlocal EnableDelayedExpansion

set "BATFILEPATH=%pb_dir%/bin/startNiftyView_%BCONF%.bat"
set PATHSTRING1=" "
set PATHSTRING2=" "
set PATHSTRING3=" "

echo Reading %BATFILEPATH%....

rem Extracting path via equal sign (???) from startNiftyView_%BCONF%.bat
rem As of 30/11/2015 the path is divided into two parts; we need to take both for proper linking.

set /a counter=1
for /f ^"usebackq^ eol^=^

^ delims^=^" %%a in (%BATFILEPATH%) do (
        if "!counter!"==4 goto :eof
        set var!counter!=%%a
        set /a counter+=1
)

rem Taking the first two lines and removing PATH=
set PATHSTRING1=%var1:/=\%
set PATHSTRING1=%PATHSTRING1:PATH=%
set PATHSTRING1=%PATHSTRING1:~1,-2%

set PATHSTRING2=%var2:/=\%
set PATHSTRING2=%PATHSTRING2:PATH=%
set PATHSTRING2=%PATHSTRING2:~1,-2%

set PATHSTRING3=%var3:/=\%
set PATHSTRING3=%PATHSTRING3:PATH=%
set PATHSTRING3=%PATHSTRING3:~1,-2%

set PATH=%PATHSTRING3%%PATHSTRING2%%PATHSTRING1%%SystemRoot%;%SystemRoot%/system32;%SystemRoot%/System32/Wbem

if defined CUDA_PATH (
  set PATH=%PATH%;%CUDA_PATH%/bin
)

echo PATH: %PATH%

@echo on

"%CMAKE_DIR%/bin/ctest.exe" -S CTestContinuous.cmake -V
if %ERRORLEVEL% NEQ 0 exit /B 4
