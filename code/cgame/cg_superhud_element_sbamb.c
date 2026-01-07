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

void* CG_SHUDElementSBAmBCreate(const superhudConfig_t* config)
{
	int i;
	shudElementStatusbarAmmoBar* element;

	SHUD_ELEMENT_INIT(element, config);

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
	int wp;
	int ammo;
	qboolean ammoOverflow;

	if (!cg.snap)
	{
		return;
	}

	wp = cg.snap->ps.weapon;

	if (wp == WP_NONE || wp == WP_GAUNTLET) return;

	ammo = CG_SHUDGetAmmo(wp);
	ammoOverflow = ammo > element->maxammo[wp];

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

	CG_SHUDFill(&element->config);
	CG_SHUDBarPrint(&element->config, &element->ctx, ammo);
}

void CG_SHUDElementSBAmBDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

