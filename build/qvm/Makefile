#
# GNU Make required
#

COMPILE_PLATFORM=$(shell uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
COMPILE_ARCH=$(shell uname -m | sed -e 's/i.86/x86/' | sed -e 's/^arm.*/arm/')

ifeq ($(COMPILE_PLATFORM),mingw32)
  ifeq ($(COMPILE_ARCH),i386)
    COMPILE_ARCH=x86
  endif
endif

#############################################################################
#
# If you require a different configuration from the defaults below, create a
# new file named "Makefile.local" in the same directory as this file and define
# your parameters there. This allows you to change configuration without
# causing problems with keeping up to date with the repository.
#
#############################################################################
-include Makefile.local

ifndef MOD
MOD=osp2
endif

ifeq ($(COMPILE_PLATFORM),cygwin)
  PLATFORM=mingw32
endif

ifndef PLATFORM
PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

ifeq ($(PLATFORM),mingw32)
  MINGW=1
endif
ifeq ($(PLATFORM),mingw64)
  MINGW=1
endif
ifeq ($(PLATFORM),windows)
  MINGW=1
endif

ifeq ($(COMPILE_ARCH),i86pc)
  COMPILE_ARCH=x86
endif

ifeq ($(COMPILE_ARCH),amd64)
  COMPILE_ARCH=x86_64
endif
ifeq ($(COMPILE_ARCH),x64)
  COMPILE_ARCH=x86_64
endif

ifndef ARCH
ARCH=$(COMPILE_ARCH)
endif
export ARCH

ifneq ($(PLATFORM),$(COMPILE_PLATFORM))
  CROSS_COMPILING=1
else
  CROSS_COMPILING=0

  ifneq ($(ARCH),$(COMPILE_ARCH))
    CROSS_COMPILING=1
  endif
endif
export CROSS_COMPILING


include config.mk

srcs = $(foreach file,$1,$(if $(patsubst %.asm,,$(file)),$2/$(file).asm,$(file)))
qa_asm = $(call srcs,$(QA_SRC),vm/game)
cg_asm = $(call srcs,$(CG_SRC),vm/cgame)
ui_asm = $(call srcs,$(UI_SRC),vm/ui)

all: dirs $(PK3)

dirs:
	mkdir -p vm/game vm/cgame vm/ui

.PHONY: all dirs

#$(PK3): vm/qagame.qvm vm/cgame.qvm vm/ui.qvm
$(PK3): vm/cgame.qvm
	$(7Z) $@ vm/*.qvm vm/*.jts ../../assets/*

cc = cd vm/$1 && $(Q3LCC) $2 -o $(notdir $@).tmp ../../$< && mv $(notdir $@).tmp $(notdir $@)
qa_cc = $(call cc,game,$(QA_CFLAGS))
cg_cc = $(call cc,cgame,$(CG_CFLAGS))
ui_cc = $(call cc,ui,$(UI_CFLAGS))

#vm/qagame.qvm: $(qa_asm)
#	$(Q3ASM) -o $@ $(qa_asm)

vm/cgame.qvm: $(cg_asm)
	$(Q3ASM) -o $@ $(cg_asm)

#vm/ui.qvm: $(ui_asm)
#	$(Q3ASM) -o $@ $(ui_asm)

vm/game/%.asm: $(QADIR)/%.c; $(qa_cc)
vm/cgame/%.asm: $(QADIR)/%.c; $(cg_cc)
vm/cgame/%.asm: $(CGDIR)/%.c; $(cg_cc)
vm/cgame/%.asm: $(COMDIR)/%.c; $(cg_cc)
#vm/ui/%.asm: $(QADIR)/%.c; $(ui_cc)
#vm/ui/%.asm: $(UIDIR)/%.c; $(ui_cc)

clean:
	$(RM) -rf *.pk3 vm/ ../cgame/*.asm ../game/*.asm ../ui/*.asm
