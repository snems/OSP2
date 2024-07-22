#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementItemPickup_t;

void* CG_SHUDElementItemPickupCreate(superhudConfig_t* config)
{
	shudElementItemPickup_t* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

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


	if (CG_SHUDGetFadeColor(element->ctx.color_origin, element->ctx.color, &element->config, cg.itemPickupTime))
	{
		int         mins, seconds, tens;
		int         msec;
		msec = cg.itemPickupTime - cgs.levelStartTime;

		seconds = msec / 1000;
		mins = seconds / 60;
		seconds -= mins * 60;
		tens = seconds / 10;
		seconds -= tens * 10;
		CG_SHUDTextPrint(va("%i:%i%i", mins, tens, seconds), &element->ctx);
		CG_SHUDTextPrint(va("%i:%i%i %s", mins, tens, seconds, bg_itemlist[cg.itemPickup].pickup_name), &element->ctx);
	}
}

void CG_SHUDElementItemPickupDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
