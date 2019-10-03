@echo off
@rem correct if needed

SETLOCAL DisableDelayedExpansion

if not exist %~dp0settings.txt (
  (
   echo R_PATH="%ProgramW6432%\R\R-3.3.2"
   echo R_TOOLS=C:\Rtools\bin
   echo VS_TOOLS="C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat"
   echo ARCGIS_DESKTOP_PATH="C:\Program Files (x86)\ArcGIS\Desktop10.6"
   echo ARCGIS_DESKTOP_KIT_PATH="C:\Program Files (x86)\ArcGIS\DeveloperKit10.3"
   echo ARCGIS_PRO_PATH="c:\ArcGISPro"
  )> %~dp0settings.txt
)
for /f "eol=# tokens=1,2 delims==" %%A in (%~dp0settings.txt) do set %%A=%%~B

rem set VS_TOOLS=%VS140COMNTOOLS%\vsvars32.bat
if not defined R_PATH ( echo "R_PATH is not set"
  goto end)
if not defined R_TOOLS ( echo "R_TOOLS is not set"
  goto end)

if not defined VS_TOOLS set VS_TOOLS=C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\Common7\Tools\VsDevCmd.bat

if "%1"=="" goto usage

SETLOCAL
set config=Release
set isbuild=false
set target=
set update_date=false
set scr=%~dp0build.ps1
set plus_build_num=false
set check_only=false
set build_html=false

call :handle_args %*
if %ERRORLEVEL% NEQ 0 (goto end)
if not defined target (set "target=desktop,pro") else (set target=%target:~1%)

powershell -ExecutionPolicy ByPass -NoProfile -File %scr% -config %config% -isbuild %isbuild% -target "%target%" -set_date %update_date% -increment_build %plus_build_num% -check_package %check_only% %build_html%

:end
ENDLOCAL
set R_PATH=
set R_TOOLS=
set ARCGIS_DESKTOP_PATH=
set ARCGIS_DESKTOP_KIT_PATH=
set ARCGIS_PRO_PATH=
exit /b %ERRORLEVEL%
@rem pause

:handle_args
if "%1"=="" goto:eof
if /i "%1"=="debug"    (set config=Debug& set isbuild=true & shift & goto handle_args)
if /i "%1"=="release"  (set config=Release& set isbuild=true & shift & goto handle_args)
if /i "%1"=="pro"      (set "target=%target%,pro"& shift & goto handle_args)
if /i "%1"=="desktop"  (set "target=%target%,desktop"& shift & goto handle_args)
if /i "%1"=="update_date" (set update_date=true& shift & goto handle_args)
if /i "%1"=="+1"       (set plus_build_num=true& shift & goto handle_args)
if /i "%1"=="check"    (set check_only=true& shift & goto handle_args)
if /i "%1"=="html"     (set build_html=true& shift & goto handle_args)
if /i "%1"=="-h" goto usage
if /i "%1"=="-?" goto usage

echo Error: invalid command line option '%1'
:usage
echo "Use: build.bat <pro|desktop> [release|debug] [update_date] [+1 (increment build number)]"
echo examples:
echo create package:                  "build.bat pro desktop"
echo rebuild dlls, create package:    "build.bat pro desktop release"
echo check package:                   "build.bat check"
echo create package for pro only:     "build.bat pro"
echo build debug dlls and package:    "build.bat pro debug"

exit /b 1
