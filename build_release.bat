@echo off
rem =====================================================================
rem  MSVC Release build - delegates to build.bat
rem =====================================================================
call "%~dp0build.bat" msvc Release
pause
