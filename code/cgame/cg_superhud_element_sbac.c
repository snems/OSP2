#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t position;
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

	CG_SHUDFill(&element->config);
	CG_SHUDTextMakeContext(&element->config, &element->position);
	element->position.maxchars = 6;

	return element;
}

void CG_SHUDElementSBACRoutine(void* context)
{
	shudElementStatusbarHealthCount* element = (shudElementStatusbarHealthCount*)context;
	const char* s;
	int ap = cg.snap->ps.stats[STAT_ARMOR];

	s = va(element->config.text.value, ap > 0 ? ap : 0);

	CG_SHUDTextPrint(s, &element->position);
}

void CG_SHUDElementSBACDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
