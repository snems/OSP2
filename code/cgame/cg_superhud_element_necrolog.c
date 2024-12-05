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
} shudElementNecrolog_t;


shudElementNecrolog_t cg_necrolog;

static void* CG_SHUDElementNecrologCreate(const superhudConfig_t* config, int line)
{
	shudElementNecrolog_t* element;
	superhudConfig_t lcfg;
	SHUD_ELEMENT_INIT(element, config);
	element->gctx = CG_SHUDGetContext();
	element->index = line;

	memcpy(&lcfg, config, sizeof(superhudConfig_t));
	CG_SHUDTextMakeContext(&lcfg, &element->ctxAttacker);

	lcfg.rect.value[2] = config->rect.value[3];
	lcfg.rect.value[3] = config->rect.value[3];
	CG_SHUDDrawMakeContext(&lcfg, &element->ctxMod);

	CG_SHUDTextMakeContext(&lcfg, &element->ctxTarget);

	return element;
}

void* CG_SHUDElementNecrolog1Create(const superhudConfig_t* config)
{
	return CG_SHUDElementNecrologCreate(config, 1);
}

void* CG_SHUDElementNecrolog2Create(const superhudConfig_t* config)
{
	return CG_SHUDElementNecrologCreate(config, 2);
}

void* CG_SHUDElementNecrolog3Create(const superhudConfig_t* config)
{
	return CG_SHUDElementNecrologCreate(config, 3);
}

void* CG_SHUDElementNecrolog4Create(const superhudConfig_t* config)
{
	return CG_SHUDElementNecrologCreate(config, 4);
}

void* CG_SHUDElementNecrolog5Create(const superhudConfig_t* config)
{
	return CG_SHUDElementNecrologCreate(config, 5);
}

void* CG_SHUDElementNecrolog6Create(const superhudConfig_t* config)
{
	return CG_SHUDElementNecrologCreate(config, 6);
}

void* CG_SHUDElementNecrolog7Create(const superhudConfig_t* config)
{
	return CG_SHUDElementNecrologCreate(config, 7);
}

void* CG_SHUDElementNecrolog8Create(const superhudConfig_t* config)
{
	return CG_SHUDElementNecrologCreate(config, 8);
}

qhandle_t CG_GetModIcon(int mod)
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
			iconShader = cg_weapons[WP_GRENADE_LAUNCHER].ammoIcon;
			break;
		case 5:  // MOD_GRENADE_SPLASH
			iconShader = cg_weapons[WP_GRENADE_LAUNCHER].ammoIcon;
			break;
		case 6:  // MOD_ROCKET
			iconShader = cg_weapons[WP_ROCKET_LAUNCHER].ammoIcon;
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
			iconShader = cg_weapons[WP_BFG].ammoIcon;
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
			// CG_Printf("CG_GetModIcon: Unknown mod value = %d\n", mod);
			iconShader = 0;
			break;
	}
	if (iconShader == 0)
	{
	}

	return iconShader;
}

static void SetTeamColor(vec4_t color, int team)
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

static void DrawObituaryBar(float x, float y, float width, float height, vec4_t color, int style, int team)
{
	if (team != 1 && team != 2)
	{
		return;
	}
	switch (style)
	{

		case SUPERHUD_OBITUARYSTYLE_1:
			break;
		case SUPERHUD_OBITUARYSTYLE_2:
			color[3] = 0.1f;
			CG_FillRect(x - 2, y - height - 4, width + 4, height + 4, color);
			break;
		case SUPERHUD_OBITUARYSTYLE_3:
			CG_FillRect(x, y, width, 1, color);
			break;
		default:
			break;
	}
}

