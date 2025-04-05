#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementHoldableItem_t;

void* CG_SHUDElementHoldableItemCreate(const superhudConfig_t* config)
{
	shudElementHoldableItem_t* element;

	SHUD_ELEMENT_INIT(element, config);
	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementHoldableItemRoutine(void* context)
{
	shudElementHoldableItem_t* element = (shudElementHoldableItem_t*)context;

	int hi = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if (hi)
	{
		element->ctx.image = cg_items[hi].icon;
	}
	else
	{
		element->ctx.image = 0;
	}

	CG_SHUDFill(&element->config);
	CG_SHUDDrawStretchPicCtx(&element->config, &element->ctx);
}


void CG_SHUDElementHoldableItemDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}