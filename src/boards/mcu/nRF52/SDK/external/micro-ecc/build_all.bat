@ECHO OFF

:: This script will use git (must be in %PATH%) and arm-none-eabi tools in combination with GNU Make 
:: to both fetch and compile all variants of micro-ecc for the nRF5 families
:: pushd and popd are commands that are provided by both Windows and *NIX based OSes.

WHERE >nul 2>nul git 
IF %ERRORLEVEL% NEQ 0 (
ECHO "git was not found in PATH, please install and append to our PATH"
)

IF NOT EXIST micro-ecc/uECC.c (
    ECHO "micro-ecc not found! Let's pull it from HEAD."
	git clone https://github.com/kmackay/micro-ecc.git	
)

pushd nrf51_armgcc\armgcc && make && popd
pushd nrf51_iar\armgcc && make && popd 
pushd nrf51_keil\armgcc && make && popd
pushd nrf52_armgcc\armgcc && make && popd
pushd nrf52_iar\armgcc && make && popd
pushd nrf52_keil\armgcc && make && popd