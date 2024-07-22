#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

enum flagType_t
{
	SHUDFLTYPE_OWN,
	SHUDFLTYPE_NME,
} flagType;

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
	enum flagType_t flagType;
} shudElementFlagStatus_t;

static void* CG_SHUDElementFlagStatusCreate(superhudConfig_t* config, enum flagType_t flagType)
{
	shudElementFlagStatus_t* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	element->flagType = flagType;

	return element;
}

void* CG_SHUDElementFlagStatusNMECreate(superhudConfig_t* config)
{
	return CG_SHUDElementFlagStatusCreate(config, SHUDFLTYPE_NME);
}

void* CG_SHUDElementFlagStatusOWNCreate(superhudConfig_t* config)
{
	return CG_SHUDElementFlagStatusCreate(config, SHUDFLTYPE_OWN);
}

void CG_SHUDElementFlagStatusRoutine(void* context)
{
	shudElementFlagStatus_t* element = (shudElementFlagStatus_t*)context;
	team_t team;
	gitem_t* item;
	qhandle_t flag;

	team = cgs.clientinfo[cg.snap->ps.clientNum].team;

	if (element->flagType == SHUDFLTYPE_NME)
	{
		switch (team)
		{
			case TEAM_RED:
				team = TEAM_BLUE;
				break;
			case TEAM_BLUE:
				team = TEAM_RED;
				break;
			case TEAM_FREE:
			case TEAM_SPECTATOR:
			case TEAM_4:
			case TEAM_5:
			case TEAM_6:
			case TEAM_7:
			case TEAM_NUM_TEAMS:
				return;
		}
	}

	if (team == TEAM_RED)
	{
		item = BG_FindItemForPowerup(PW_REDFLAG);
		flag = cgs.media.redFlagShader[cgs.redflag];
	}
	else if (team == TEAM_BLUE)
	{
		item = BG_FindItemForPowerup(PW_BLUEFLAG);
		flag = cgs.media.blueFlagShader[cgs.blueflag];
	}
	else
	{
		return;
	}

	CG_SHUDFill(&element->config);

	if (item && flag)
	{
		// установка цвета пока отключена, надо доработать шейдеры
		//trap_R_SetColor(element->ctx.color);
		trap_R_DrawStretchPic(element->ctx.x, element->ctx.y, element->ctx.w, element->ctx.h, 0, 0, 1, 1, flag);
		//trap_R_SetColor(NULL);
	}
}

void CG_SHUDElementFlagStatusDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

