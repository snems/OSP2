#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctxAttacker;
	superhudDrawContext_t ctxMod;
	superhudTextContext_t ctxTarget;
	superhudGlobalContext_t* gctx;
	int index;
} shudElementObituaries_t;

shudElementObituaries_t cg_obituaries;

static qhandle_t CG_SHUDObituaryGetModIcon(int mod, qboolean unfrozen);
static void CG_SHUDObituarySetTeamColor(vec4_t color, int team);

static void* CG_SHUDElementObituariesCreate(const superhudConfig_t* config, int line)
{
	shudElementObituaries_t* element;
	superhudConfig_t lcfg;
	SHUD_ELEMENT_INIT(element, config);
	element->gctx = CG_SHUDGetContext();
	element->index = line;

	memcpy(&lcfg, config, sizeof(superhudConfig_t));

	if (!lcfg.alignV.isSet)
	{
		lcfg.alignV.value = SUPERHUD_ALIGNV_CENTER;
		lcfg.alignV.isSet = qtrue;
	}

	CG_SHUDTextMakeContext(&lcfg, &element->ctxAttacker);
	CG_SHUDDrawMakeContext(&lcfg, &element->ctxMod);
	CG_SHUDTextMakeContext(&lcfg, &element->ctxTarget);

	return element;
}

void* CG_SHUDElementObituaries1Create(const superhudConfig_t* config)
{
	return CG_SHUDElementObituariesCreate(config, 1);
}

void* CG_SHUDElementObituaries2Create(const superhudConfig_t* config)
{
	return CG_SHUDElementObituariesCreate(config, 2);
}

void* CG_SHUDElementObituaries3Create(const superhudConfig_t* config)
{
	return CG_SHUDElementObituariesCreate(config, 3);
}

void* CG_SHUDElementObituaries4Create(const superhudConfig_t* config)
{
	return CG_SHUDElementObituariesCreate(config, 4);
}

void* CG_SHUDElementObituaries5Create(const superhudConfig_t* config)
{
	return CG_SHUDElementObituariesCreate(config, 5);
}

void* CG_SHUDElementObituaries6Create(const superhudConfig_t* config)
{
	return CG_SHUDElementObituariesCreate(config, 6);
}

void* CG_SHUDElementObituaries7Create(const superhudConfig_t* config)
{
	return CG_SHUDElementObituariesCreate(config, 7);
}

void* CG_SHUDElementObituaries8Create(const superhudConfig_t* config)
{
	return CG_SHUDElementObituariesCreate(config, 8);
}

static void CG_SHUDElementObituariesUpdatePosition(shudElementObituaries_t* element, superhudObituariesEntry_t* entry)
{
	CG_FontSelect(element->ctxAttacker.fontIndex); // update font metrics to make right calculation
	entry->runtime.attackerWidth = CG_OSPDrawStringLenPix(entry->runtime.attackerName, element->config.fontsize.value[0], element->ctxAttacker.flags, entry->runtime.maxNameLenPix);
	CG_FontSelect(element->ctxTarget.fontIndex);
	entry->runtime.targetWidth = CG_OSPDrawStringLenPix(entry->runtime.targetName, element->config.fontsize.value[0], element->ctxTarget.flags, entry->runtime.maxNameLenPix);

	if (element->config.alignH.value == SUPERHUD_ALIGNH_LEFT)
	{
		entry->runtime.baseX = element->config.rect.value[0];
	}
	else if (element->config.alignH.value == SUPERHUD_ALIGNH_RIGHT)
	{
		entry->runtime.baseX = element->config.rect.value[0] + element->config.rect.value[2] - (entry->runtime.attackerWidth + element->ctxMod.coord.named.w + entry->runtime.targetWidth + 2 * entry->runtime.spacing);
	}
	else // SUPERHUD_ALIGNH_CENTER
	{
		entry->runtime.baseX = element->config.rect.value[0] + (element->config.rect.value[2] / 2) - (element->ctxMod.coord.named.w / 2) - entry->runtime.attackerWidth;
	}
}

