@echo off

echo Building main...

for /f %%A in ('powershell -NoProfile -Command "[DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()"') do set "START=%%A"

if "%~1"=="" (
  mingw32-make -j%NUMBER_OF_PROCESSORS%
) else (
  mingw32-make %*
)

for /f %%A in ('powershell -NoProfile -Command "[math]::Round(([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds() - %START%) / 1000, 2)"') do set "ELAPSED=%%A"

echo Build finished in %ELAPSED% seconds
