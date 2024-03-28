TARGET     := Emu4VitaArch
MAJOR_VER  := 1
MINOR_VER  := 8

PADDED_MAJOR_VER  := $(shell printf "%02d" $(MAJOR_VER))
PADDED_MINOR_VER  := $(shell printf "%02d" $(MINOR_VER))
APP_VER           := $(MAJOR_VER).$(PADDED_MINOR_VER)
SFO_APP_VER       := $(PADDED_MAJOR_VER).$(PADDED_MINOR_VER)
SFO_TITLE_ID      := EMUARCH01
SFO_TITLE_NAME    := Emu4Vita Arch

APP_NAME_STR      := Emu4Vita\ Arch
APP_VER_STR       := $(APP_VER)
APP_DIR_NAME      := Arch
BUILD_DATE        := $(shell date +"%Y/%m/%d")
BUILD_DATE2       := $(shell date +"%Y_%m_%d")

MAKEFILE_PATH     := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR      := $(dir $(MAKEFILE_PATH))
ROOT_DIR          := $(patsubst %/,%, $(MAKEFILE_DIR))

CORES_DIR         := $(ROOT_DIR)/cores
APPS_DIR          := $(ROOT_DIR)/../apps
PKG_DIR           := $(ROOT_DIR)/pkg
PKG_DIR2          := $(ROOT_DIR)/../frontend/pkgs/vita
IMPORTS_DIR       := $(ROOT_DIR)/imports
BUILD_DIR         := $(ROOT_DIR)/../build/arch
RELEASES_DIR      := $(ROOT_DIR)/../releases
NIGHTLIES_DIR     := $(ROOT_DIR)/../nightlies

SRC_DIR           := $(ROOT_DIR)/source
INCLUDE_DIR       := $(IMPORTS_DIR)/include
LIB_DIR           := $(IMPORTS_DIR)/lib

EBOOT_PATH        := $(PKG_DIR)/eboot.bin
SFO_PATH          := $(PKG_DIR)/sce_sys/param.sfo

ifeq ($(nightly), 1)
	TARGET_PATH   := $(NIGHTLIES_DIR)/Emu4VitaArch/$(TARGET)_nightly_$(BUILD_DATE2).vpk
else
	TARGET_PATH   := $(RELEASES_DIR)/Emu4VitaArch/$(TARGET)_v$(APP_VER).vpk
endif

MAKE_ARGS := arch_build=1 ARCH_DIR=$(ROOT_DIR)

INCDIRS := \
	-I$(SRC_DIR) \
	-I$(INCLUDE_DIR)

LIBDIRS := \
	-L$(LIB_DIR)

SRC_C := \
	browser.c \
	config.c \
	file.c \
	init.c \
	main.c \
	gui_lib.c \
	gui.c \
	utils.c \
	boot.c

LIBS := \
	-lvita2d \
	-lfreetype \
	-lpng \
	-ljpeg \
	-lz \
	-lm \
	-lc \
	-lSceDisplay_stub \
	-lSceGxm_stub \
	-lSceCtrl_stub \
	-lSceTouch_stub \
	-lScePower_stub \
	-lSceAudio_stub \
	-lSceRtc_stub \
	-lScePgf_stub \
	-lSceCommonDialog_stub \
	-lSceSysmodule_stub \
	-lSceAppUtil_stub \
	-lSceAppMgr_stub \
	-lSceShellSvc_stub

DEFINES := \
	-DAPP_NAME_STR=\"$(APP_NAME_STR)\" \
	-DAPP_NAME_EXT_STR=\"$(APP_NAME_EXT)\" \
	-DAPP_VER_STR=\"$(APP_VER_STR)\" \
	-DAPP_DIR_NAME=\"$(APP_DIR_NAME)\" \
	-DBUILD_DATE=\"$(BUILD_DATE)\"

WARNINGS := \
	-Wall \
	-Wno-format-truncation

