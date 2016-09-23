SUMMARY = "Qt BLE Server : Open Silicium n°20 demo"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = " \
	file://ServerBluetooth.pro \
	file://main.cpp \
	file://init-ble-server.init \
"

S = "${WORKDIR}"

PR = "r0"

DEPENDS = " qtbase qtconnectivity"

inherit qmake5

do_install_append() {
		
        install -d ${D}${sysconfdir}/init.d/
        
        install -m 0755 ${WORKDIR}/init-ble-server.init ${D}${sysconfdir}/init.d/init-ble-server
}


inherit update-rc.d

INITSCRIPT_NAME = "init-ble-server"
INITSCRIPT_PARAMS = "start 99 5 2 . stop 19 0 1 6 ."
