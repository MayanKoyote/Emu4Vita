APPS_DIR    := ./apps
ARCH_DIR    := ./arch

MAKE_ARGS   := 
ifeq ($(nightly), 1)
	MAKE_ARGS := nightly=1
endif

all: build-app build-arch

clean: clean-app clean-arch

build-arch:
	cd $(ARCH_DIR) && make $(MAKE_ARGS)

clean-arch:
	cd $(ARCH_DIR) && make clean $(MAKE_ARGS)

build-app:
	cd $(APPS_DIR)/fba_lite && make $(MAKE_ARGS)
	cd $(APPS_DIR)/fceumm && make $(MAKE_ARGS)
	cd $(APPS_DIR)/gambatte && make $(MAKE_ARGS)
	cd $(APPS_DIR)/genesis_plus_gx && make $(MAKE_ARGS)
	cd $(APPS_DIR)/gpsp && make $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_ngp && make $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_pce_fast && make $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_supergrafx && make $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_wswan && make $(MAKE_ARGS)
	cd $(APPS_DIR)/nestopia && make $(MAKE_ARGS)
	cd $(APPS_DIR)/pcsx_rearmed && make $(MAKE_ARGS)
	cd $(APPS_DIR)/picodrive && make $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2002 && make $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005 && make $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005_plus && make $(MAKE_ARGS)
	cd $(APPS_DIR)/vba_next && make $(MAKE_ARGS)

clean-app:
	cd $(APPS_DIR)/fba_lite && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/fceumm && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/gambatte && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/genesis_plus_gx && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/gpsp && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_ngp && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_pce_fast && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_supergrafx && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_wswan && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/nestopia && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/pcsx_rearmed && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/picodrive && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2002 && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005 && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005_plus && make clean $(MAKE_ARGS)
	cd $(APPS_DIR)/vba_next && make clean $(MAKE_ARGS)

build-deps:
	cd $(APPS_DIR)/fba_lite && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/fceumm && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/gambatte && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/genesis_plus_gx && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/gpsp && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_ngp && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_pce_fast && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_supergrafx && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_wswan && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/nestopia && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/pcsx_rearmed && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/picodrive && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2002 && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005 && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005_plus && make build-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/vba_next && make build-deps $(MAKE_ARGS)

clean-deps:
	cd $(APPS_DIR)/fba_lite && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/fceumm && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/gambatte && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/genesis_plus_gx && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/gpsp && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_ngp && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_pce_fast && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_supergrafx && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_wswan && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/nestopia && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/pcsx_rearmed && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/picodrive && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2002 && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005 && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005_plus && make clean-deps $(MAKE_ARGS)
	cd $(APPS_DIR)/vba_next && make clean-deps $(MAKE_ARGS)

clean-all: clean-arch
	cd $(APPS_DIR)/fba_lite && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/fceumm && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/gambatte && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/genesis_plus_gx && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/gpsp && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_ngp && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_pce_fast && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_supergrafx && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/mednafen_wswan && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/nestopia && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/pcsx_rearmed && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/picodrive && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2002 && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005 && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/snes9x2005_plus && make clean-all $(MAKE_ARGS)
	cd $(APPS_DIR)/vba_next && make clean-all $(MAKE_ARGS)
