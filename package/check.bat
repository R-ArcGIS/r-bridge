@echo off

echo CHECKING

set ORIG_PATH=%PATH%
set Rcmd="C:\Program Files\R\R-3.1.3\bin\x64\Rcmd.exe"

PATH=C:\Rtools\bin;C:\Rtools\gcc-4.6.3\bin;%PATH%

%Rcmd% check --no-multiarch arc

PATH=%ORIG_PATH%
pause
