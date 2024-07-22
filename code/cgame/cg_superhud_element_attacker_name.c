#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementAttackerName_t;

void* CG_SHUDElementAttackerNameCreate(superhudConfig_t* config)
{
	shudElementAttackerName_t* an;

	an = Z_Malloc(sizeof(*an));
	OSP_MEMORY_CHECK(an);

	memset(an, 0, sizeof(*an));

	memcpy(&an->config, config, sizeof(an->config));

	CG_SHUDTextMakeContext(&an->config, &an->ctx);
	an->ctx.maxchars = MAX_QPATH;

	return an;
}

void CG_SHUDElementAttackerNameRoutine(void* context)
{
	shudElementAttackerName_t* element = (shudElementAttackerName_t*)context;
	const float* fade;
	int         t;
	const char*  info;
	const char*  name;
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
	name = Info_ValueForKey(info, "n");

	CG_SHUDTextPrint(name, &element->ctx);
}

void CG_SHUDElementAttackerNameDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

