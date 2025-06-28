#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

enum shudPWType
{
	SHUDPWTYPE_TIME,
	SHUDPWTYPE_ICON,
};

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t textCtx;
	superhudDrawContext_t drawCtx;
	enum shudPWType pwType;
	int pwIndex;
} shudElementPowerupContext;

static void* CG_SHUDElementPwCreate(const superhudConfig_t* config, enum shudPWType pwType, int pwIndex)
{
	shudElementPowerupContext* element;

	SHUD_ELEMENT_INIT(element, config);

	element->pwType = pwType;
	element->pwIndex = pwIndex;

	if (pwType == SHUDPWTYPE_TIME)
	{
		CG_SHUDTextMakeContext(&element->config, &element->textCtx);
	}
	else
	{
		CG_SHUDDrawMakeContext(&element->config, &element->drawCtx);
	}

	return element;
}

void* CG_SHUDElementPwTime1Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 1);
}

void* CG_SHUDElementPwTime2Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 2);
}

void* CG_SHUDElementPwTime3Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 3);
}

void* CG_SHUDElementPwTime4Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 4);
}

void* CG_SHUDElementPwTime5Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 5);
}

void* CG_SHUDElementPwTime6Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 6);
}

void* CG_SHUDElementPwTime7Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 7);
}

void* CG_SHUDElementPwTime8Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 8);
}

void* CG_SHUDElementPwIcon1Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 1);
}

void* CG_SHUDElementPwIcon2Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 2);
}

void* CG_SHUDElementPwIcon3Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 3);
}

void* CG_SHUDElementPwIcon4Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 4);
}

void* CG_SHUDElementPwIcon5Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 5);
}

void* CG_SHUDElementPwIcon6Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 6);
}

void* CG_SHUDElementPwIcon7Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 7);
}

void* CG_SHUDElementPwIcon8Create(const superhudConfig_t* config)
{
	return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 8);
}

static void CG_SHUDElementPwUpdateState(struct superhudPowerupsCache_t* pw)
{
	int     i, j, k;
	playerState_t*   ps;
	int     t;
	gitem_t* item;

	ps = &cg.snap->ps;

	if (ps->stats[STAT_HEALTH] <= 0)
	{
		return;
	}

	// sort the list by time remaining
	pw->numberOfActive = 0;
	for (i = 0 ; i < MAX_POWERUPS ; i++)
	{
		if (!ps->powerups[ i ])
		{
			continue;
		}
		t = ps->powerups[ i ] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if (t < 0 || t > 999000)
		{
			continue;
		}

		// insert with sort by time
		for (j = 0 ; j < pw->numberOfActive ; j++)
		{
			if (pw->element[j].time >= t)
			{
				for (k = pw->numberOfActive - 1 ; k >= j ; k--)
				{
					pw->element[k + 1].powerup = pw->element[k].powerup;
					pw->element[k + 1].time = pw->element[k].time;
					pw->element[k + 1].isHoldable = pw->element[k].isHoldable;
				}
				break;
			}
		}
		pw->element[j].powerup = i;
		pw->element[j].time = t;
		pw->element[j].isHoldable = qfalse;
		++pw->numberOfActive;
	}

	{
		int hi = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
		if (hi)
		{
			pw->element[pw->numberOfActive].powerup = hi;
			pw->element[pw->numberOfActive].time = SUPERHUD_UPDATE_TIME * 2;
			pw->element[pw->numberOfActive].isHoldable = qtrue;
			++pw->numberOfActive;
		}
	}

	for (i = 0 ; i < pw->numberOfActive ; i++)
	{
		if (pw->element[i].isHoldable)
		{
			CG_RegisterItemVisuals(pw->element[i].powerup);
			pw->element[i].powerup = cg_items[pw->element[i].powerup].icon;
			pw->element[i].time /= 1000;
		}
		else
		{
			item = BG_FindItemForPowerup(pw->element[i].powerup);
			if (item)
			{
				pw->element[i].powerup = trap_R_RegisterShader(item->icon);
				pw->element[i].time /= 1000;
			}
		}
	}
}

void CG_SHUDElementPwRoutine(void* context)
{
	shudElementPowerupContext* element = (shudElementPowerupContext*)context;
	struct superhudPowerupsCache_t* pw = &CG_SHUDGetContext()->powerupsCache;
	struct superhudPowerupElement_t* pwElement;

	if (cg.time - pw->lastUpdateTime > SUPERHUD_UPDATE_TIME)
	{
		CG_SHUDElementPwUpdateState(pw);
		pw->lastUpdateTime = cg.time;
	}

	if (!pw->numberOfActive || element->pwIndex > pw->numberOfActive)
	{
		//nothing to do
		return;
	}

	CG_SHUDFill(&element->config);

	pwElement = &pw->element[element->pwIndex - 1];

	if (element->pwType == SHUDPWTYPE_TIME)
	{
		if (!pwElement->isHoldable)
		{
			element->textCtx.text = va("%d", pwElement->time);
			CG_SHUDTextPrint(&element->config, &element->textCtx);
		}
	}
	else
	{
		element->drawCtx.image = pwElement->powerup;
		CG_SHUDDrawStretchPicCtx(&element->config, &element->drawCtx);
	}
}

void CG_SHUDElementPwDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}



