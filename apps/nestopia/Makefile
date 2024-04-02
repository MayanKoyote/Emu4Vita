TARGET = Nestopia

APP_NAME         := Nestopia
APP_NAME_EXT     := 
APP_DIR_NAME     := Nestopia
CORE_FILE_NAME   := nestopia

SFO_TITLE_NAME   := Nestopia
SFO_TITLE_ID     := NESTOPIA1

CORE_SOFTWARE    := Nintendo\ -\ NES\ /\ Famicom

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)/libretro
CORE_MAKEFILE_NAME := Makefile

WANT_DISPLAY_ROTATE :=
WANT_ARCHIVE_ROM    := 1

CORE_MAKE_ARGS := 

CORE_DEFINES := \
	-DNESTOPIA_BUILD \
	-DFC_BUILD

include $(FRONTEND_DIR)/Makefile.common
