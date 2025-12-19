@echo off
REM Firmware Flash Script
REM Flashes STM32 MCU using USB port

setlocal

REM Configuration - Set to 1 to run firmware after flashing, 0 to halt
set RUN_AFTER_FLASH=1

REM Define paths
set PROGRAMMER="c:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
set BUILD_DIR=build

REM FUS/BLE firmware filenames and addresses (editable)
set FUS_BIN=stm32wb5x_FUS_fw.bin
set FUS_ADDR=0x0807A000
set BLE_BIN=stm32wb5x_BLE_Stack_full_fw.bin
set BLE_ADDR=0x0805C000

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

echo %CONFIG%



REM If invoked as: flash.bat ffus -> only run FUS upgrade
if "%CONFIG%"=="ffus" (
    goto :RUN_FUS_ONLY
)
REM If invoked as: flash.bat ble -> only run BLE upgrade
if "%CONFIG%"=="ble" (
    goto :RUN_BLE_ONLY
)

REM Validate config
if not "%CONFIG%"=="debug" if not "%CONFIG%"=="release" (
    echo ERROR: Invalid configuration. Use 'debug', 'release', 'ffus' or 'ble'
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
REM No automatic FUS upgrade after flashing in this mode. Use 'flash.bat ffus' to run FUS.

exit /b 0

:RUN_FUS_ONLY
:RUN_FUS_ONLY
echo Running FUS firmware upgrade (standalone)...
if not exist "%FUS_BIN%" (
    echo ERROR: FUS binary %FUS_BIN% not found in current directory
    exit /b 1
) 
%PROGRAMMER% -c port=%MCU_USB% -fwupgrade %FUS_BIN% %FUS_ADDR% firstinstall=0
if errorlevel 1 (
    echo ERROR: FUS upgrade failed
    exit /b 1
)
echo FUS upgrade completed successfully!
exit /b 0

:RUN_BLE_ONLY
echo Running BLE firmware upgrade (standalone)...
if not exist "%BLE_BIN%" (
    echo ERROR: BLE binary %BLE_BIN% not found in current directory
    exit /b 1
) 
%PROGRAMMER% -c port=%MCU_USB% -fwupgrade %BLE_BIN% %BLE_ADDR% firstinstall=0
if errorlevel 1 (
    echo ERROR: BLE upgrade failed
    exit /b 1
)
echo BLE upgrade completed successfully!
exit /b 0