static void CG_SHUDElementObituariesInitializeRuntime(shudElementObituaries_t* element, superhudObituariesEntry_t* entry)
{
	entry->runtime.maxVisibleChars = 13;
	entry->runtime.spacing = element->config.rect.value[3] / 4.0f;

	entry->runtime.maxNameLenPix = (element->config.rect.value[2] - (element->config.rect.value[3] + entry->runtime.spacing * 2)) / 2;

	entry->runtime.iconShader = CG_SHUDObituaryGetModIcon(entry->mod, entry->unfrozen);

	CG_SHUDObituarySetTeamColor(entry->runtime.attackerColor, entry->attackerTeam);
	CG_SHUDObituarySetTeamColor(entry->runtime.targetColor, entry->targetTeam);
	CG_SHUDObituarySetTeamColor(entry->runtime.targetColor, entry->targetTeam);
	VectorCopy(cgs.osp.enemyColors.legs, entry->runtime.enemyColor);


	if (element->config.bgcolor.isSet)
	{
		entry->runtime.attackerColor[3] = element->config.bgcolor.value[3];
		entry->runtime.targetColor[3] = element->config.bgcolor.value[3];
	}

	if (entry->attacker == ENTITYNUM_WORLD)
	{
		strcpy(entry->runtime.attackerName, "^1world");
	}
	else if (entry->attacker >= 0 && entry->attacker < MAX_CLIENTS)
	{
		Q_strncpyz(entry->runtime.attackerName, cgs.clientinfo[entry->attacker].name, MAX_QPATH);
	}


	if (entry->target >= 0 && entry->target < MAX_CLIENTS)
	{
		Q_strncpyz(entry->runtime.targetName, cgs.clientinfo[entry->target].name, MAX_QPATH);
	}

	entry->runtime.isInitialized = qtrue;
}

void CG_SHUDElementObituariesRoutine(void* context)
{
	shudElementObituaries_t* element = (shudElementObituaries_t*)context;
	superhudObituariesEntry_t* entry;
	int index;
	float currentX;

	index = (element->gctx->obituaries.index - element->index) % SHUD_MAX_OBITUARIES_LINES;
	entry = &element->gctx->obituaries.line[index];

	if (entry->time == 0 || !CG_SHUDGetFadeColor(element->ctxAttacker.color_origin, element->ctxAttacker.color, &element->config, entry->time))
	{
		entry->time = 0;
		return;
	}

	element->ctxMod.coord.named.w = element->config.rect.value[3];   // icon size scaling
	element->ctxMod.coord.named.h = element->config.rect.value[3];

	if (!entry->runtime.isInitialized)
	{
		CG_SHUDElementObituariesInitializeRuntime(element, entry);
	}

	CG_SHUDElementObituariesUpdatePosition(element, entry);

	currentX = entry->runtime.baseX;

	if (entry->attacker != entry->target)
	{
		element->ctxAttacker.text = entry->runtime.attackerName;
		element->ctxAttacker.coord.named.x = currentX;

		if ((entry->attackerTeam == TEAM_RED || entry->attackerTeam == TEAM_BLUE) && element->config.style.isSet && element->config.style.value)
		{
			if (element->config.style.value == 1 || cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR)
			{
				Vector4Copy(entry->runtime.attackerColor, element->ctxAttacker.background);
			}
			else if (element->config.style.value == 2)
			{
				if (cgs.clientinfo[cg.snap->ps.clientNum].team == entry->attackerTeam)
				{
					element->ctxAttacker.background[3] = 0;
				}
				else
				{
					Vector4Copy(entry->runtime.attackerColor, element->ctxAttacker.background);
				}
			}
			else if (element->config.style.value == 3)
			{
				if (cgs.clientinfo[cg.snap->ps.clientNum].team == entry->attackerTeam)
				{
					element->ctxAttacker.background[3] = 0;
				}
				else
				{
					Vector4Copy(entry->runtime.enemyColor, element->ctxAttacker.background);
				}
			}
			else if (element->config.style.value == 4)
			{
				if (cgs.clientinfo[cg.snap->ps.clientNum].team == entry->attackerTeam)
				{
					Vector4Copy(entry->runtime.attackerColor, element->ctxAttacker.background);
				}
				else
				{
					element->ctxAttacker.background[3] = 0;
				}
			}
		}
		else
		{
			element->ctxAttacker.background[3] = 0;
		}

		element->ctxAttacker.width = entry->runtime.maxNameLenPix;
		CG_SHUDTextPrint(&element->config, &element->ctxAttacker);
		currentX += entry->runtime.attackerWidth;
	}
	if (entry->runtime.iconShader)
	{
		element->ctxMod.image = entry->runtime.iconShader;
		element->ctxMod.coord.named.x = currentX + entry->runtime.spacing;
		CG_SHUDDrawStretchPicCtx(&element->config, &element->ctxMod);
		currentX += element->ctxMod.coord.named.w + entry->runtime.spacing * 2;
	}
	element->ctxTarget.text = entry->runtime.targetName;
	element->ctxTarget.coord.named.x = currentX;
	if ((entry->targetTeam == TEAM_RED || entry->targetTeam == TEAM_BLUE) && element->config.style.isSet && element->config.style.value)
	{
		if (element->config.style.value == 1 || cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR)
		{
			Vector4Copy(entry->runtime.targetColor, element->ctxTarget.background);
		}
		else if (element->config.style.value == 2)
		{
			if (cgs.clientinfo[cg.snap->ps.clientNum].team == entry->targetTeam)
			{
				element->ctxTarget.background[3] = 0;
			}
			else
			{
				Vector4Copy(entry->runtime.targetColor, element->ctxTarget.background);
			}
		}
		else if (element->config.style.value == 3)
		{
			if (cgs.clientinfo[cg.snap->ps.clientNum].team == entry->targetTeam)
			{
				element->ctxTarget.background[3] = 0;
			}
			else
			{
				Vector4Copy(entry->runtime.enemyColor, element->ctxTarget.background);
			}
		}
		else if (element->config.style.value == 4)
		{
			if (cgs.clientinfo[cg.snap->ps.clientNum].team == entry->targetTeam)
			{
				Vector4Copy(entry->runtime.targetColor, element->ctxTarget.background);
			}
			else
			{
				element->ctxTarget.background[3] = 0;
			}
		}
	}
	else
	{
		element->ctxTarget.background[3] = 0;
	}
	element->ctxTarget.width = entry->runtime.maxNameLenPix;
	CG_SHUDTextPrint(&element->config, &element->ctxTarget);
}

