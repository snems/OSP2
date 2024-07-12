
#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudTextContext position;
} shudElementStatusbarHealthCount;

void* CG_SHUDElementSBHCCreate(superhudConfig_t* config)
{
  shudElementStatusbarHealthCount *sbhc;

  sbhc = Z_Malloc(sizeof(*sbhc));
  OSP_MEMORY_CHECK(sbhc);

  memset(sbhc,0,sizeof(*sbhc));

  memcpy(&sbhc->config, config, sizeof(sbhc->config));

  //load defaults
  if (!sbhc->config.color.isSet)
  {
    sbhc->config.color.isSet = qtrue;
    sbhc->config.color.value.type = SUPERHUD_COLOR_RGBA;
    Vector4Set(sbhc->config.color.value.rgba, 1, 0.7, 0, 1);
  }
  if (!sbhc->config.text.isSet)
  {
    sbhc->config.text.isSet = qtrue;
    Q_strncpyz(sbhc->config.text.value, "%i", sizeof(sbhc->config.text.value));
  }

  CG_SHUDTextMakeContext(&sbhc->config, &sbhc->position);
  sbhc->position.maxchars = 6;

  return sbhc;
}

void CG_SHUDElementSBHCRoutine(void *context)
{
  shudElementStatusbarHealthCount *sbhc = (shudElementStatusbarHealthCount *)context;
  const char *s;
  int hp = cg.snap->ps.stats[STAT_HEALTH];

	s = va(sbhc->config.text.value, hp > 0 ? hp : 0);

  CG_SHUDTextPrint(s, &sbhc->position);
}

void CG_SHUDElementSBHCDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
