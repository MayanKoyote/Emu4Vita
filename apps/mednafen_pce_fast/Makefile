TARGET = Mednafen-PCE-Fast

APP_NAME         := Mednafen\ PCE\ Fast
APP_NAME_EXT     := 
APP_DIR_NAME     := MednafenPCEFast
CORE_FILE_NAME   := mednafen_pce_fast

SFO_TITLE_NAME   := Mednafen PCE Fast
SFO_TITLE_ID     := MPCEFAST1

CORE_SOFTWARE    := NEC\ -\ PC\ Engine\ /\ CD

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)
CORE_MAKEFILE_NAME := Makefile

WANT_DISPLAY_ROTATE :=
WANT_ARCHIVE_ROM    := 1

CORE_MAKE_ARGS := \
	SYSTEM_ZLIB=1

CORE_DEFINES := \
	-DMEDNAFEN_PCE_FAST_BUILD \
	-DPCE_BUILD

include $(FRONTEND_DIR)/Makefile.common
