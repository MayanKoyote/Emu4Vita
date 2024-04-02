TARGET = Gambatte

APP_NAME         := Gambatte
APP_NAME_EXT     := 
APP_DIR_NAME     := Gambatte
CORE_FILE_NAME   := gambatte

SFO_TITLE_NAME   := Gambatte
SFO_TITLE_ID     := GAMEBATTE

CORE_SOFTWARE    := Nintendo\ -\ Game\ Boy\ /\ Color

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
	-DGAMEBATTE_BUILD \
	-DGBC_BUILD

include $(FRONTEND_DIR)/Makefile.common
