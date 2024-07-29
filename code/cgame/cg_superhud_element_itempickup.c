#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementItemPickup_t;

void* CG_SHUDElementItemPickupCreate(const superhudConfig_t* config)
{
	shudElementItemPickup_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	if (!element->config.time.isSet)
	{
		element->config.time.isSet = qtrue;
		element->config.time.value = 1000;
	}

	return element;
}

void CG_SHUDElementItemPickupRoutine(void* context)
{
	shudElementItemPickup_t* element = (shudElementItemPickup_t*)context;
	qboolean visible;

	visible = CG_SHUDGetFadeColor(element->ctx.color_origin, element->ctx.color, &element->config, cg.itemPickupTime);

	if (visible)
	{
		int         mins, seconds, tens;
		int         msec;
		msec = cg.itemPickupTime - cgs.levelStartTime;

		seconds = msec / 1000;
		mins = seconds / 60;
		seconds -= mins * 60;
		tens = seconds / 10;
		seconds -= tens * 10;
		CG_SHUDFill(&element->config);
		if (bg_itemlist[cg.itemPickup].pickup_name)
		{
			element->ctx.text = va("%i:%i%i %s", mins, tens, seconds, bg_itemlist[cg.itemPickup].pickup_name);
			CG_SHUDTextPrint(&element->config, &element->ctx);
		}
	}
}

void CG_SHUDElementItemPickupDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
