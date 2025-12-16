@echo off
REM Firmware Flash Script
REM Flashes STM32 MCU using USB port

setlocal

REM Configuration - Set to 1 to run firmware after flashing, 0 to halt
set RUN_AFTER_FLASH=1

REM Define paths
set PROGRAMMER="c:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
set BUILD_DIR=build

REM USB port assignment
set MCU_USB=usb1

REM Set run flag based on configuration
if "%RUN_AFTER_FLASH%"=="1" (
    set RUN_FLAG=-s
) else (
    set RUN_FLAG=
)

REM Check if programmer exists
if not exist %PROGRAMMER% (
    echo ERROR: STM32_Programmer_CLI.exe not found!
    echo Please install STM32CubeProgrammer or update the path in this script.
    exit /b 1
)

REM Check if build directory exists
if not exist "%BUILD_DIR%" (
    echo ERROR: Build directory not found!
    echo Please run 'make debug' or 'make release' first.
    exit /b 1
)

REM Parse command line arguments
set CONFIG=%1

if "%CONFIG%"=="" (
    echo Usage: flash.bat [CONFIG]
    echo.
    echo Configs:
    echo   debug     - Flash debug build
    echo   release   - Flash release build
    echo.
    echo Examples:
    echo   flash.bat debug
    echo   flash.bat release
    exit /b 0
)

REM Validate config
if not "%CONFIG%"=="debug" if not "%CONFIG%"=="release" (
    echo ERROR: Invalid configuration. Use 'debug' or 'release'
    exit /b 1
)

REM Flash firmware
echo.
echo ========================================
echo Flashing Firmware (%CONFIG%)
echo ========================================
set ELF_FILE=%BUILD_DIR%\%CONFIG%\software.elf
if not exist "%ELF_FILE%" (
    echo ERROR: %ELF_FILE% not found!
    exit /b 1
)
echo Connecting to MCU on %MCU_USB%...
%PROGRAMMER% -c port=%MCU_USB% -w "%ELF_FILE%" -v %RUN_FLAG%
if errorlevel 1 (
    echo ERROR: Failed to flash firmware
    exit /b 1
)
echo Firmware flashed successfully!
exit /b 0