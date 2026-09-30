@echo off
rem =====================================================================
rem  MSVC Debug build - delegates to build.bat
rem =====================================================================
call "%~dp0build.bat" msvc Debug
pause
