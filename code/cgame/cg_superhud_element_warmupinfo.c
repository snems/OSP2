#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	int timePrev;
	superhudTextContext_t ctx;
} shudElementWarmupInfo_t;

void* CG_SHUDElementWarmupInfoCreate(const superhudConfig_t* config)
{
	shudElementWarmupInfo_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementWarmupInfoRoutine(void* context)
{
	shudElementWarmupInfo_t* element = (shudElementWarmupInfo_t*)context;
	char str[256];

	int sec = cg.warmup;

	CG_SHUDFill(&element->config);

	element->ctx.text = NULL;

	if (sec < 0)
	{
		element->ctx.text = cgs.gametype != GT_TOURNAMENT ? "^BWaiting for Players" : "^BWaiting for Opponent";
	}
	else if (sec > 0)
	{
		if (cg.showScores == 0)
		{
			sec = (sec - cg.time) / 1000;
			if (sec < 0)
			{
				cg.warmup = 0;
				sec = 0;
			}
			Com_sprintf(str, 256, "Starts in: %i", sec + 1);
			element->ctx.text = str;
		}
	}
	if (!element->ctx.text)
	{
		return;
	}
	CG_SHUDFill(&element->config);

	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementWarmupInfoDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

