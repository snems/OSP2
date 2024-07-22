#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementPlayerSpeed_t;

void* CG_SHUDElementPlayerSpeedCreate(superhudConfig_t* config)
{
	shudElementPlayerSpeed_t* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.maxchars = 9;

	return element;
}

void CG_SHUDElementPlayerSpeedRoutine(void* context)
{
	shudElementPlayerSpeed_t* element = (shudElementPlayerSpeed_t*)context;

	CG_SHUDFill(&element->config);
	CG_SHUDTextPrint(va("%dups", (int)cg.xyspeed), &element->ctx);
}

void CG_SHUDElementPlayerSpeedDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
