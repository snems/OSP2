PK3 = zz-osp-pak8.pk3

export PATH := $(realpath ./tools):$(PATH) 

ifeq ($(PLATFORM),linux)

Q3ASM_BIN = q3asm
Q3LCC_BIN = q3lcc
7Z_BIN = 7z

endif #Linux

ifdef MINGW

Q3ASM_BIN = q3asm.exe
Q3LCC_BIN = q3lcc.exe
7Z_BIN = 7za.exe

endif #MINGW

Q3ASM = $(Q3ASM_BIN) -r -m -v
Q3LCC = $(Q3LCC_BIN) -DQ3_VM -S -Wf-g -I$(QADIR)
7Z = $(7Z_BIN) u -tzip -mx=9 -mpass=8 -mfb=255 --

basedir = ../../code

QADIR = $(basedir)/game
CGDIR = $(basedir)/cgame
UIDIR = $(basedir)/q3_ui
COMDIR = $(basedir)/qcommon


QA_CFLAGS = -DQAGAME
CG_CFLAGS = -DCGAME -I$(CGDIR)
UI_CFLAGS = -DQ3UI -I$(UIDIR)

include srcs.mk
