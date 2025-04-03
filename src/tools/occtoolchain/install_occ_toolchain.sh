#!/bin/bash
# Installs the ppe42 toolchain that supports the OCC PPEs
# Installs the powerpc toolchain for the OCC 405
# To be run with root authority

set -x
set -e

## If you are on a cloud machine, not all of these package may be available to you via yum
## If so, You will need to manually install them using ibm-yum (bison, flex, texinfo)
yum install --assumeyes \
    gmp gmp-devel \
    libmpc-devel \
    mpfr mpfr-devel \
    patch \
    bison \
    flex \
    texinfo

BASEDIR=/tmp
WORKDIR=$BASEDIR/occ_toolchains
mkdir -p $WORKDIR
cd $WORKDIR

############################################
###### PPE cross compiler tool chain #######
############################################
rm -Rf *
git clone https://github.com/open-power/ppe42-binutils.git \
              --depth=1 -b binutils-2_24-ppe42 || exit -1

git clone https://github.com/open-power/ppe42-gcc.git \
              --depth=1 -b gcc-4_9_2-ppe42 || exit -1

cd $WORKDIR/ppe42-gcc
wget raw.githubusercontent.com/open-power/op-build/refs/heads/master/openpower/package/ppe42-gcc/0001-2016-02-19-Jakub-Jelinek-jakub-redhat.com.patch

wget raw.githubusercontent.com/open-power/op-build/refs/heads/master/openpower/package/ppe42-gcc/0001-reload-Change-to-type-of-x_spill_indirect_levels.patch

patch -p1 -i 0001-2016-02-19-Jakub-Jelinek-jakub-redhat.com.patch
patch -p1 -i 0001-reload-Change-to-type-of-x_spill_indirect_levels.patch

export TARGET=powerpc-eabi
export PREFIX=/usr/local/ppe42

mkdir -p $PREFIX

mkdir -p $WORKDIR/build-ppe-binutils
cd $WORKDIR/build-ppe-binutils

../ppe42-binutils/configure -v \
    --enable-shared \
    --enable-64-bit-bfd \
    --target=$TARGET \
    --prefix=$PREFIX && \
 make -j16 configure-host && \
 make -j16 MAKEINFO=true \
    CFLAGS+=-Wno-implicit-fallthrough \
    CFLAGS+=-Wno-error=cast-function-type \
    CFLAGS+=-Wno-error=stringop-truncation \
    CFLAGS+=-Wno-error=pointer-compare && \
 make MAKEINFO=true install

mkdir -p $WORKDIR/build-ppe-gcc
cd $WORKDIR/build-ppe-gcc

../ppe42-gcc/configure \
    --target=$TARGET \
    --prefix=$PREFIX \
    --without-headers \
    --with-newlib \
    --with-gnu-as \
    --with-gnu-ld \
    --disable-doc \
    --disable-pod2man && \
 make -j16 configure-host && \
 make -j16 MAKEINFO=true all-gcc && \
 make MAKEINFO=true install-gcc

#################################################
# powerpc cross compiler  toolchain for OCC 405 #
#################################################

cd $WORKDIR

git clone http://sourceware.org/git/binutils-gdb.git \
              --depth=1 -b binutils-2_36-branch || exit -1
git clone https://github.com/gcc-mirror/gcc.git \
              --depth=1 -b releases/gcc-11  || exit -1

export TARGET=powerpc-linux
export PREFIX=/usr/local/power

mkdir -p $PREFIX

mkdir -p $WORKDIR/build-ppc-binutils
cd $WORKDIR/build-ppc-binutils

../binutils-gdb/configure -v \
    --enable-shared \
    --enable-64-bit-bfd \
    --target=$TARGET \
    --prefix=$PREFIX && \
 make -j16 configure-host && \
 make -j16 MAKEINFO=true && \
 make MAKEINFO=true install

mkdir -p $WORKDIR/build-ppc-gcc
cd $WORKDIR/build-ppc-gcc

../gcc/configure \
    --target=$TARGET \
    --prefix=$PREFIX \
    --without-headers \
    --with-newlib \
    --with-gnu-as \
    --with-gnu-ld \
    --disable-doc \
    --disable-pod2man && \
 make -j16 configure-host && \
 make -j16 MAKEINFO=true all-gcc && \
 make MAKEINFO=true install-gcc

/usr/local/ppe42/bin/powerpc-eabi-gcc --version
/usr/local/power/bin/powerpc-linux-gcc --version

cd $BASEDIR

rm -Rf occ_toolchains*



