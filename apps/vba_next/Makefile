TARGET = VBA-Next

APP_NAME         := VBA\ Next
APP_NAME_EXT     := 
APP_DIR_NAME     := VBANext
CORE_FILE_NAME   := vba_next

SFO_TITLE_NAME   := VBA Next
SFO_TITLE_ID     := VBANEXT01

CORE_SOFTWARE    := Nintendo\ -\ Game\ Boy\ Advance

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
	-DVBA_NEXT_BUILD \
	-DGBA_BUILD

include $(FRONTEND_DIR)/Makefile.common
