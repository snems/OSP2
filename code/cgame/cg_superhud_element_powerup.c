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

static void* CG_SHUDElementPwCreate(superhudConfig_t* config, enum shudPWType pwType, int pwIndex)
{
  shudElementPowerupContext *element;

  element = Z_Malloc(sizeof(*element));
  OSP_MEMORY_CHECK(element);


  memset(element,0,sizeof(*element));

  memcpy(&element->config, config, sizeof(element->config));

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

void* CG_SHUDElementPwTime1Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 1);
}

void* CG_SHUDElementPwTime2Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 2);
}

void* CG_SHUDElementPwTime3Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 3);
}

void* CG_SHUDElementPwTime4Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 4);
}

void* CG_SHUDElementPwTime5Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 5);
}

void* CG_SHUDElementPwTime6Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 6);
}

void* CG_SHUDElementPwTime7Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 7);
}

void* CG_SHUDElementPwTime8Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_TIME, 8);
}

void* CG_SHUDElementPwIcon1Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 1);
}

void* CG_SHUDElementPwIcon2Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 2);
}

void* CG_SHUDElementPwIcon3Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 3);
}

void* CG_SHUDElementPwIcon4Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 4);
}

void* CG_SHUDElementPwIcon5Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 5);
}

void* CG_SHUDElementPwIcon6Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 6);
}

void* CG_SHUDElementPwIcon7Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 7);
}

void* CG_SHUDElementPwIcon8Create(superhudConfig_t* config)
{
  return CG_SHUDElementPwCreate(config, SHUDPWTYPE_ICON, 8);
}

static void CG_SHUDElementPwUpdateState(struct superhudPowerupsCache_t *pw)
{
	int     i, j, k;
	playerState_t*   ps;
	int     t;
	gitem_t* item;
	int     color;
	float   size;
	float   f;

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

		// insert into the list
		for (j = 0 ; j < pw->numberOfActive ; j++)
		{
			if (pw->element[j].time >= t)
			{
				for (k = pw->numberOfActive - 1 ; k >= j ; k--)
				{
					pw->element[k + 1].powerup = pw->element[k].powerup;
					pw->element[k + 1].time = pw->element[k].time;
				}
				break;
			}
		}
		pw->element[j].powerup = i;
		pw->element[j].time = t;
		++pw->numberOfActive;
	}

	for (i = 0 ; i < pw->numberOfActive ; i++)
	{
		item = BG_FindItemForPowerup(pw->element[i].powerup);
		if (item)
		{
		  pw->element[i].powerup = trap_R_RegisterShader(item->icon);
		  pw->element[i].time /= 1000;
		}
	}
}

void CG_SHUDElementPwRoutine(void *context)
{
  shudElementPowerupContext *element = (shudElementPowerupContext *)context;
	struct superhudPowerupsCache_t *pw = &CG_SHUDGetContext()->powerupsCache;
  struct superhudPowerupElement_t *pwElement;

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

  pwElement = &pw->element[element->pwIndex - 1];

  if (element->pwType == SHUDPWTYPE_TIME)
  {
    CG_SHUDTextPrint(va("%d", pwElement->time), &element->textCtx);
  }
  else
  {
	  trap_R_DrawStretchPic(element->drawCtx.x, element->drawCtx.y, element->drawCtx.w, element->drawCtx.h, 
	                       0, 0, 1, 1, 
	                       pwElement->powerup);
  }
}

void CG_SHUDElementPwDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}



