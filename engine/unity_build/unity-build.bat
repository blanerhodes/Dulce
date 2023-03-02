
REM Build script for engine
@echo off
SetLocal EnableDelayedExpansion

REM Get a list of all .cpp files
SET cFilenames=../engine/unity_build/unity_build.cpp
REM FOR /R %%f in (*.cpp) do (
REM     SET cFilenames=!cFilenames! %%f
REM )

REM echo "Files:" %cFilenames%
SET assembly=engine
SET compilerFlags=-g -shared -Wvarargs -Wall -Werror -Wno-c++11-compat-deprecated-writable-strings -Wno-writable-strings -Wno-missing-braces
REM -Wall - Werror
SET includeFlags=-Isrc -I%VULKAN_SDK%/Include
SET linkerFlags=-luser32 -lvulkan-1 -L%VULKAN_SDK%/Lib
SET defines=-D_DEBUG -DDEXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%..."

clang++ %cFilenames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%