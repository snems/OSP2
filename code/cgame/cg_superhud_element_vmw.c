#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudTextContext_t ctx;
} shudElementVMW_t;

void* CG_SHUDElementVMWCreate(superhudConfig_t* config)
{
  shudElementVMW_t *tn;

  tn = Z_Malloc(sizeof(*tn));
  OSP_MEMORY_CHECK(tn);

  memset(tn,0,sizeof(*tn));

  memcpy(&tn->config, config, sizeof(tn->config));

  CG_SHUDTextMakeContext(&tn->config, &tn->ctx);
  tn->ctx.maxchars = MAX_QPATH;

  return tn;
}

void CG_SHUDElementVMWRoutine(void *context)
{
  shudElementVMW_t *vmw = (shudElementVMW_t *)context;
	char *s;
	int time;

	if (cgs.voteTime == 0) return;

	if (cgs.voteModified)
	{
		cgs.voteModified = 0;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	time = (30000 - (cg.time - cgs.voteTime)) / 1000;

	if (time < 0)
	{
		time = 0;
	}
	s = va("VOTE(%i):%s yes(F1):%i no(F2):%i", time, &cgs.voteString, cgs.voteYes, cgs.voteNo);

  CG_SHUDTextPrint(s, &vmw->ctx);
}

void CG_SHUDElementVMWDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}

