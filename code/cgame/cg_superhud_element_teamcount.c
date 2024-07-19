#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t position;
	qboolean enemy;
} shudElementStatusbarTeamCount;

void* CG_SHUDElementTeamCountCreate(superhudConfig_t* config, qboolean enemy)
{
	shudElementStatusbarTeamCount* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	//load defaults
	if (!element->config.color.isSet)
	{
		element->config.color.isSet = qtrue;
		element->config.color.value.type = SUPERHUD_COLOR_RGBA;
		Vector4Set(element->config.color.value.rgba, 1, 0.7, 0, 1);
	}

	if (!element->config.text.isSet)
	{
		element->config.text.isSet = qtrue;
		Q_strncpyz(element->config.text.value, "%i", sizeof(element->config.text.value));
	}

	CG_SHUDTextMakeContext(&element->config, &element->position);
	element->position.maxchars = 6;

	element->enemy = enemy;

	return element;
}

void* CG_SHUDElementTeamCountOWNCreate(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCountCreate(config, qfalse);
}

void* CG_SHUDElementTeamCountNMECreate(superhudConfig_t* config)
{
	return CG_SHUDElementTeamCountCreate(config, qtrue);
}

void CG_SHUDElementTeamCountRoutine(void* context)
{
	shudElementStatusbarTeamCount* element = (shudElementStatusbarTeamCount*)context;
	const char* s;
	int count;
	team_t team;

	if (!CG_OSPIsGameTypeFreeze())
	{
		return;
	}

	team = CG_SHUDGetOurActiveTeam();

	if (team == TEAM_RED)
	{
		count	= element->enemy ? cgs.osp.osp_teamcount2 : cgs.osp.osp_teamcount1;
	}
	else if (team == TEAM_BLUE)
	{
		count	= element->enemy ? cgs.osp.osp_teamcount1 : cgs.osp.osp_teamcount2;
	}
	else
	{
		return;
	}

	if ( count > 0)
	{
		s = va(element->config.text.value, count);
		CG_SHUDTextPrint(s, &element->position);
	}

}

void CG_SHUDElementTeamCountDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

