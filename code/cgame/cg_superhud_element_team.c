
#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"


typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctxPowerup;
	superhudTextContext_t ctxName;
	superhudTextContext_t ctxHealthArmor;
	superhudDrawContext_t ctxWeapon;
	superhudTextContext_t ctxLocation;
	int index;
} shudElementTeam_t;


static void* CG_SHUDElementTeamCreate(const superhudConfig_t* config, int line)
{
	shudTeamOverlay_t teamOverlay;

	superhudConfig_t lcfg;

	shudElementTeam_t* element;

	SHUD_ELEMENT_INIT(element, config);

	element->index = line;

	//force aligin settings
	element->config.textAlign.isSet = qtrue;
	element->config.textAlign.value = SUPERHUD_ALIGNH_LEFT;
	element->config.alignH.isSet = qtrue;
	element->config.alignH.value = SUPERHUD_ALIGNH_LEFT;
	element->config.alignV.isSet = qtrue;
	element->config.alignV.value = SUPERHUD_ALIGNV_CENTER;

	CG_SHUDElementCompileTeamOverlayConfig(config->fontsize.value[0], &teamOverlay);

	memcpy(&lcfg, &element->config, sizeof(element->config));

	// setup powerup
	lcfg.rect.value[0] = config->rect.value[0] + teamOverlay.powerupOffsetPix;
	lcfg.rect.value[2] = teamOverlay.powerupLenPix;
	CG_SHUDDrawMakeContext(&lcfg, &element->ctxPowerup);

	// setup name
	lcfg.rect.value[0] = config->rect.value[0] + teamOverlay.nameOffsetPix;
	lcfg.rect.value[2] = teamOverlay.nameLenPix;
	CG_SHUDTextMakeContext(&lcfg, &element->ctxName);
	element->ctxName.width = teamOverlay.nameLenPix;

	// setup health and armor
	lcfg.rect.value[0] = config->rect.value[0] + teamOverlay.healthAndArmorOffsetPix;
	lcfg.rect.value[2] = teamOverlay.healthAndArmorLenPix;
	CG_SHUDTextMakeContext(&lcfg, &element->ctxHealthArmor);
	element->ctxHealthArmor.width = teamOverlay.healthAndArmorLenPix;
	element->ctxHealthArmor.flags |= DS_FORCE_COLOR;

	// setup weapon
	lcfg.rect.value[0] = config->rect.value[0] + teamOverlay.weaponOffsetPix;
	lcfg.rect.value[2] = teamOverlay.weaponLenPix;
	CG_SHUDDrawMakeContext(&lcfg, &element->ctxWeapon);

	// setup location
	lcfg.rect.value[0] = config->rect.value[0] + teamOverlay.locationOffsetPix;
	lcfg.rect.value[2] = teamOverlay.locationLenPix;
	CG_SHUDTextMakeContext(&lcfg, &element->ctxLocation);
	element->ctxLocation.width = teamOverlay.locationLenPix;

	// setup width of element
	element->config.rect.value[2] = teamOverlay.overlayWidthPix;

	return element;
}

void* CG_SHUDElementTeam1Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 1);
}

void* CG_SHUDElementTeam2Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 2);
}

void* CG_SHUDElementTeam3Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 3);
}

void* CG_SHUDElementTeam4Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 4);
}

void* CG_SHUDElementTeam5Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 5);
}

void* CG_SHUDElementTeam6Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 6);
}

void* CG_SHUDElementTeam7Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 7);
}

void* CG_SHUDElementTeam8Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 8);
}

void* CG_SHUDElementTeam9Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 9);
}

void* CG_SHUDElementTeam10Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 10);
}

void* CG_SHUDElementTeam11Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 11);
}

void* CG_SHUDElementTeam12Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 12);
}

void* CG_SHUDElementTeam13Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 13);
}

void* CG_SHUDElementTeam14Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 14);
}

void* CG_SHUDElementTeam15Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 15);
}

void* CG_SHUDElementTeam16Create(const superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 16);
}

void CG_SHUDElementTeamRoutine(void* context)
{
	shudElementTeam_t* element = (shudElementTeam_t*)context;
	clientInfo_t* ci;
	int cnt = 0;
	int index;
	int ourTeam;

	if (CG_OSPIsGameTypeFreeze())
	{
		ourTeam = cgs.clientinfo[cg.clientNum].rt;
	}
	else
	{
		ourTeam = cg.snap->ps.persistant[PERS_TEAM];
	}

	for (index = 0; index < numSortedTeamPlayers; ++index)
	{
		ci = &cgs.clientinfo[sortedTeamPlayers[index]];
		if (ci->infoValid)
		{
			if (ci->team != ourTeam && ci->rt != ourTeam)
			{
				continue;
			}
			if (++cnt == element->index)
			{
				break;
			}
		}
	}

	if (cnt != element->index)
	{
		//no elements
		return;
	}
	CG_SHUDFill(&element->config);

	//get player
	ci = &cgs.clientinfo[sortedTeamPlayers[index]];

	// draw name
	element->ctxName.text = ci->name;
	CG_SHUDTextPrint(&element->config, &element->ctxName);

	// draw health and armor
	CG_GetColorForHealth(ci->health, ci->armor, element->ctxHealthArmor.color, NULL);
	element->ctxHealthArmor.text = va("%3i/%i", ci->health, ci->armor);

	CG_FontSelect(element->ctxHealthArmor.fontIndex);
	CG_OSPDrawString(element->ctxHealthArmor.coord.named.x,
	                 element->ctxHealthArmor.coord.named.y,
	                 element->ctxHealthArmor.text,
	                 element->ctxHealthArmor.color,
	                 element->ctxHealthArmor.coord.named.w,
	                 element->ctxHealthArmor.coord.named.h,
	                 SCREEN_WIDTH,
	                 element->ctxHealthArmor.flags,
	                 NULL);

	// draw weapon
	element->ctxWeapon.image = cg_weapons[ci->curWeapon].ammoIcon ?  cg_weapons[ci->curWeapon].ammoIcon : cgs.media.deferShader;
	CG_SHUDDrawStretchPicCtx(&element->config, &element->ctxWeapon);

	// draw powerup
	{
		int k = 0;
		gitem_t* gi;

		do
		{
			if (cgs.osp.gameTypeFreeze && ci->health <= 0)
			{
				element->ctxPowerup.image = cgs.media.noammoShader;
				CG_SHUDDrawStretchPicCtx(&element->config, &element->ctxPowerup);
				break;
			}
			else if (ci->powerups & (1 << k))
			{
				gi = BG_FindItemForPowerup(k);
				if (gi)
				{
					element->ctxPowerup.image = trap_R_RegisterShader(gi->icon);
					CG_SHUDDrawStretchPicCtx(&element->config, &element->ctxPowerup);
				}
			}
		}
		while (++k < 16);
	}

	// draw location
	{
		if (customLocationsEnabled != 0)
		{
			element->ctxLocation.text = CG_CustomLocationsGetName(ci->customLocation);
		}
		else
		{
			if (ch_ColorLocations.integer != 0 && cgs.gametype == GT_CTF)
			{
				element->ctxLocation.text = CG_GetCTFLocation(ci->location);
			}
			else
			{
				element->ctxLocation.text = CG_ConfigString(CS_LOCATIONS + ci->location);
			}
		}
		if (!element->ctxLocation.text || *element->ctxLocation.text == 0)
		{
			element->ctxLocation.text = "unknown";
		}

		CG_SHUDTextPrint(&element->config, &element->ctxLocation);
	}
}

void CG_SHUDElementTeamDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
