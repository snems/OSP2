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

	CG_SHUDFill(&element->config);

	if (element->config.style.value == 1) // default
	{
		CG_ColorForHealth(element->ctx.color_top, NULL);
	}
	else if (element->config.style.value == 2)
	{
		Vector4Copy(element->config.color.value.rgba, element->ctx.color_top);
		if (!element->config.color2.isSet) // set same color if color2 isn't set
		{
			Vector4Copy(element->ctx.color_top, element->ctx.color2_top);
			element->ctx.color2_top[3] *= 3; // make more visible
		}
		else
		{
			Vector4Copy(element->config.color2.value.rgba, element->ctx.color2_top);
		}
	}
	CG_SHUDBarPrint(&element->config, &element->ctx, hp);
}

void CG_SHUDElementSBHBDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

