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
static int CG_TruncateStringWithCodes(const char* input, char* output, int maxVisibleChars);
static void CG_SHUDStylesObituaries_Bars(float x, float y, float width, float height, vec4_t color, int style, int team);

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

static void CG_SHUDElementObituariesInitializeRuntime(shudElementObituaries_t* element, superhudObituariesEntry_t* entry)
{
	entry->runtime.maxVisibleChars = 13;
	entry->runtime.spacing = element->config.rect.value[3] / 2.0f;

	entry->runtime.iconShader = CG_SHUDObituaryGetModIcon(entry->mod, entry->unfrozen);

	CG_SHUDObituarySetTeamColor(entry->runtime.attackerColor, entry->attackerTeam);
	CG_SHUDObituarySetTeamColor(entry->runtime.targetColor, entry->targetTeam);

	if (entry->attacker == ENTITYNUM_WORLD)
	{
		strcpy(entry->runtime.truncatedAttacker, "^1world");
	}
	else if (entry->attacker >= 0 && entry->attacker < MAX_CLIENTS)
	{
		(void)CG_TruncateStringWithCodes(cgs.clientinfo[entry->attacker].name, entry->runtime.truncatedAttacker, entry->runtime.maxVisibleChars);
	}


	if (entry->target >= 0 && entry->target < MAX_CLIENTS)
	{
		(void)CG_TruncateStringWithCodes(cgs.clientinfo[entry->target].name, entry->runtime.truncatedTarget, entry->runtime.maxVisibleChars);
	}

	entry->runtime.attackerWidth = CG_OSPDrawStringLenPix(entry->runtime.truncatedAttacker, element->config.fontsize.value[0], MAX_QPATH, element->ctxAttacker.flags);
	entry->runtime.targetWidth = CG_OSPDrawStringLenPix(entry->runtime.truncatedTarget, element->config.fontsize.value[0], MAX_QPATH, element->ctxTarget.flags);

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
		entry->runtime.baseX = (element->config.rect.value[2] / 2) - (element->ctxMod.coord.named.w / 2) - entry->runtime.attackerWidth;
	}

	entry->runtime.isInitialized = qtrue;
}

void CG_SHUDElementObituariesRoutine(void* context)
{
	shudElementObituaries_t* element = (shudElementObituaries_t*)context;
	superhudObituariesEntry_t* entry;
	int index;
	//float currentX, baseX, attackerWidth, targetWidth;
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

	currentX = entry->runtime.baseX;

	if (entry->attacker != entry->target)
	{
		element->ctxAttacker.text = entry->runtime.truncatedAttacker;
		element->ctxAttacker.coord.named.x = currentX;
		CG_SHUDTextPrint(&element->config, &element->ctxAttacker);
		CG_SHUDStylesObituaries_Bars(currentX, element->ctxAttacker.coord.named.y, entry->runtime.attackerWidth, element->config.fontsize.value[1], entry->runtime.attackerColor, element->config.style.value, entry->attackerTeam);
		currentX += entry->runtime.attackerWidth;
	}
	if (entry->runtime.iconShader)
	{
		element->ctxMod.image = entry->runtime.iconShader;
		element->ctxMod.coord.named.x = currentX + entry->runtime.spacing;
		//element->ctxMod.coord.named.y = element->ctxAttacker.coord.named.y + (element->config.fontsize.value[0] - element->ctxMod.coord.named.w) / 2; // Центрирование по вертикали
		CG_SHUDDrawStretchPicCtx(&element->config, &element->ctxMod);
		currentX += element->ctxMod.coord.named.w + entry->runtime.spacing * 2;
	}
	element->ctxTarget.text = entry->runtime.truncatedTarget;
	element->ctxTarget.coord.named.x = currentX;
	CG_SHUDTextPrint(&element->config, &element->ctxTarget);
	CG_SHUDStylesObituaries_Bars(currentX, element->ctxTarget.coord.named.y, entry->runtime.targetWidth, element->config.fontsize.value[1], entry->runtime.targetColor, element->config.style.value, entry->targetTeam);
}


