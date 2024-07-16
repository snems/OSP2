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
  shudElementTargetName_t *tn;

  tn = Z_Malloc(sizeof(*tn));
  OSP_MEMORY_CHECK(tn);

  memset(tn,0,sizeof(*tn));

  memcpy(&tn->config, config, sizeof(tn->config));

  CG_SHUDTextMakeContext(&tn->config, &tn->ctx);
  tn->ctx.maxchars = MAX_QPATH;

  return tn;
}

void CG_SHUDElementTargetNameRoutine(void *context)
{
  shudElementTargetName_t *tn = (shudElementTargetName_t *)context;
	char    s[1024];
	float* fade;
	clientInfo_t* ci;

	if (cg_drawCrosshair.integer == 0) return;
	if (cg_drawCrosshairNames.integer == 0) return;
	if (cg.renderingThirdPerson != 0) return;
	if (global_viewlistFirstOption > 1) return;

	CG_ScanForCrosshairEntity();

	fade = CG_FadeColor(cg.crosshairClientTime, 1000);
	if (fade == NULL)
	{
		trap_R_SetColor(NULL);
		return;
	}

	ci = &cgs.clientinfo[cg.crosshairClientNum];

  // our team OR not team only OR not our freeze team
	if ((ci->team == cg.snap->ps.persistant[PERS_TEAM]) || cg_drawCrosshairNames.integer != 2 || (cgs.osp.gameTypeFreeze && ci->team == cgs.clientinfo[cg.snap->ps.clientNum].team))
	{
		if (ci->team != TEAM_FREE && ci->team == cg.snap->ps.persistant[PERS_TEAM] && ch_TeamCrosshairHealth.integer != 0 && !(cg.snap->ps.pm_flags & PMF_FOLLOW))
		{
			Com_sprintf(s, 1024, "%s %i/%i", ci->name, ci->health, ci->armor);
		}
		else
		{
			Com_sprintf(s, 1024, "%s", ci->name);
		}
	}
	tn->ctx.color[3] = fade[3];

  CG_SHUDTextPrint(s, &tn->ctx);
}

void CG_SHUDElementTargetNameDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}