static void CG_SHUDObituarySetTeamColor(vec4_t color, int team)
{
	switch (team)
	{
		case TEAM_RED:
			VectorCopy(colorRed, color);
			break;
		case TEAM_BLUE:
			VectorCopy(colorBlue, color);
			break;
		default:
			VectorCopy(colorWhite, color);
			break;
	}
	color[3] = 0.25f;
}

static qhandle_t CG_SHUDObituaryGetModIcon(int mod, qboolean unfrozen)
{
	if (unfrozen)
	{
		return cgs.media.frozenFoeTagShader;
	}

	switch (mod)
	{
		case MOD_SHOTGUN:
			return cg_weapons[WP_SHOTGUN].ammoIcon;
		case MOD_GAUNTLET:
			return cg_weapons[WP_GAUNTLET].ammoIcon;
		case MOD_MACHINEGUN:
			return cg_weapons[WP_MACHINEGUN].ammoIcon;
		case MOD_GRENADE:
			return cgs.media.obituariesGrenadeDirect;
		case MOD_GRENADE_SPLASH:
			return cg_weapons[WP_GRENADE_LAUNCHER].ammoIcon;
		case MOD_ROCKET:
			return cgs.media.obituariesRocketDirect;
		case MOD_ROCKET_SPLASH:
			return cg_weapons[WP_ROCKET_LAUNCHER].ammoIcon;
		case MOD_PLASMA:
			return cg_weapons[WP_PLASMAGUN].ammoIcon;
		case MOD_PLASMA_SPLASH:
			return cg_weapons[WP_PLASMAGUN].ammoIcon;
		case MOD_RAILGUN:
			return cg_weapons[WP_RAILGUN].ammoIcon;
		case MOD_LIGHTNING:
			return cg_weapons[WP_LIGHTNING].ammoIcon;
		case MOD_BFG:
			return cgs.media.obituariesBFGDirect;
		case MOD_BFG_SPLASH:
			return cg_weapons[WP_BFG].ammoIcon;
		case MOD_WATER:
			return cgs.media.obituariesDrowned;
		case MOD_SLIME:
			return cgs.media.obituariesMelted;
		case MOD_LAVA:
			return cgs.media.obituariesLava;
		case MOD_TELEFRAG:
			return cgs.media.obituariesTelefrag;
		case MOD_FALLING:
			return cgs.media.obituariesFallenCrashed;
		case MOD_TRIGGER_HURT:
			return cgs.media.obituariesFalling;
		case MOD_SUICIDE:
		case MOD_TARGET_LASER:
		case MOD_GRAPPLE:
		case MOD_UNKNOWN:
		case MOD_CRUSH:
			return cgs.media.obituariesSkull;
		default:
			break;
	}
	return 0;// no shader
}

void CG_SHUDElementObituariesDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
