TARGET = Snes9x2005

APP_NAME         := Snes9x\ 2005
APP_NAME_EXT     := 
APP_DIR_NAME     := Snes9x2005
CORE_FILE_NAME   := snes9x2005

SFO_TITLE_NAME   := Snes9x 2005
SFO_TITLE_ID     := SNEX9X005

CORE_SOFTWARE    := Nintendo\ -\ SNES\ /\ SFC

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)
CORE_MAKEFILE_NAME := Makefile

WANT_DISPLAY_ROTATE :=
WANT_ARCHIVE_ROM    := 1

CORE_MAKE_ARGS := 

CORE_DEFINES := \
	-DSNES9X2005_BUILD \
	-DSFC_BUILD

include $(FRONTEND_DIR)/Makefile.common
