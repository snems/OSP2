#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarAmmoIcon;

void* CG_SHUDElementSBAmICreate(superhudConfig_t* config)
{
	shudElementStatusbarAmmoIcon* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	if (config->image.isSet)
	{
		element->ctx.image = trap_R_RegisterShader(element->config.image.value);
	}

	return element;
}

void CG_SHUDElementSBAmIRoutine(void* context)
{
	shudElementStatusbarAmmoIcon* element = (shudElementStatusbarAmmoIcon*)context;
	int wp = cg.weaponSelect;
	qhandle_t icon = cg_weapons[wp].ammoIcon;

	if (wp == WP_NONE) return;

	if (icon)
	{
		trap_R_SetColor(element->ctx.color);
		trap_R_DrawStretchPic(element->ctx.x, element->ctx.y, element->ctx.w + 1, element->ctx.h, 0, 0, 1, 1, icon);
		trap_R_SetColor(NULL);
	}

}

void CG_SHUDElementSBAmIDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
