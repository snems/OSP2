#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudBarContext_t ctx;
} shudElementStatusbarArmorBar;

void* CG_SHUDElementSBABCreate(const superhudConfig_t* config)
{
	shudElementStatusbarArmorBar* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDBarMakeContext(&element->config, &element->ctx, 200);

	return element;
}

void CG_SHUDElementSBABRoutine(void* context)
{
	shudElementStatusbarArmorBar* element = (shudElementStatusbarArmorBar*)context;
	float ap = cg.snap->ps.stats[STAT_ARMOR];


	CG_SHUDFill(&element->config);
	CG_ColorForHealth(element->ctx.color_top, NULL);
	CG_SHUDBarPrint(&element->config, &element->ctx, ap);
}

void CG_SHUDElementSBABDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

