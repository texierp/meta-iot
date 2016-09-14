DESCRIPTION = "Basic image"
LICENSE = "MIT"

# set password to iot
inherit extrausers
EXTRA_USERS_PARAMS = " \
    usermod -P iot root; \
    "

inherit core-image

# Package gesture
IMAGE_FEATURES += "package-management"

# SSH gesture
IMAGE_FEATURES += "ssh-server-dropbear"

# To Include splash at boot-up
IMAGE_FEATURES += "splash"

CORE_OS_INSTALL = " \
    	dbus \
    	sysfsutils \
    	neard \
    	kernel-devicetree \
    	kernel-image \
    	kernel-modules \
    	firmware-imx-brcm \
"

IMAGE_INSTALL += " \
	${CORE_OS_INSTALL} \
	openssh-sftp-server \
	packagegroup-distro-base \
	packagegroup-machine-base \
	packagegroup-iot-base \
	packagegroup-tools-wifi-hotspot \
	packagegroup-tools-io \
    	${CORE_IMAGE_EXTRA_INSTALL} \
"

PACKAGE_EXCLUDE = "packagegroup-base-extended"

# some default locales
IMAGE_LINGUAS ?= "fr-fr en-us"

export IMAGE_BASENAME = "base-image"

