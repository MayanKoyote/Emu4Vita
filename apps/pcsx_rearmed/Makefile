TARGET = PCSX-ReARMed

APP_NAME         := PCSX\ ReARMed
APP_NAME_EXT     := 
APP_DIR_NAME     := PCSXReARMed
CORE_FILE_NAME   := pcsx_rearmed

SFO_TITLE_NAME   := PCSX ReARMed
SFO_TITLE_ID     := EMUPCSX01

CORE_SOFTWARE    := Sony\ -\ PlayStation

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)
CORE_MAKEFILE_NAME := Makefile.libretro

WANT_DISPLAY_ROTATE  :=
WANT_EXT_ARCHIVE_ROM :=

CORE_ARGS := \
	DYNAREC=ari64

CORE_DEFINES := \
	-DPCSX_REARMED_BUILD \
	-DPS_BUILD
	
include $(FRONTEND_DIR)/Makefile.common
