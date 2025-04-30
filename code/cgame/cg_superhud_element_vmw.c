#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementVMW_t;

void* CG_SHUDElementVMWCreate(const superhudConfig_t* config)
{
	shudElementVMW_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementVMWRoutine(void* context)
{
	shudElementVMW_t* element = (shudElementVMW_t*)context;
	int time;

	if (cgs.voteTime == 0) return;

	if (cgs.voteModified)
	{
		cgs.voteModified = 0;
		if (!cg_noVoteBeep.integer)
		{
			trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
		}
	}

	time = (30000 - (cg.time - cgs.voteTime)) / 1000;

	if (time < 0)
	{
		time = 0;
	}
	element->ctx.text = va("VOTE(%i):%s yes(F1):%i no(F2):%i", time, &cgs.voteString, cgs.voteYes, cgs.voteNo);

	CG_SHUDFill(&element->config);
	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementVMWDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

