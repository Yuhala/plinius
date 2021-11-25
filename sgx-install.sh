#!/bin/bash

#
# Script to install the Intel SGX driver, SDK, and PSW on Ubuntu based systems.
# Tested on Ubuntu 18.04 and sgx_2.11
# Copyright (c) Peterson Yuhala, IIUN
# Based on sgx install script from voltpillager
#


# Exit immediately if a command exits with a non-zero status
set -e


# Ubuntu release
OS_REL=$(lsb_release -sr)
# Current working directory
script_path=$(pwd)
# SGX version tag
sgx_sdk_version="sgx_2.11"
# SGX driver version tag
sgx_driver_version="sgx_driver_2.11"
# Debug info
debug_info=1


# TODO: factorize these packages...
if [ $OS_REL = "16.04" ];
then 
# SGX SDK requirements
sudo apt-get install build-essential ocaml automake autoconf libtool wget python libssl-dev git cmake perl -y
# SGX PSW requirements
sudo apt-get install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip -y

elif [ $OS_REL = "18.04" ];
then 
# SGX SDK requirements
sudo apt-get install build-essential ocaml ocamlbuild automake autoconf libtool wget python libssl-dev git cmake perl -y
# SGX PSW requirements
sudo apt-get install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip -y


elif [ $OS_REL = "20.04" ];
then 
# SGX SDK requirements
sudo apt-get install build-essential ocaml ocamlbuild automake autoconf libtool wget python-is-python3 libssl-dev git cmake perl -y
# SGX PSW requirements
sudo apt-get install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip -y

fi

# Install additional requirements for 16.04, 18.04, 20.04
sudo apt-get install libssl-dev libcurl4-openssl-dev protobuf-compiler libprotobuf-dev debhelper cmake reprepro unzip -y 


#git submodule init
#git submodule update

#OS_ID=$(lsb_release -si | tr '[:upper:]' '[:lower:]')
#OS_REL=$(lsb_release -sr)
#OS_STR=$OS_ID$OS_REL

# ----------------------------------------------------------------------
# Stop aesmd service
# sudo service aesmd stop
# Install SGX Driver
git clone https://github.com/intel/linux-sgx-driver.git
cd linux-sgx-driver
git fetch --all --tags
git checkout tags/$sgx_driver_version

sudo apt-get -yqq install linux-headers-$(uname -r)
make
sudo mkdir -p "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
sudo cp isgx.ko "/lib/modules/"`uname -r`"/kernel/drivers/intel/sgx"
sudo sh -c "cat /etc/modules | grep -Fxq isgx || echo isgx >> /etc/modules"
sudo /sbin/depmod
sudo /sbin/modprobe isgx

echo "SGX driver succesfully installed"

# ----------------------------------------------------------------------
echo "[ Building SGX SDK ]"
cd $script_path
git clone https://github.com/intel/linux-sgx.git
cd linux-sgx
git fetch --all --tags
git checkout tags/$sgx_sdk_version

# Prepare submodules and prebuilt binaries
make preparation

# Copy mitigation tools for current OS 
echo "[ Copying mitigation tools]"
sudo cp external/toolset/ubuntu$OS_REL/* /usr/local/bin

# Build the sdk with or without debug info
if [ $debug_info -eq 1 ];
then 
make sdk_install_pkg DEBUG=1 -j`nproc`
else
make sdk_install_pkg -j`nproc`
fi


echo "[ Installing SGX SDK system-wide ]"
cd linux/installer/bin/
sudo ./sgx_linux_x64_sdk_*.bin << EOF
no
/opt/intel
EOF
cd ../../../

# ----------------------------------------------------------------------
echo "[ Building SGX PSW ]"

# Build the PSW with or without debug info
if [ $debug_info -eq 1 ]
then 
make psw_install_pkg DEBUG=1 -j`nproc`
else
make psw_install_pkg -j`nproc`
fi

# Build local debian package repository
#make deb_local_repo

echo "[ Installing PSW/SDK system-wide ]"
cd linux/installer/bin/

if [ -e /opt/intel/sgxpsw/uninstall.sh ]
then
    sudo /opt/intel/sgxpsw/uninstall.sh
fi
sudo ./sgx_linux_x64_psw_*.bin

echo "SGX SDK succesfully installed!"
