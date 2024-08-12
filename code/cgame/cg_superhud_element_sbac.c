#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementStatusbarHealthCount;

void* CG_SHUDElementSBACCreate(const superhudConfig_t* config)
{
	shudElementStatusbarHealthCount* element;

	SHUD_ELEMENT_INIT(element, config);

	//load defaults
	if (!element->config.color.isSet)
	{
		element->config.color.isSet = qtrue;
		element->config.color.value.type = SUPERHUD_COLOR_RGBA;
		Vector4Set(element->config.color.value.rgba, 1, 0.7, 0, 1);
	}

	if (!element->config.text.isSet)
	{
		element->config.text.isSet = qtrue;
		Q_strncpyz(element->config.text.value, "%i", sizeof(element->config.text.value));
	}

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.maxchars = 6;
	element->ctx.flags |= DS_FORCE_COLOR;

	return element;
}

void CG_SHUDElementSBACRoutine(void* context)
{
	shudElementStatusbarHealthCount* element = (shudElementStatusbarHealthCount*)context;
	int ap = cg.snap->ps.stats[STAT_ARMOR];

	if (ap <= 0) return;

	element->ctx.text = va(element->config.text.value, ap > 0 ? ap : 0);

	element->config.color.isSet = qtrue;
	element->config.color.value.type = SUPERHUD_COLOR_RGBA;
	CG_ColorForHealth(element->config.color.value.rgba, NULL);

	CG_SHUDFill(&element->config);
	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementSBACDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
