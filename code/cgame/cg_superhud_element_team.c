
#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"


typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
	const shudTeamOverlay_t *teamOverlay;
	int index;
	float x, y, w, h;
} shudElementTeam_t;


static void* CG_SHUDElementTeamCreate(superhudConfig_t* config, int line)
{
	static shudTeamOverlay_t teamOverlay;
	static qboolean teamOverlayInitialized = qfalse;
	const char *err_msg = "";

	shudElementTeam_t* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);


	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	element->index = line;

	//force aligin settings
	element->config.textAlign.isSet = qtrue;
	element->config.textAlign.value = SUPERHUD_ALIGNH_LEFT;
	element->config.alignH.isSet = qtrue;
	element->config.alignH.value = SUPERHUD_ALIGNH_LEFT;
	element->config.alignV.isSet = qtrue;
	element->config.alignV.value = SUPERHUD_ALIGNV_CENTER;

	
	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	if (!teamOverlayInitialized)
	{
		if (!shudElementCompileTeamOverlayConfig(ch_Teamoverlay.string, config->fontsize.value[0], cg_MaxlocationWidth.integer, &teamOverlay, &err_msg))
		{
			CG_Printf("Could not parse team overlay config string \"%s\" with message: %s \n", ch_Teamoverlay.string, err_msg);
			shudElementCompileTeamOverlayConfig("p12n h/aw l", config->fontsize.value[0], cg_MaxlocationWidth.integer, &teamOverlay, &err_msg);
		}
		teamOverlayInitialized = qtrue;
	}

	element->teamOverlay = &teamOverlay;
	element->x = element->ctx.textX;
	element->y = element->ctx.textY;
	element->w = element->ctx.fontW;
	element->h = element->ctx.fontH;
	
	NYAN_MSG(element->teamOverlay->str);

	CG_AdjustFrom640(&element->x, &element->y, &element->w, &element->h);

	return element;
}

void* CG_SHUDElementTeam1Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 1);
}

void* CG_SHUDElementTeam2Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 2);
}

void* CG_SHUDElementTeam3Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 3);
}

void* CG_SHUDElementTeam4Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 4);
}

void* CG_SHUDElementTeam5Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 5);
}

void* CG_SHUDElementTeam6Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 6);
}

void* CG_SHUDElementTeam7Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 7);
}

void* CG_SHUDElementTeam8Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 8);
}

void* CG_SHUDElementTeam9Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 9);
}

void* CG_SHUDElementTeam10Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 10);
}

void* CG_SHUDElementTeam11Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 11);
}

void* CG_SHUDElementTeam12Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 12);
}

void* CG_SHUDElementTeam13Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 13);
}

void* CG_SHUDElementTeam14Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 14);
}

void* CG_SHUDElementTeam15Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 15);
}

void* CG_SHUDElementTeam16Create(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCreate(config, 16);
}

