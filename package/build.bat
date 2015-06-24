@echo off

::::: install Rtools from http://cran.r-project.org/bin/windows/Rtools
::::: adjust system PATH

set pkg_name=arcgisbinding
set pkg_main_ver=1.0.0
echo Building '%pkg_name%' package

set ORIG_PATH=%PATH%
set Rbinpath="C:\Program Files\R\R-3.1.3\bin\x64"
set Rcmd=%Rbinpath%\Rcmd.exe
set Rscript=%Rbinpath%\Rscript.exe

PATH=C:\Rtools\bin;C:\Rtools\gcc-4.6.3\bin;%PATH%

:::: add build number to the package version - first line
FOR /F "skip=1 tokens=3 delims= " %%I IN (..\buildnum.h) DO (set /a u=%%I)
set pkg_full_ver=%pkg_main_ver%.%u%
echo Build Number: %u%
echo Package: %pkg_name%> arc/DESCRIPTION.new
echo Version: %pkg_full_ver%>> arc/DESCRIPTION.new

::: add current date - second line
For /f "tokens=2-4 delims=/ " %%a in ('date /t') do (set curdate=%%c-%%a-%%b)
echo Date: %curdate%>> arc/DESCRIPTION.new

::: copy rest of the DESCRIPTION
FOR /F "skip=3 tokens=*" %%I IN (arc/DESCRIPTION) DO (echo %%I>> arc/DESCRIPTION.new)

:::: compare and override new file with current
FC /W arc\DESCRIPTION.new arc\DESCRIPTION
IF ERRORLEVEL 1 (mv arc/DESCRIPTION.new arc/DESCRIPTION) ELSE (rm -f arc/DESCRIPTION.new)

:::: build roxygen docs
set PKG_DIR=%CD:\=^/%/arc
%Rscript% --vanilla --verbose -e "roxygen2::roxygenize('%PKG_DIR%')"

:::: build API pdf
IF NOT EXIST arc/inst/doc/ (mkdir arc\inst\doc)
%Rcmd% Rd2pdf --batch --no-preview --force --pdf arc -o arc/inst/doc/%pkg_name%.pdf

IF ERRORLEVEL 0 (
  IF EXIST ..\..\%pkg_name%\ (rm -R ../../%pkg_name%/*)
  %Rcmd% INSTALL --build --clean --compile-both --html --library=../../ arc
  move /Y %pkg_name%_%pkg_full_ver%.zip ../../
  rm -f arc/src/*.dll
)

PATH=%ORIG_PATH%
pause
