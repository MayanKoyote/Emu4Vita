TARGET = gpSP

APP_NAME         := gpSP
APP_NAME_EXT     := 
APP_DIR_NAME     := gpSP
CORE_FILE_NAME   := gpsp

SFO_TITLE_NAME   := gpSP
SFO_TITLE_ID     := GPSP00001

CORE_SOFTWARE    := Nintendo\ -\ Game\ Boy\ Advance

MAKEFILE_PATH      := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR       := $(dir $(MAKEFILE_PATH))

APP_DIR            := $(patsubst %/,%, $(MAKEFILE_DIR))
FRONTEND_DIR       := $(APP_DIR)/../../frontend
CORE_DIR           := $(APP_DIR)/../../cores/$(CORE_FILE_NAME)
CORE_MAKEFILE_DIR  := $(CORE_DIR)
CORE_MAKEFILE_NAME := Makefile

WANT_DISPLAY_ROTATE  :=
WANT_EXT_ARCHIVE_ROM := 1

CORE_ARGS := 

CORE_DEFINES := \
	-DGPSP_BUILD \
	-DGBA_BUILD

include $(FRONTEND_DIR)/Makefile.common
