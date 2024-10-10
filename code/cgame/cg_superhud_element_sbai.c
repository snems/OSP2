#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarArmorIcon;

void* CG_SHUDElementSBAICreate(const superhudConfig_t* config)
{
	shudElementStatusbarArmorIcon* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementSBAIRoutine(void* context)
{

	if (cg.snap->ps.stats[STAT_ARMOR])
	{
		unsigned int index = cg.snap->ps.stats[STAT_ARMOR_TYPE];
		shudElementStatusbarArmorIcon* element = (shudElementStatusbarArmorIcon*)context;
		if (index > 2) index = 0;
		element->ctx.image = cgs.media.armorIcon[index];
		CG_SHUDFill(&element->config);
		CG_SHUDDrawStretchPicCtx(&element->config, &element->ctx);
	}
}

void CG_SHUDElementSBAIDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
