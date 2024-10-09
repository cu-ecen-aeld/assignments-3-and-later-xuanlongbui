#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=/home/long/Downloads/gcc/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

#     # TODO: Add your kernel build steps here
#     echo a| sudo -S apt-get install git fakeroot build-essential ncurses-dev xz-utils libssl-dev bc flex libelf-dev bison
#     make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
#     make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j12
#     echo a| sudo -S cp $OUTDIR/linux-stable/arch/$ARCH/boot/Image $OUTDIR/
fi

# echo "Adding the Image in outdir"
# echo "Creating the staging directory for the root filesystem"
# cd "$OUTDIR"
# if [ -d "${OUTDIR}/rootfs" ]
# then
# 	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
#     echo a| sudo -S rm  -rf ${OUTDIR}/rootfs
# fi

# # TODO: Create necessary base directories
# ROOTFS=$OUTDIR/rootfs
# mkdir -p $ROOTFS
# cd $ROOTFS
# mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var 
# mkdir -p usr/bin usr/lib usr/sbin
# mkdir -p var/log
 
# cd "$OUTDIR"
# if [ ! -d "${OUTDIR}/busybox" ]
# then
# git clone git://busybox.net/busybox.git
#     cd busybox
#     git checkout ${BUSYBOX_VERSION}
#     # TODO:  Configure busybox
#     make distclean
#     # make defconfig
#     make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig
# else
#     cd busybox
# fi

# # TODO: Make and install busybox
# make CONFIG_PREFIX=$ROOTFS ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE install
# cd $ROOTFS
# echo "Library dependencies"
# ${CROSS_COMPILE}readelf -a ${ROOTFS}/bin/busybox | grep "program interpreter"
# ${CROSS_COMPILE}readelf -a ${ROOTFS}/bin/busybox | grep "Shared library"

# # TODO: Add library dependencies to rootfs
# cp /home/long/Downloads/gcc/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib/ld-linux-aarch64.so.1 $ROOTFS/lib
# cp /home/long/Downloads/gcc/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libm.so.6 $ROOTFS/lib64
# cp /home/long/Downloads/gcc/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libresolv.so.2 $ROOTFS/lib64
# cp /home/long/Downloads/gcc/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libc.so.6 $ROOTFS/lib64

# # TODO: Make device nodes
# echo a| sudo -S mknod -m 666 dev/null c 1 3
# echo a| sudo -S mknod -m 666 dev/tty c 5 1

# # TODO: Clean and build the writer utility
# cd $FINDER_APP_DIR
# make clean
# make CROSS_COMPILE=$CROSS_COMPILE"gcc"
# cp ./writer $ROOTFS/home
# # TODO: Copy the finder related scripts and executables to the /home directory
# # on the target rootfs
# cp ./finder.sh $ROOTFS/home
# cp ./writer.sh $ROOTFS/home
# cp ./finder-test.sh $ROOTFS/home
# cp ./autorun-qemu.sh $ROOTFS/home

# mkdir -p $ROOTFS/home/conf
# cp ./conf/assignment.txt $ROOTFS/home/conf
# cp ./conf/username.txt $ROOTFS/home/conf

# # TODO: Chown the root directory
# echo a| sudo -S chown root:root $ROOTFS
# # TODO: Create initramfs.cpio.gz
# cd $ROOTFS
# find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
# cd ..
# gzip -f initramfs.cpio