void CG_SHUDElementNecrologRoutine(void* context)
{
	shudElementNecrolog_t* element;
	superhudNecrologEntry_t* entry;
	qhandle_t iconShader;
	int index;
	float currentX, iconSize, textSize;
	const float spacing = 8.0f;
	char truncatedTarget[128];
	char truncatedAttacker[128];
	vec4_t attackerColor, targetColor;
	float barY, barWidth;
	float centerX;

	element = (shudElementNecrolog_t*)context;

	index = (element->gctx->necrolog.index - element->index) % SHUD_MAX_NECROLOG_LINES;
	entry = &element->gctx->necrolog.line[index];

	if (entry->time == 0)
	{
		return;
	}

	if (!CG_SHUDGetFadeColor(element->ctxAttacker.color_origin, element->ctxAttacker.color, &element->config, entry->time))
	{
		entry->time = 0;
		return;
	}

	textSize = element->config.fontsize.value[0];
	iconSize = element->config.rect.value[3];
	if (entry->attacker == 1022 && entry->mod >= 1 && entry->mod <= 13)
	{
		strcpy(truncatedAttacker, "^1enviroment");
		entry->attackerLength = 10;
	}
	else
	{
		CG_TruncateStringWithCodes(cgs.clientinfo[entry->attacker].name, truncatedAttacker, 12);
		entry->attackerLength = strlen(cgs.clientinfo[entry->attacker].name_clean);
	}

	CG_TruncateStringWithCodes(cgs.clientinfo[entry->target].name, truncatedTarget, 12);
	entry->targetLength = strlen(cgs.clientinfo[entry->target].name_clean);

	if (entry->targetLength > 12) entry->targetLength = 12;
	if (entry->attackerLength > 12) entry->attackerLength = 12;

	SetTeamColor(attackerColor, entry->attackerTeam);
	SetTeamColor(targetColor, entry->targetTeam);

	if (element->config.alignH.value == SUPERHUD_ALIGNH_LEFT)
	{
		currentX = element->config.rect.value[0];

		if (entry->attacker != entry->target)
		{
			element->ctxAttacker.text = truncatedAttacker;
			element->ctxAttacker.coord.named.x = currentX;
			barY = element->ctxAttacker.coord.named.y + textSize + 2;
			barWidth = entry->attackerLength * textSize;

			DrawObituaryBar(currentX, barY, barWidth, textSize, attackerColor, element->config.obituarystyle.value, entry->attackerTeam);
			CG_SHUDTextPrint(&element->config, &element->ctxAttacker);

			currentX += barWidth + spacing;
		}

		iconShader = CG_GetModIcon(entry->mod);
		if (iconShader)
		{
			element->ctxMod.image = iconShader;
			element->ctxMod.coord.named.x = currentX;
			element->ctxMod.coord.named.w = iconSize;
			element->ctxMod.coord.named.h = iconSize;
			element->ctxMod.coord.named.y = element->ctxAttacker.coord.named.y + (textSize - iconSize) / 2; // Центрирование по вертикали
			CG_SHUDDrawStretchPicCtx(&element->config, &element->ctxMod);
		}

		currentX += iconSize + spacing;
		element->ctxTarget.text = truncatedTarget;
		element->ctxTarget.coord.named.x = currentX;
		barY = element->ctxTarget.coord.named.y + textSize + 2;
		barWidth = entry->targetLength * textSize;

		DrawObituaryBar(currentX, barY, barWidth, textSize, targetColor, element->config.obituarystyle.value, entry->targetTeam);
		CG_SHUDTextPrint(&element->config, &element->ctxTarget);

	}
	else if (element->config.alignH.value == SUPERHUD_ALIGNH_RIGHT)
	{
		currentX = element->config.rect.value[0] + element->config.rect.value[2];

		if (entry->attacker != entry->target)
		{
			currentX -= entry->attackerLength * textSize + spacing;
			element->ctxAttacker.text = truncatedAttacker;
			element->ctxAttacker.coord.named.x = currentX;
			barY = element->ctxAttacker.coord.named.y + textSize + 2;
			barWidth = entry->attackerLength * textSize;

			DrawObituaryBar(currentX, barY, barWidth, textSize, attackerColor, element->config.obituarystyle.value, entry->attackerTeam);
			CG_SHUDTextPrint(&element->config, &element->ctxAttacker);
		}

		currentX -= iconSize + spacing;
		iconShader = CG_GetModIcon(entry->mod);
		if (iconShader)
		{
			element->ctxMod.image = iconShader;
			element->ctxMod.coord.named.x = currentX;
			element->ctxMod.coord.named.w = iconSize;
			element->ctxMod.coord.named.h = iconSize;
			element->ctxMod.coord.named.y = element->ctxAttacker.coord.named.y + (textSize - iconSize) / 2; // Центрирование по вертикали
			CG_SHUDDrawStretchPicCtx(&element->config, &element->ctxMod);
		}

		currentX -= spacing + entry->targetLength * textSize;
		element->ctxTarget.text = truncatedTarget;
		element->ctxTarget.coord.named.x = currentX;
		barY = element->ctxTarget.coord.named.y + textSize + 2;
		barWidth = entry->targetLength * textSize;

		DrawObituaryBar(currentX, barY, barWidth, textSize, targetColor, element->config.obituarystyle.value, entry->targetTeam);
		CG_SHUDTextPrint(&element->config, &element->ctxTarget);

	}
	else if (element->config.alignH.value == SUPERHUD_ALIGNH_CENTER)
	{
		centerX = element->config.rect.value[0] + (element->config.rect.value[2] / 2);

		if (entry->attacker != entry->target)
		{
			currentX = centerX - (iconSize / 2) - (entry->attackerLength * textSize) - spacing;
			element->ctxAttacker.text = truncatedAttacker;
			element->ctxAttacker.coord.named.x = currentX;
			barY = element->ctxAttacker.coord.named.y + textSize + 2;
			barWidth = entry->attackerLength * textSize;

			DrawObituaryBar(currentX, barY, barWidth, textSize, attackerColor, element->config.obituarystyle.value, entry->attackerTeam);
			CG_SHUDTextPrint(&element->config, &element->ctxAttacker);
		}

		currentX = centerX - (iconSize / 2);
		iconShader = CG_GetModIcon(entry->mod);
		if (iconShader)
		{
			element->ctxMod.image = iconShader;
			element->ctxMod.coord.named.x = currentX;
			element->ctxMod.coord.named.w = iconSize;
			element->ctxMod.coord.named.h = iconSize;
			element->ctxMod.coord.named.y = element->ctxAttacker.coord.named.y + (textSize - iconSize) / 2; // Центрирование по вертикали
			CG_SHUDDrawStretchPicCtx(&element->config, &element->ctxMod);
		}

		currentX = centerX + (iconSize / 2) + spacing;
		element->ctxTarget.text = truncatedTarget;
		element->ctxTarget.coord.named.x = currentX;
		barY = element->ctxTarget.coord.named.y + textSize + 2;
		barWidth = entry->targetLength * textSize;

		DrawObituaryBar(currentX, barY, barWidth, textSize, targetColor, element->config.obituarystyle.value, entry->targetTeam);
		CG_SHUDTextPrint(&element->config, &element->ctxTarget);
	}
}






void CG_TruncateStringWithCodes(const char* input, char* output, int maxVisibleChars)
{
	int visibleCount = 0;
	const char* src = input;
	char* dst = output;
	int i;

	while (*src && visibleCount < maxVisibleChars)
	{
		if (*src == '^')
		{
			*dst++ = *src++;
			if (*src == 'x')
			{
				*dst++ = *src++;
				for (i = 0; i < 6 && *src && ((*src >= '0' && *src <= '9') || (*src >= 'a' && *src <= 'f') || (*src >= 'A' && *src <= 'F')); i++)
				{
					*dst++ = *src++;
				}
			}
			else if (*src == '#')
			{
				*dst++ = *src++;
				for (i = 0; i < 6 && *src && ((*src >= '0' && *src <= '9') || (*src >= 'a' && *src <= 'f') || (*src >= 'A' && *src <= 'F')); i++)
				{
					*dst++ = *src++;
				}
			}
			else if ((*src >= '0' && *src <= '9') || *src == 'n')
			{
				*dst++ = *src++;
			}
			else
			{
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
		}
	}

	*dst = '\0';
}

void CG_SHUDElementNecrologDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
