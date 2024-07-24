#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarItemPickupIcon;

void* CG_SHUDElementItemPickupIconCreate(superhudConfig_t* config)
{
	shudElementStatusbarItemPickupIcon* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	if (!element->config.time.isSet)
	{
		element->config.time.isSet = qtrue;
		element->config.time.value = 1000;
	}

	return element;
}

void CG_SHUDElementItemPickupIconRoutine(void* context)
{
	shudElementStatusbarItemPickupIcon* element = (shudElementStatusbarItemPickupIcon*)context;

	float* fade;

	if (CG_SHUDGetFadeColor(element->ctx.color_origin, element->ctx.color, &element->config, cg.itemPickupTime))
	{
		element->ctx.image = cg_items[cg.itemPickup].icon;
		CG_SHUDFill(&element->config);
		CG_RegisterItemVisuals(cg.itemPickup);
	  CG_SHUDDrawStretchPicCtx(&element->ctx);
	}
}

void CG_SHUDElementItemPickupIconDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
