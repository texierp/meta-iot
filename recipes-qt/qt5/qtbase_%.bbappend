
COMPATIBLE_MACHINE = "(imx7s-warp|chip)"

PACKAGECONFIG_append = " libinput linuxfb accessibility sql-sqlite"
PACKAGECONFIG_remove = " icu x11"
