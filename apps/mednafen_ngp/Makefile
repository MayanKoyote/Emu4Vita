TARGET = Mednafen-Ngp

APP_NAME         := Mednafen\ NeoPop
APP_NAME_EXT     := 
APP_DIR_NAME     := MednafenNgp
CORE_FILE_NAME   := mednafen_ngp

SFO_TITLE_NAME   := Mednafen NeoPop
SFO_TITLE_ID     := NGP000001

CORE_SOFTWARE    := SNK\ -\ Neo\ Geo\ Pocket\ /\ Color

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
	-DMEDNAFEN_NGP_BUILD \
	-DNGP_BUILD

include $(FRONTEND_DIR)/Makefile.common
