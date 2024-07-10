#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  float timeAverage;
  int framesNum;
	int timePrev;
  superhudTextContext ctx;
} shudElementPlayerSpeed_t;

void* CG_SHUDElementPlayerSpeedCreate(superhudConfig_t* config)
{
  shudElementPlayerSpeed_t *ps;

  ps = Z_Malloc(sizeof(*ps));
  OSP_MEMORY_CHECK(ps);

  memset(ps,0,sizeof(*ps));

  memcpy(&ps->config, config, sizeof(ps->config));

  CG_SHUDTextMakeContext(&ps->config, &ps->ctx);
  ps->ctx.maxchars = 9;

  return ps;
}

void CG_SHUDElementPlayerSpeedRoutine(void *context)
{
  shudElementPlayerSpeed_t *ps = (shudElementPlayerSpeed_t *)context;

  CG_SHUDTextPrint(va("%dups", (int)cg.xyspeed), &ps->ctx);
}

void CG_SHUDElementPlayerSpeedDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
