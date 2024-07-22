#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementTargetName_t;

void* CG_SHUDElementTargetNameCreate(superhudConfig_t* config)
{
	shudElementTargetName_t* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.maxchars = MAX_QPATH;

	return element;
}

void CG_SHUDElementTargetNameRoutine(void* context)
{
	shudElementTargetName_t* element = (shudElementTargetName_t*)context;
	char    s[1024];
	clientInfo_t* ci;

	if (cg_drawCrosshair.integer == 0) return;
	if (cg_drawCrosshairNames.integer == 0) return;
	if (cg.renderingThirdPerson != 0) return;
	if (global_viewlistFirstOption > 1) return;

	CG_ScanForCrosshairEntity();
	if (cg.crosshairClientTime == 0) return;


	if (!CG_SHUDGetFadeColor(element->ctx.color_origin, element->ctx.color, &element->config, cg.crosshairClientTime))
	{
		cg.crosshairClientTime = 0;
		return;
	}

	ci = &cgs.clientinfo[cg.crosshairClientNum];

	// our team OR not team only OR not our freeze team
	if ((ci->team == cg.snap->ps.persistant[PERS_TEAM]) || cg_drawCrosshairNames.integer != 2 || (cgs.osp.gameTypeFreeze && ci->team == cgs.clientinfo[cg.snap->ps.clientNum].team))
	{
		Com_sprintf(s, 1024, "%s", ci->name);
	  CG_SHUDFill(&element->config);
		CG_SHUDTextPrint(s, &element->ctx);
	}
}

void CG_SHUDElementTargetNameDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

