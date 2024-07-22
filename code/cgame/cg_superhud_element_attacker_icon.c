#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarAttackerIcon;

void* CG_SHUDElementAttackerIconCreate(superhudConfig_t* config)
{
	shudElementStatusbarAttackerIcon* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	if (!config->rect.isSet)
	{
		element->config.rect.value[0] = 0;
		element->config.rect.value[1] = 0;
		element->config.rect.value[2] = 50;
		element->config.rect.value[3] = 50;
		element->config.rect.isSet = qtrue;
	}

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	if (config->image.isSet)
	{
		element->ctx.image = trap_R_RegisterShader(element->config.image.value);
	}

	return element;
}

void CG_SHUDElementAttackerIconRoutine(void* context)
{
	shudElementStatusbarAttackerIcon* element = (shudElementStatusbarAttackerIcon*)context;

	int         t;
	vec3_t      angles;
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

	angles[PITCH] = 0;
	angles[YAW] = 180;
	angles[ROLL] = 0;
	CG_DrawHead(element->config.rect.value[0], element->config.rect.value[1], element->config.rect.value[2], element->config.rect.value[3], clientNum, angles);

}

void CG_SHUDElementAttackerIconDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

