#!/bin/bash

# This script will use git (must be in $PATH) and arm-none-eabi tools in combination with GNU Make 
# to both fetch and compile all variants of micro-ecc for the nRF5 families
# pushd and popd are commands that are provided by both Windows and *NIX based OSes.

if ! [ -x "$(command -v git)" ]; then
  echo 'git is not installed. Please install and append to PATH' >&2
  exit
fi

if [ ! -f micro-ecc/uECC.c ]; then
    echo "micro-ecc not found! Let's pull it from HEAD."
	git clone https://github.com/kmackay/micro-ecc.git	
fi

pushd nrf51_armgcc/armgcc && make && popd &&
pushd nrf51_iar/armgcc && make && popd &&
pushd nrf51_keil/armgcc && make && popd &&
pushd nrf52_armgcc/armgcc && make && popd &&
pushd nrf52_iar/armgcc && make && popd &&
pushd nrf52_keil/armgcc && make && popd