TARGET = FCEUmm

APP_NAME         := FCEUmm
APP_NAME_EXT     := 
APP_DIR_NAME     := FCEUmm
CORE_FILE_NAME   := fceumm

SFO_TITLE_NAME   := FCEUmm
SFO_TITLE_ID     := FCEUMM001

CORE_SOFTWARE    := Nintendo\ -\ NES\ /\ Famicom

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)
CORE_MAKEFILE_NAME := Makefile.libretro

WANT_DISPLAY_ROTATE :=
WANT_ARCHIVE_ROM    := 1

CORE_MAKE_ARGS := 

CORE_DEFINES := \
	-DFCEUMM_BUILD \
	-DFC_BUILD

include $(FRONTEND_DIR)/Makefile.common
