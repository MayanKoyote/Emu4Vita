TARGET = PicoDrive

APP_NAME         := PicoDrive
APP_NAME_EXT     := 
APP_DIR_NAME     := PicoDrive
CORE_FILE_NAME   := picodrive

SFO_TITLE_NAME   := PicoDrive
SFO_TITLE_ID     := PRCODRIVE

CORE_SOFTWARE    := Sega\ -\ MS/MD/CD/32x

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)
CORE_MAKEFILE_NAME := Makefile.libretro

WANT_DISPLAY_ROTATE :=
WANT_ARCHIVE_ROM    := 1

CORE_MAKE_ARGS := \
	PLATFORM_ZLIB=0

CORE_DEFINES := \
	-DPICODRIVE_BUILD \
	-DSMS_BUILD \
	-DMD_BUILD \
	-DMDCD_BUILD \
	-DMD32X_BUILD

include $(FRONTEND_DIR)/Makefile.common
