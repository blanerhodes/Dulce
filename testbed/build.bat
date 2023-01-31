REM Build script for testbed
@echo off
SetLocal EnableDelayedExpansion

REM Get a list of all .c files
SET cFilenames=
FOR /R %%f in (*.cpp) do (
    SET cFilenames=!cFilenames! %%f
)

REM echo "Files:" %cFilenames%
SET assembly=testbed
SET compilerFlags=-g 
REM -Wall - Werror
SET includeFlags=-Isrc -I../engine/src/
SET linkerFlags=-L../bin/ -lengine.lib
SET defines=-D_DEBUG -DDIMPORT

ECHO "Building %assembly%..."
clang++ %cFilenames% %compilerFlags% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%