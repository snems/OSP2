#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementTargetStatus_t;

void* CG_SHUDElementTargetStatusCreate(const superhudConfig_t* config)
{
	shudElementTargetStatus_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.maxchars = MAX_QPATH;

	return element;
}

void CG_SHUDElementTargetStatusRoutine(void* context)
{
	shudElementTargetStatus_t* element = (shudElementTargetStatus_t*)context;
	char    s[1024];
	float* fade;
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

	if ((ci->team == cg.snap->ps.persistant[PERS_TEAM]) || (cgs.osp.gameTypeFreeze && ci->team == cgs.clientinfo[cg.snap->ps.clientNum].team))
	{
		if (ci->team != TEAM_FREE && ci->team == cg.snap->ps.persistant[PERS_TEAM] && ch_TeamCrosshairHealth.integer != 0 && !(cg.snap->ps.pm_flags & PMF_FOLLOW))
		{
			Com_sprintf(s, 1024, "^5[^7%i/%i^5]", ci->health, ci->armor);
	    CG_SHUDFill(&element->config);
	    element->ctx.text = s;
			CG_SHUDTextPrint(&element->ctx);
		}
	}
}

void CG_SHUDElementTargetStatusDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

