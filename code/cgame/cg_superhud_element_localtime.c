#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	int timePrev;
	char s[MAX_QPATH];
	superhudTextContext_t ctx;
} shudElementLocalTime_t;

void* CG_SHUDElementLocalTimeCreate(const superhudConfig_t* config)
{
	shudElementLocalTime_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.text = &element->s[0];

	return element;
}

void CG_SHUDElementLocalTimeRoutine(void* context)
{
	shudElementLocalTime_t* element = (shudElementLocalTime_t*)context;
	qtime_t qtime;

	if (cg.time - element->timePrev > 1000)
	{
		element->timePrev = cg.time;
		trap_RealTime(&qtime);
		Com_sprintf(element->s, MAX_QPATH, "%02d:%02d", qtime.tm_hour, qtime.tm_min);
	}
	CG_SHUDFill(&element->config);

	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementLocalTimeDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

