#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementTargetName_t;

void* CG_SHUDElementTargetNameCreate(const superhudConfig_t* config)
{
	shudElementTargetName_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementTargetNameRoutine(void* context)
{
	shudElementTargetName_t* element = (shudElementTargetName_t*)context;
	char    s[1024];
	clientInfo_t* ci;
	qboolean hide = qfalse;

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
	if (cgs.gametype >= GT_TEAM && cg_drawCrosshairNames.integer == 2)
	{
		if (cgs.osp.gameTypeFreeze)
		{
			hide = ci->rt != cg.snap->ps.persistant[PERS_TEAM];
		}
		else
		{
			hide = ci->team != cg.snap->ps.persistant[PERS_TEAM];
		}

	}

	if (!hide)
	{
		Com_sprintf(s, 1024, "%s", ci->name);
		CG_SHUDFill(&element->config);
		element->ctx.text = s;
		CG_SHUDTextPrint(&element->config, &element->ctx);
		element->ctx.text = NULL;
	}
}

void CG_SHUDElementTargetNameDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

