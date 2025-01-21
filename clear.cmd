@echo off
setlocal

for /d /r %%i in (build) do (
    if exist "%%i" (
        echo "Delete": %%i
        rmdir /s /q "%%i"
    )
)

for /d /r %%i in (logs) do (
    if exist "%%i" (
        echo "Delete": %%i
        rmdir /s /q "%%i"
    )
)

for /d /r %%i in (database) do (
    if exist "%%i" (
        echo "Delete": %%i
        rmdir /s /q "%%i"
    )
)

echo "All build folders was deleted."
endlocal
pause