CFLAGS    += -DVITA -D__vita__ -O3
ASFLAGS   := $(CFLAGS) $(WARNINGS) $(INCDIRS)

CFLAGS    += $(WARNINGS) $(INCDIRS) $(DEFINES)
CXXFLAGS  := $(CFLAGS) -fno-rtti -fno-exceptions
LDFLAGS   := -Wl,-q $(LIBDIRS)

OBJS := $(patsubst %.c,%.o, $(SRC_C))
OBJS := $(addprefix $(BUILD_DIR)/, $(OBJS))

PREFIX  := arm-vita-eabi
CC      := $(PREFIX)-gcc
CXX     := $(PREFIX)-g++
AS      := $(PREFIX)-as
AR      := $(PREFIX)-ar
OBJCOPY := $(PREFIX)-objcopy
STRIP   := $(PREFIX)-strip
NM      := $(PREFIX)-nm
LD      := $(CXX)

all: clean_files create_dirs build-cores build-libvita2d build-app

clean_files:
	@rm -rf $(CORES_DIR) $(TARGET_PATH) $(EBOOT_PATH) $(SFO_PATH) $(BUILD_DIR)/$(TARGET).velf $(BUILD_DIR)/$(TARGET).elf

create_dirs:
	@mkdir -p $(BUILD_DIR)

build-libvita2d: 
	cd $(ROOT_DIR) && make -f Makefile.libvita2d

build-app: $(EBOOT_PATH) $(SFO_PATH)
	@rm -rf $(TARGET_PATH)
	@mkdir -p $(dir $(TARGET_PATH))
	cd $(PKG_DIR2) && zip -r $(TARGET_PATH) ./*
	cd $(PKG_DIR) && zip -r $(TARGET_PATH) ./*
	cd $(CORES_DIR) && zip -r $(TARGET_PATH) ./*

$(SFO_PATH):
	@mkdir -p $(dir $@)
	vita-mksfoex -s APP_VER="$(SFO_APP_VER)" \
		-s TITLE_ID="$(SFO_TITLE_ID)" "$(SFO_TITLE_NAME)" \
		-d ATTRIBUTE2=12 $@

$(EBOOT_PATH): $(BUILD_DIR)/$(TARGET).velf
	@mkdir -p $(dir $@)
	vita-make-fself -c -a 0x2800000000000001 $< $@

$(BUILD_DIR)/$(TARGET).velf: $(BUILD_DIR)/$(TARGET).elf
	@mkdir -p $(dir $@)
	$(STRIP) -g $<
	vita-elf-create $< $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(dir $@)
	$(LD) -o $@ $^ $(LIBS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(ASFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(ASFLAGS)

clean: clean-app

clean-all: clean-cores clean-libvita2d clean-app

clean-libvita2d: 
	cd $(ROOT_DIR) && make -f Makefile.libvita2d clean

clean-app:
	rm -rf $(BUILD_DIR) $(TARGET_PATH) $(EBOOT_PATH) $(SFO_PATH)

build-cores:
	cd $(APPS_DIR)/fba_lite && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/fceumm && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/gambatte && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/genesis_plus_gx && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/gpsp && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_ngp && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_pce_fast && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_supergrafx && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_wswan && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/nestopia && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/pcsx_rearmed && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/picodrive && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2002 && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005 && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005_plus && make build-self $(MAKE_ARGS)
	cd $(APPS_DIR)/vba_next && make build-self $(MAKE_ARGS)
	
clean-cores:
	cd $(APPS_DIR)/fba_lite && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/fceumm && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/gambatte && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/genesis_plus_gx && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/gpsp && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_ngp && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_pce_fast && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_supergrafx && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_wswan && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/nestopia && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/pcsx_rearmed && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/picodrive && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2002 && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005 && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005_plus && make clean-self $(MAKE_ARGS)
	cd $(APPS_DIR)/vba_next && make clean-self $(MAKE_ARGS)
