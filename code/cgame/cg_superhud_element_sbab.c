#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudBarContext_t ctx;
} shudElementStatusbarArmorBar;

void* CG_SHUDElementSBABCreate(superhudConfig_t* config)
{
	shudElementStatusbarArmorBar* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	CG_SHUDBarMakeContext(&element->config, &element->ctx, 200);

	return element;
}

void CG_SHUDElementSBABRoutine(void* context)
{
	shudElementStatusbarArmorBar* element = (shudElementStatusbarArmorBar*)context;
	float ap = cg.snap->ps.stats[STAT_ARMOR];

	element->ctx.color_top[3] = 1.0f;

	if (ap > 100)
	{
		VectorCopy(colorWhite, element->ctx.color_top);
	}
	else
	{
		VectorSet(element->ctx.color_top, 1.0f, 0.7f, 0.0f);
	}

	CG_SHUDFill(&element->config);
	CG_SHUDBarPrint(&element->ctx, ap);
}

void CG_SHUDElementSBABDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

