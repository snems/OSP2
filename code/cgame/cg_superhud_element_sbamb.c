#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"


typedef struct
{
	superhudConfig_t config;
	superhudBarContext_t ctx;
	int maxammo[MAX_WEAPONS];
	int last_weapon;
	int red;
	int white;
} shudElementStatusbarAmmoBar;

void* CG_SHUDElementSBAmBCreate(superhudConfig_t* config)
{
	int i;
	shudElementStatusbarAmmoBar* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));


	for (i = 0; i < MAX_WEAPONS; ++i)
	{
		element->maxammo[i] = 100;
	}
	element->last_weapon = -1;

	return element;
}

void CG_SHUDElementSBAmBRoutine(void* context)
{
	shudElementStatusbarAmmoBar* element = (shudElementStatusbarAmmoBar*)context;
	int wp = cg.weaponSelect;
	int ammo = cg.predictedPlayerState.ammo[wp];
	qboolean ammoOverflow = ammo > element->maxammo[wp];

	if (wp == WP_NONE || wp == WP_GAUNTLET) return;

	if (wp != element->last_weapon || ammoOverflow)
	{
		//if weapon changed or got more ammo than we have before, reinitailize
		element->last_weapon = wp;
		if (ammoOverflow)
		{
			element->maxammo[wp] = ammo;
		}
		element->white = element->maxammo[wp] / 2;
		element->red = element->maxammo[wp] / 8;
		CG_SHUDBarMakeContext(&element->config, &element->ctx, element->maxammo[wp]);
	}

	element->ctx.color_top[3] = 1.0f;

	if (ammo > element->white)
	{
		VectorCopy(colorWhite, element->ctx.color_top);
	}
	else if (ammo > element->red)
	{
		VectorSet(element->ctx.color_top, 1.0f, 0.7f, 0.0f);
	}
	else if (cg.time & 512)
	{
		VectorCopy(colorRed, element->ctx.color_top);
	}
	else
	{
		element->ctx.color_top[3] = 0;
	}

	CG_SHUDBarPrint(&element->ctx, ammo);
}

void CG_SHUDElementSBAmBDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

