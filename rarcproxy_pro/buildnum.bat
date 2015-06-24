@echo off
FOR /F "skip=1 tokens=3 delims= " %%I IN (..\buildnum.h) DO (set /a u=%%I)
set /a u += 1
echo #pragma once> ..\buildnum.h
echo #define BUILD_NUM %u%>> ..\buildnum.h
