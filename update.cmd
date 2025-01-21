@echo off
setlocal

set REPO_DIR=%~dp0

cd /d %REPO_DIR%

echo Updating repository...
git pull

endlocal
pause