void CG_SHUDElementTeamRoutine(void* context)
{
	shudElementTeam_t* element = (shudElementTeam_t*)context;
	clientInfo_t* ci;
	int cnt = 0;
	int index;
	int persistantTeam;


	persistantTeam = cg.snap->ps.persistant[PERS_TEAM];

	for (index = 0; index < numSortedTeamPlayers; ++index)
	{
		ci = &cgs.clientinfo[sortedTeamPlayers[index]];
		if (ci->infoValid)
		{
			if (ci->team != persistantTeam && ci->rt != persistantTeam)
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

	CG_FontSelect(element->ctx.fontIndex);
	if (element->teamOverlay->isNameEnabled)
	{
		CG_OSPDrawString(element->ctx.textX + element->teamOverlay->nameOffsetPix,
		                 element->ctx.textY,
		                 ci->name,
		                 element->ctx.color,
		                 element->ctx.fontW,
		                 element->ctx.fontH,
		                 element->teamOverlay->nameLenChar,
		                 element->ctx.flags);
	}

	if (element->teamOverlay->isHealthEnabled)
	{
		char buf[16];
		vec4_t healthColor;
		CG_GetColorForHealth(ci->health, ci->armor, healthColor);
		Com_sprintf(buf, 16, "%3i", ci->health);
		CG_OSPDrawString(element->ctx.textX + element->teamOverlay->healthOffsetPix,
		                 element->ctx.textY,
		                 buf,
		                 healthColor,
		                 element->ctx.fontW,
		                 element->ctx.fontH,
		                 element->teamOverlay->healthLenChar,
		                 element->ctx.flags);
	}

	if (element->teamOverlay->isArmorEnabled)
	{
		char buf[16];
		Com_sprintf(buf, 16, "%3i", ci->armor);
		CG_OSPDrawString(element->ctx.textX + element->teamOverlay->armorOffsetPix,
		                 element->ctx.textY,
		                 buf,
		                 element->ctx.color,
		                 element->ctx.fontW,
		                 element->ctx.fontH,
		                 element->teamOverlay->armorLenChar,
		                 element->ctx.flags);
	}

	if (element->teamOverlay->isAmmoEnabled)
	{
		qhandle_t icon = cg_weapons[ci->curWeapon].ammoIcon ?  cg_weapons[ci->curWeapon].ammoIcon : cgs.media.deferShader;
		float offset = element->teamOverlay->ammoOffsetPix;
		CG_AdjustFrom640(&offset, NULL, NULL, NULL);

		trap_R_SetColor(element->ctx.color);
		trap_R_DrawStretchPic(element->x + offset, 
												  element->y, 
												  element->w, 
												  element->h, 
												  0, 0, 1, 1, 
												  icon);
		trap_R_SetColor(NULL);

	}
	if (element->teamOverlay->isPowerupEnabled)
	{
		int k = 0;
		gitem_t* gi;
		float offset = element->teamOverlay->powerupOffsetPix;
		ci->powerups |= 2;
		CG_AdjustFrom640(&offset, NULL, NULL, NULL);
		do
		{
			if (cgs.osp.gameTypeFreeze && ci->health <= 0)
			{
				trap_R_SetColor(element->ctx.color);
				trap_R_DrawStretchPic(element->x + offset, 
														  element->y, 
														  element->w, 
														  element->h, 
														  0, 0, 1, 1, 
														  cgs.media.noammoShader);
				trap_R_SetColor(NULL);
				break;
			}
			else if (ci->powerups & (1 << k))
			{
				gi = BG_FindItemForPowerup(k);
				if (gi)
				{
					trap_R_SetColor(element->ctx.color);
					trap_R_DrawStretchPic(element->x + offset, 
															  element->y, 
															  element->w, 
															  element->h, 
															  0, 0, 1, 1, 
															  trap_R_RegisterShader(gi->icon));
					trap_R_SetColor(NULL);
				}
			}
		}
		while (++k < 16);
	}
	if (cg_MaxlocationWidth.integer && element->teamOverlay->isLocationEnabled)
	{
		const char* location = NULL;
		if (customLocationsEnabled != 0)
		{
			location = CG_CustomLocationsGetName(ci->customLocation);
		}
		else
		{
			if (qfalse && ch_ColorLocations.integer != 0 && cgs.gametype == GT_CTF)
			{
				//location = osp_get_something(ci->location);
			}
			else
			{
				location = CG_ConfigString(CS_LOCATIONS + ci->location);
			}
		}
		if (!location || *location == 0)
		{
			location = "unknown";
		}
		CG_OSPDrawString(element->ctx.textX + element->teamOverlay->locationOffsetPix,
		                 element->ctx.textY,
		                 location,
		                 element->ctx.color,
		                 element->ctx.fontW,
		                 element->ctx.fontH,
		                 element->teamOverlay->locationLenChar,
		                 element->ctx.flags);
	}
}

void CG_SHUDElementTeamDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
