#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementPlayerSpeed_t;

void* CG_SHUDElementPlayerSpeedCreate(const superhudConfig_t* config)
{
	shudElementPlayerSpeed_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.maxchars = 9;

	return element;
}

void CG_SHUDElementPlayerSpeedRoutine(void* context)
{
	shudElementPlayerSpeed_t* element = (shudElementPlayerSpeed_t*)context;

	CG_SHUDFill(&element->config);
	element->ctx.text = va("%dups", (int)cg.xyspeed);
	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementPlayerSpeedDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
