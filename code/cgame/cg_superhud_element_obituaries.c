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

static void* CG_SHUDElementObituariesCreate(const superhudConfig_t* config, int line)
{
	shudElementObituaries_t* element;
	superhudConfig_t lcfg;
	SHUD_ELEMENT_INIT(element, config);
	element->gctx = CG_SHUDGetContext();
	element->index = line;

	memcpy(&lcfg, config, sizeof(superhudConfig_t));

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

void CG_SHUDElementObituariesRoutine(void* context)
{
	shudElementObituaries_t* element = (shudElementObituaries_t*)context;
	superhudObituariesEntry_t* entry;
	qhandle_t iconShader;
	int index;
	float currentX, baseX, attackerWidth, targetWidth;
	const float spacing = 8.0f;
	char truncatedAttacker[MAX_QPATH];
	char truncatedTarget[MAX_QPATH];
	int maxVisibleChars = 13;
	int attackerVisibleChars, targetVisibleChars;
	vec4_t attackerColor, targetColor;

	index = (element->gctx->obituaries.index - element->index) % SHUD_MAX_OBITUARIES_LINES;
	entry = &element->gctx->obituaries.line[index];

	if (entry->time == 0 || !CG_SHUDGetFadeColor(element->ctxAttacker.color_origin, element->ctxAttacker.color, &element->config, entry->time))
	{
		entry->time = 0;
		return;
	}

	element->ctxMod.coord.named.w = element->config.fontsize.value[0] * 1.6f;   // icon size scaling
	element->ctxMod.coord.named.h = element->config.fontsize.value[1] * 1.6f;

	iconShader = CG_SHUDObituaryGetModIcon(entry->mod);

	CG_SHUDObituarySetTeamColor(attackerColor, entry->attackerTeam);
	CG_SHUDObituarySetTeamColor(targetColor, entry->targetTeam);

	attackerVisibleChars = (entry->attacker == 1022) ? (strcpy(truncatedAttacker, "^1world"), 5) : CG_TruncateStringWithCodes(cgs.clientinfo[entry->attacker].name, truncatedAttacker, maxVisibleChars);
	attackerWidth = element->config.fontsize.value[0] * attackerVisibleChars;

	targetVisibleChars = CG_TruncateStringWithCodes(cgs.clientinfo[entry->target].name, truncatedTarget, maxVisibleChars);
	targetWidth = element->config.fontsize.value[0] * targetVisibleChars;

	if (element->config.alignH.value == SUPERHUD_ALIGNH_LEFT)
	{
		baseX = element->config.rect.value[0];
	}
	else if (element->config.alignH.value == SUPERHUD_ALIGNH_RIGHT)
	{
		baseX = element->config.rect.value[0] + element->config.rect.value[2] - (attackerWidth + element->ctxMod.coord.named.w + targetWidth + 2 * spacing);
	}
	else // SUPERHUD_ALIGNH_CENTER
	{
		baseX = (element->config.rect.value[2] / 2) - (element->ctxMod.coord.named.w / 2) - attackerWidth;
	}

	currentX = baseX;

	if (entry->attacker != entry->target)
	{
		element->ctxAttacker.text = truncatedAttacker;
		element->ctxAttacker.coord.named.x = currentX;
		CG_SHUDTextPrint(&element->config, &element->ctxAttacker);
		CG_SHUDStylesObituaries_Bars(currentX, element->ctxAttacker.coord.named.y + element->config.fontsize.value[1], attackerWidth, element->config.fontsize.value[1], attackerColor, element->config.style.value, entry->attackerTeam);
		currentX += attackerWidth;
	}
	if (iconShader)
	{
		element->ctxMod.image = iconShader;
		element->ctxMod.coord.named.x = currentX + spacing;
		element->ctxMod.coord.named.y = element->ctxAttacker.coord.named.y + (element->config.fontsize.value[0] - element->ctxMod.coord.named.w) / 2; // Центрирование по вертикали
		CG_SHUDDrawStretchPicCtx(&element->config, &element->ctxMod);
		currentX += element->ctxMod.coord.named.w + spacing * 2;
	}
	element->ctxTarget.text = truncatedTarget;
	element->ctxTarget.coord.named.x = currentX;
	CG_SHUDTextPrint(&element->config, &element->ctxTarget);
	CG_SHUDStylesObituaries_Bars(currentX, element->ctxTarget.coord.named.y + element->config.fontsize.value[1], targetWidth, element->config.fontsize.value[1], targetColor, element->config.style.value, entry->targetTeam);
}


int CG_TruncateStringWithCodes(const char* input, char* output, int maxVisibleChars)
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
	switch (style)
	{
		case SUPERHUD_STYLE_1:  // colored background
			color[3] = 0.1f;
			CG_FillRect(x - 2, y - height - 2, width + 4, height + 4, color);
			break;
		case SUPERHUD_STYLE_2:  // colored underline
			CG_FillRect(x, y + 2, width, 1, color);
			break;
		default:
			break;
	}
}

