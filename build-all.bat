@ECHO off
REM Build Everything

ECHO "Building everything..."

PUSHD engine
CALL build.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

PUSHD testbed
CALL build.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

PUSHD tests
CALL build.bat
POPD
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

@REM REM engine
@REM make -f "Makefile.engine.windows.mak" all
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)
@REM 
@REM REM testbed
@REM make -f "Makefile.testbed.windows.mak" all
@REM IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)
@REM 
ECHO "All assemblies built successfully."