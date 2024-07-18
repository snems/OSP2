#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t position;
} shudElementStatusbarAmmoCount;

void* CG_SHUDElementSBAmCCreate(superhudConfig_t* config)
{
	shudElementStatusbarAmmoCount* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

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

	CG_SHUDTextMakeContext(&element->config, &element->position);
	element->position.maxchars = 6;

	return element;
}

void CG_SHUDElementSBAmCRoutine(void* context)
{
	shudElementStatusbarAmmoCount* element = (shudElementStatusbarAmmoCount*)context;
	const char* s;
	int wp = cg.weaponSelect;
	int ammo = cg.predictedPlayerState.ammo[wp];

	if (wp == WP_NONE || wp == WP_GAUNTLET) return;

	s = va(element->config.text.value, ammo > 0 ? ammo : 0);

	CG_SHUDTextPrint(s, &element->position);
}

void CG_SHUDElementSBAmCDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
