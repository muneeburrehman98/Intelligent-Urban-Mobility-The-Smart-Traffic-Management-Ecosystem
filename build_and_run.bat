@echo off
set PATH=C:\msys64\mingw64\bin;%PATH%
echo Building Project...
C:\msys64\mingw64\bin\g++.exe main.cpp TrafficNetwork.cpp Intersection.cpp Vehicle.cpp -o main.exe
if %errorlevel% neq 0 (
    echo Compilation Failed!
    pause
    exit /b %errorlevel%
)
echo Compilation Successful!
echo Running Simulation...
main.exe > output.txt
pause
