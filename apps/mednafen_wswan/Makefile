TARGET = Mednafen-Wswan

APP_NAME         := Mednafen\ WonderSwan
APP_NAME_EXT     := 
APP_DIR_NAME     := MednafenWswan
CORE_FILE_NAME   := mednafen_wswan

SFO_TITLE_NAME   := Mednafen Wswan
SFO_TITLE_ID     := WSWAN0001

CORE_SOFTWARE    := Bandai\ -\ WonderSwan\ /\ Color

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)
CORE_MAKEFILE_NAME := Makefile

WANT_DISPLAY_ROTATE := 1
WANT_ARCHIVE_ROM    := 1

CORE_MAKE_ARGS := 

CORE_DEFINES := \
	-DMEDNAFEN_WSWAN_BUILD \
	-DWSC_BUILD

include $(FRONTEND_DIR)/Makefile.common
