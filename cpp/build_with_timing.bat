@echo off
setlocal EnableDelayedExpansion

:: Enable ANSI escape codes
for /f "tokens=2 delims=: " %%i in ('"prompt $E & for %%E in (1) do rem"') do set "ESC=%%i"

:: Create build directory if it doesn't exist
if not exist build mkdir build
cd build

echo  [32m=== Build and Compilation Time Statistics === [0m
echo.

:: Time the CMake configuration step
echo  [94mStarting CMake configuration... [0m
set CONFIG_START=%time%

:: Run CMake configuration
cmake ..

set CONFIG_END=%time%

:: Calculate configuration time
call :TimeDiff CONFIG_TIME "%CONFIG_START%" "%CONFIG_END%"
echo  [32mCMake configuration completed in %CONFIG_TIME% seconds [0m
echo.

:: Time the compilation step
echo  [94mStarting compilation... [0m
set COMPILE_START=%time%

:: Run compilation
cmake --build . --config Release

set COMPILE_END=%time%

:: Calculate compilation time
call :TimeDiff COMPILE_TIME "%COMPILE_START%" "%COMPILE_END%"
echo  [32mCompilation completed in %COMPILE_TIME% seconds [0m
echo.

:: Calculate total time with decimal values
for /F "tokens=1,2 delims=." %%a in ("%CONFIG_TIME%") do (
    set CONFIG_TIME_INT=%%a
    set CONFIG_TIME_DEC=%%b
)

for /F "tokens=1,2 delims=." %%a in ("%COMPILE_TIME%") do (
    set COMPILE_TIME_INT=%%a
    set COMPILE_TIME_DEC=%%b
)

set /A TOTAL_TIME_INT=CONFIG_TIME_INT+COMPILE_TIME_INT
set /A TOTAL_TIME_DEC=CONFIG_TIME_DEC+COMPILE_TIME_DEC

if !TOTAL_TIME_DEC! GEQ 100 (
    set /A TOTAL_TIME_INT+=1
    set /A TOTAL_TIME_DEC-=100
)

:: Format the total time
if !TOTAL_TIME_DEC! LSS 10 (
    set TOTAL_TIME=!TOTAL_TIME_INT!.0!TOTAL_TIME_DEC!
) else (
    set TOTAL_TIME=!TOTAL_TIME_INT!.!TOTAL_TIME_DEC!
)

:: Print summary
echo [32m=== Build Time Summary ===[0m
echo CMake Configuration: %CONFIG_TIME% seconds
echo Compilation:         %COMPILE_TIME% seconds
echo Total Build Time:    !TOTAL_TIME! seconds

cd ..
goto :eof

:TimeDiff
:: This function calculates the time difference between two timestamps
:: and returns the result in seconds
setlocal

set start=%~2
set end=%~3

:: Extract hours, minutes, seconds, and centiseconds
for /F "tokens=1-4 delims=:." %%a in ("%start%") do (
   set /A "start_h=%%a"
   set /A "start_m=1%%b-100"
   set /A "start_s=1%%c-100"
   set /A "start_cs=1%%d-100"
)

for /F "tokens=1-4 delims=:." %%a in ("%end%") do (
   set /A "end_h=%%a"
   set /A "end_m=1%%b-100"
   set /A "end_s=1%%c-100"
   set /A "end_cs=1%%d-100"
)

:: Calculate total centiseconds for start and end times
set /A start_total_cs=start_h*360000 + start_m*6000 + start_s*100 + start_cs
set /A end_total_cs=end_h*360000 + end_m*6000 + end_s*100 + end_cs

:: Handle midnight crossing
if %end_total_cs% LSS %start_total_cs% set /A end_total_cs+=8640000

:: Calculate difference in centiseconds
set /A diff_cs=end_total_cs-start_total_cs

:: Convert to seconds with two decimal places
set /A diff_s=diff_cs/100
set /A diff_cs_remainder=diff_cs%%100

:: Format the result
if %diff_cs_remainder% LSS 10 (
    set diff_formatted=%diff_s%.0%diff_cs_remainder%
) else (
    set diff_formatted=%diff_s%.%diff_cs_remainder%
)

endlocal & set %~1=%diff_formatted%
goto :eof