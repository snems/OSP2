#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef enum
{
	SHUD_REWARD_ICON,
	SHUD_REWARD_COUNT,
} shudRewardType_t;

typedef struct
{
	superhudConfig_t config;
	union
	{
		superhudDrawContext_t d;
		superhudTextContext_t t;
	}ctx;
	shudRewardType_t type;
} shudElementStatusbarRewards;

static void* CG_SHUDElementRewardCreate(superhudConfig_t* config, shudRewardType_t type)
{
	shudElementStatusbarRewards* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	element->type = type;

	if (type == SHUD_REWARD_ICON)
	{
		CG_SHUDDrawMakeContext(&element->config, &element->ctx.d);
	}
	else
	{
		CG_SHUDTextMakeContext(&element->config, &element->ctx.t);
		if (!element->config.text.isSet)
		{
			element->config.text.isSet = qtrue;
			Q_strncpyz(element->config.text.value, "%d", sizeof(element->config.text.value));
		}
	}

	return element;
}

void* CG_SHUDElementRewardIconCreate(superhudConfig_t* config)
{
	return CG_SHUDElementRewardCreate(config, SHUD_REWARD_ICON);
}

void* CG_SHUDElementRewardCountCreate(superhudConfig_t* config)
{
	return CG_SHUDElementRewardCreate(config, SHUD_REWARD_COUNT);
}

void CG_SHUDElementRewardRoutine(void* context)
{
	shudElementStatusbarRewards* element = (shudElementStatusbarRewards*)context;

	float*   color;
	int     i, count;
	float   x, y, w, h;
	char    buf[32];

	if (!cg_drawRewards.integer)
	{
		return;
	}

	color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
	if (!color)
	{
		if (cg.rewardStack > 0)
		{
			for (i = 0; i < cg.rewardStack; i++)
			{
				cg.rewardSound[i] = cg.rewardSound[i + 1];
				cg.rewardShader[i] = cg.rewardShader[i + 1];
				cg.rewardCount[i] = cg.rewardCount[i + 1];
			}
			cg.rewardTime = cg.time;
			cg.rewardStack--;
			color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
		}
		else
		{
			return;
		}
	}

	if (element->type == SHUD_REWARD_ICON)
	{
		trap_R_SetColor(color);
		trap_R_DrawStretchPic(element->ctx.d.x, element->ctx.d.y, element->ctx.d.w + 1, element->ctx.d.h, 0, 0, 1, 1, cg.rewardShader[0]);
		trap_R_SetColor(NULL);
	}
	else if (1 || cg.rewardCount[0])
	{
		const char *s;
		s = va(element->config.text.value, cg.rewardCount[0]);
		CG_SHUDTextPrint(s, &element->ctx.t);
	}
}

void CG_SHUDElementRewardDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

