TARGET = FBA-Lite

APP_NAME         := FBA\ Lite
APP_NAME_EXT     := 0.2.97.44
APP_DIR_NAME     := FBA-Lite
CORE_FILE_NAME   := fba_lite

SFO_TITLE_NAME   := FBA Lite
SFO_TITLE_ID     := FBALITE01

CORE_SOFTWARE    := Arcade\ \(CPS1\ 2\ 3,\ NEO\ GEO,\ PGM\)

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)
CORE_MAKEFILE_NAME := makefile.libretro

WANT_DISPLAY_ROTATE := 1
WANT_ARCHIVE_ROM    :=

CORE_MAKE_ARGS := 

CORE_DEFINES := \
	-DFBA_LITE_BUILD \
	-DARC_BUILD \
	-DFBA_BUILD

include $(FRONTEND_DIR)/Makefile.common
