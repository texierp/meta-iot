SUMMARY = "Qt Heart Rate Server"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

SRC_URI = " \
	file://heart-rate-server.pro \
	file://main.cpp \
"

S = "${WORKDIR}"

PR = "r0"

DEPENDS = " qtbase qtconnectivity"

inherit qmake5
