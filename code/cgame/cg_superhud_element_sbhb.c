#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudBarContext_t ctx;
} shudElementStatusbarHealthBar;

void* CG_SHUDElementSBHBCreate(const superhudConfig_t* config)
{
	shudElementStatusbarHealthBar* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDBarMakeContext(&element->config, &element->ctx, 200);

	return element;
}

void CG_SHUDElementSBHBRoutine(void* context)
{
	shudElementStatusbarHealthBar* element = (shudElementStatusbarHealthBar*)context;
	float hp = cg.snap->ps.stats[STAT_HEALTH];

	element->ctx.color_top[3] = 1.0f;

	if (hp > 100)
	{
		VectorCopy(colorWhite, element->ctx.color_top);
	}
	else if (hp > 30)
	{
		VectorSet(element->ctx.color_top, 1.0f, 0.7f, 0.0f);
	}
	else if (cg.time & 512)
	{
		VectorCopy(colorRed, element->ctx.color_top);
	}
	else
	{
		element->ctx.color_top[3] = 0;
	}

	CG_SHUDFill(&element->config);
	CG_SHUDBarPrint(&element->config, &element->ctx, hp);
}

void CG_SHUDElementSBHBDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

