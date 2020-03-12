#! /bin/sh

# export

# Ubuntu example path to ARMGCC:
GCC_INSTALL_DIR=/home/$USER/gcc/gcc-arm-none-eabi-8-2019-q3-update

# MacOS example path to ARMGCC:
#GCC_INSTALL_DIR=/Users/$USER/gcc-arm-none-eabi-8-2019-q3-update/

export PATH=$GCC_INSTALL_DIR/bin:$PATH
export PATH=$GCC_INSTALL_DIR/arm-none-eabi/bin:$PATH

LD_LIBRARY_PATH=$GCC_INSTALL_DIR/bin:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=$GCC_INSTALL_DIR/arm-none-eabi/lib:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=$GCC_INSTALL_DIR/lib:$LD_LIBRARY_PATH
LD_LIBRARY_PATH=$GCC_INSTALL_DIR/include:$LD_LIBRARY_PATH

export LD_LIBRARY_PATH

CPATH=$GCC_INSTALL_DIR/arm-none-eabi/include
export CPATH

echo ""
echo "    -> Loaded arm gcc variables"
echo "    -> Altered: PATH, LD_LIBRARY_PATH, CPATH"
echo ""
