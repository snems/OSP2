#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

#define FPS_MAX_FRAMES  32
typedef struct
{
  superhudConfig_t config;
  superhudTextContext ctx;
} shudElementTargetName_t;

void* CG_SHUDElementTargetNameCreate(superhudConfig_t* config)
{
  shudElementTargetName_t *tn;

  tn = Z_Malloc(sizeof(*tn));
  OSP_MEMORY_CHECK(tn);

  memset(tn,0,sizeof(*tn));

  memcpy(&tn->config, config, sizeof(tn->config));

  NYAN_FLOAT(tn->config.rect.value[0]);

  CG_SHUDTextMakeContext(&tn->config, &tn->ctx);
  NYAN_FLOAT(tn->config.rect.value[0]);
  NYAN_FLOAT(tn->ctx.textX);
  tn->ctx.maxchars = 6;

  return tn;
}

void CG_SHUDElementTargetNameRoutine(void *context)
{
  shudElementTargetName_t *tn = (shudElementTargetName_t *)context;
	char    s[1024];
	int     fps_val;
	int     t;

	float* fade;
	float delta_time;
	clientInfo_t* ci;
	int w;
	int h;

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
  //NYAN_POINT();

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
	//tn->ctx.color[3] = fade[3];
  NYAN_MSG(s);
  NYAN_FLOAT(tn->ctx.textX);
  NYAN_FLOAT(tn->ctx.textY);
  NYAN_FLOAT(tn->ctx.fontW);
  NYAN_FLOAT(tn->ctx.fontH);
  NYAN_FLOAT(tn->ctx.fontIndex);

  CG_SHUDTextPrint(s, &tn->ctx);
}

void CG_SHUDElementTargetNameDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}

