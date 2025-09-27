LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://git@github.com/cu-ecen-aeld/assignments-3-and-later-xuanlongbui.git;protocol=ssh;branch=master"
SRCREV = "<commit-hash>"
PV = "1.0+git${SRCPV}"
SRCREV = "7cec14a58241310a0bac56979e59eb3d76918115"

inherit module

do_install() {
    install -d ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0644 aesdchar.ko ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra
}

FILES:${PN} += "${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/aesdchar.ko"
