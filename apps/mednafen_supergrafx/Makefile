TARGET = Mednafen-PCE-SuperGrafx

APP_NAME         := Mednafen\ PCE\ SuperGrafx
APP_NAME_EXT     := 
APP_DIR_NAME     := MednafenPCESuperGrafx
CORE_FILE_NAME   := mednafen_supergrafx

SFO_TITLE_NAME   := Mednafen PCE SuperGrafx
SFO_TITLE_ID     := MPCESG001

CORE_SOFTWARE    := NEC\ -\ PC\ Engine\ SuperGrafx

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)
CORE_MAKEFILE_NAME := Makefile

WANT_DISPLAY_ROTATE  :=
WANT_EXT_ARCHIVE_ROM := 1

CORE_ARGS := \
	SYSTEM_ZLIB=1

CORE_DEFINES := \
	-DMEDNAFEN_SUPERGRAFX_BUILD \
	-DPCE_BUILD

include $(FRONTEND_DIR)/Makefile.common