qhandle_t CG_SHUDObituaryGetModIcon(int mod)
{
	qhandle_t iconShader = 0;
	switch (mod)
	{
		case 0: // MOD_UNKOWN
			iconShader = trap_R_RegisterShader("ObituariesSkull");
			break;
		case 1:  // MOD_SHOTGUN
			iconShader = cg_weapons[WP_SHOTGUN].ammoIcon;
			break;
		case 2:  // MOD_GAUNTLET
			iconShader = cg_weapons[WP_GAUNTLET].ammoIcon;
			break;
		case 3:  // MOD_MACHINEGUN
			iconShader = cg_weapons[WP_MACHINEGUN].ammoIcon;
			break;
		case 4:  // MOD_GRENADE
			iconShader = trap_R_RegisterShader("ObituariesGrenadeDirect");
			break;
		case 5:  // MOD_GRENADE_SPLASH
			iconShader = cg_weapons[WP_GRENADE_LAUNCHER].ammoIcon;
			break;
		case 6:  // MOD_ROCKET
			iconShader = trap_R_RegisterShader("ObituariesRocketDirect");
			break;
		case 7:  // MOD_ROCKET_SPLASH
			iconShader = cg_weapons[WP_ROCKET_LAUNCHER].ammoIcon;
			break;
		case 8:  // MOD_PLASMA
			iconShader = cg_weapons[WP_PLASMAGUN].ammoIcon;
			break;
		case 9:  // MOD_PLASMA_SPLASH
			iconShader = cg_weapons[WP_PLASMAGUN].ammoIcon;
			break;
		case 10: // MOD_RAILGUN
			iconShader = cg_weapons[WP_RAILGUN].ammoIcon;
			break;
		case 11: // MOD_LIGHTNING
			iconShader = cg_weapons[WP_LIGHTNING].ammoIcon;
			break;
		case 12: // MOD_BFG
			iconShader = trap_R_RegisterShader("ObituariesBFGDirect");
			break;
		case 13: // MOD_BFG_SPLASH
			iconShader = cg_weapons[WP_BFG].ammoIcon;
			break;
		case 14: // drowned
			iconShader = trap_R_RegisterShader("ObituariesDrowned");
			break;
		case 15: // melted
			iconShader = trap_R_RegisterShader("ObituariesMelted");
			break;
		case 16: // LAVA
			iconShader = trap_R_RegisterShader("ObituariesLava");
			break;
		case 17: // CRUSH
			iconShader = trap_R_RegisterShader("ObituariesSkull");
			break;
		case 18: // telefrag
			iconShader = trap_R_RegisterShader("ObituariesTelefrag");
			break;
		case 19: // Fallen (crashed)
			iconShader = trap_R_RegisterShader("ObituariesFallenCrashed");
			break;
		case 20: // /kill
			iconShader = trap_R_RegisterShader("ObituariesSkull");
			break;
		case 21: // LASER
			iconShader = trap_R_RegisterShader("ObituariesSkull");
			break;
		case 22: // fell off the map
			iconShader = trap_R_RegisterShader("ObituariesFalling");
			break;
		case 23: // grapple...
			iconShader = trap_R_RegisterShader("ObituariesSkull");
			break;
		case 1337: // thawed
			iconShader = cgs.media.frozenFoeTagShader;
			break;
		default:
			iconShader = 0;
			break;
	}
	return iconShader;
}

void CG_SHUDElementObituariesDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
