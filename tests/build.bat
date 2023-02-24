@ECHO off
SetLocal EnableDelayedExpansion

SET filenames= 
FOR /R %%f in (*.cpp) do (SET filenames=!filenames! %%f)

SET assembly=tests
SET compilerFlags=-g -Wno-missing-braces -Wno-c++11-compat-deprecated-writable-strings -Wno-writable-strings
REM -Wall -Werror -save-temps=obj -O0
SET includeFlags=-Isrc -I../engine/src/
SET linkerFlags=-L../bin/ -lengine.lib
SET defiens=-D_DEBUG -DDIMPORT

ECHO "Building %assembly%..."
clang++ %filenames% %compilerFlags% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%