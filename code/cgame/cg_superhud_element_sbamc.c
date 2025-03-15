#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementStatusbarAmmoCount;

void* CG_SHUDElementSBAmCCreate(const superhudConfig_t* config)
{
	shudElementStatusbarAmmoCount* element;

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

	return element;
}

void CG_SHUDElementSBAmCRoutine(void* context)
{
	shudElementStatusbarAmmoCount* element = (shudElementStatusbarAmmoCount*)context;
	int wp;
	int ammo;

	if (!cg.snap)
	{
		return;
	}

	wp = cg.snap->ps.weapon;

	if (wp == WP_NONE || wp == WP_GAUNTLET) return;

	ammo = CG_SHUDGetAmmo(wp);

	element->ctx.text = va(element->config.text.value, ammo > 0 ? ammo : 0);

	CG_SHUDFill(&element->config);
	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementSBAmCDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