static int CG_TruncateStringWithCodes(const char* input, char* output, int maxVisibleChars)
{
	int visibleCount = 0, actualCount = 0;
	int maxVisible = 0, currentVisibleCount = 0;
	qboolean mode = qfalse;
	const char* src = input;
	char* dst = output;
	int i;

	while (*src && visibleCount < maxVisibleChars)
	{
		if (*src == '^')
		{
			const char* next = src + 1;

			if (*next == 'x' || *next == 'X')
			{
				*dst++ = *src++;
				*dst++ = *src++;
				for (i = 0; i < 6 && ((*src >= '0' && *src <= '9') || (*src >= 'a' && *src <= 'f') || (*src >= 'A' && *src <= 'F')); i++)
				{
					*dst++ = *src++;
				}
			}
			else if (*next == 'f' || *next == 'F' || *next == 'b' || *next == 'B')
			{
				mode = qtrue;
				src += 2;
				currentVisibleCount = 0;
			}

			else if ((*next >= '0' && *next <= '9') || *next == 'n' || *next == 'N' || *next == '#')
			{
				*dst++ = *src++;
				*dst++ = *src++;
			}
			else
			{
				*dst++ = *src++;
				*dst++ = *src++;
			}
		}
		else if (*src == '\x19')
		{
			*dst++ = *src++;
		}
		else
		{
			*dst++ = *src++;
			visibleCount++;
			actualCount++;
			currentVisibleCount++;

			if (mode && currentVisibleCount > maxVisible)
			{
				maxVisible = currentVisibleCount;
			}
		}
	}
	*dst = '\0';
	return maxVisible > actualCount ? maxVisible : actualCount;
}

static void CG_SHUDObituarySetTeamColor(vec4_t color, int team)
{
	switch (team)
	{
		case 1:
			Vector4Set(color, 1.0f, 0.0f, 0.0f, 0.75f);
			break;
		case 2:
			Vector4Set(color, 0.0f, 0.0f, 1.0f, 0.75f);
			break;
		default:
			Vector4Set(color, 0.5f, 0.5f, 0.5f, 1.0f);
			break;
	}
}

static void CG_SHUDStylesObituaries_Bars(float x, float y, float width, float height, vec4_t color, int style, int team)
{
	if (team != 1 && team != 2)
	{
		return;
	}

	if (style == 1)  // colored background
	{
		color[3] = 0.1f;
		CG_FillRect(x - 2, y - ((height + 4) / 2), width + 4, height + 4, color);
	}
	else if (style == 2)  // colored underline
	{
		CG_FillRect(x, y + (height / 2), width, 1, color);
	}
	else
	{
		return;
	}
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
			return trap_R_RegisterShader("ObituariesGrenadeDirect");
		case MOD_GRENADE_SPLASH:
			return cg_weapons[WP_GRENADE_LAUNCHER].ammoIcon;
		case MOD_ROCKET:
			return trap_R_RegisterShader("ObituariesRocketDirect");
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
			return trap_R_RegisterShader("ObituariesBFGDirect");
		case MOD_BFG_SPLASH:
			return cg_weapons[WP_BFG].ammoIcon;
		case MOD_WATER:
			return trap_R_RegisterShader("ObituariesDrowned");
		case MOD_SLIME:
			return trap_R_RegisterShader("ObituariesMelted");
		case MOD_LAVA:
			return trap_R_RegisterShader("ObituariesLava");
		case MOD_TELEFRAG:
			return trap_R_RegisterShader("ObituariesTelefrag");
		case MOD_FALLING:
			return trap_R_RegisterShader("ObituariesFallenCrashed");
		case MOD_TRIGGER_HURT:
			return trap_R_RegisterShader("ObituariesFalling");
		case MOD_SUICIDE:
		case MOD_TARGET_LASER:
		case MOD_GRAPPLE:
		case MOD_UNKNOWN:
		case MOD_CRUSH:
			return trap_R_RegisterShader("ObituariesSkull");
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
