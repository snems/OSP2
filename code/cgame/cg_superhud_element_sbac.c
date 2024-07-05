
#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudTextContext position;
} shudElementStatusbarHealthCount;

void* CG_SHUDElementSBACCreate(superhudConfig_t* config)
{
  shudElementStatusbarHealthCount *sbac;

  sbac = Z_Malloc(sizeof(*sbac));
  OSP_MEMORY_CHECK(sbac);

  memset(sbac,0,sizeof(*sbac));

  memcpy(&sbac->config, config, sizeof(sbac->config));

  //load defaults
  if (!sbac->config.color.isSet)
  {
    sbac->config.color.isSet = qtrue;
    sbac->config.color.value.type = SUPERHUD_COLOR_RGBA;
    Vector4Set(sbac->config.color.value.rgba, 1, 0.7, 0, 1);
  }

  CG_SHUDTextMakeContext(&sbac->config, &sbac->position);
  sbac->position.maxchars = 6;

  return sbac;
}

void CG_SHUDElementSBACRoutine(void *context)
{
  shudElementStatusbarHealthCount *sbac = (shudElementStatusbarHealthCount *)context;
  const char *s;

	playerState_t* ps = &cg.snap->ps;

	s = va("^X000000%i", ps->stats[STAT_ARMOR]);

  CG_SHUDTextPrint(s, &sbac->position);
}

void CG_SHUDElementSBACDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
