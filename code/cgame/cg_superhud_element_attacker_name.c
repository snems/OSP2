#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementAttackerName_t;

void* CG_SHUDElementAttackerNameCreate(const superhudConfig_t* config)
{
	shudElementAttackerName_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	if (!element->config.time.isSet)
	{
		element->config.time.value = ATTACKER_HEAD_TIME;
		element->config.time.isSet = qtrue;
	}

	return element;
}

void CG_SHUDElementAttackerNameRoutine(void* context)
{
	shudElementAttackerName_t* element = (shudElementAttackerName_t*)context;
	const char*  info;
	int         clientNum;

	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
	{
		return;
	}

	if (!cg.attackerTime)
	{
		return;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if (clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum)
	{
		return;
	}

	if (!CG_SHUDGetFadeColor(element->ctx.color_origin, element->ctx.color, &element->config, cg.attackerTime))
	{
		cg.attackerTime = 0;
		return;
	}

	CG_SHUDFill(&element->config);

	info = CG_ConfigString(CS_PLAYERS + clientNum);
	element->ctx.text = Info_ValueForKey(info, "n");

	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementAttackerNameDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

