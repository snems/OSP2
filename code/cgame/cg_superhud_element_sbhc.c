
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

  CG_SHUDTextMakeContext(&sbhc->config, &sbhc->position);
  sbhc->position.maxchars = 6;

  return sbhc;
}

void CG_SHUDElementSBHCRoutine(void *context)
{
  shudElementStatusbarHealthCount *sbhc = (shudElementStatusbarHealthCount *)context;
  const char *s;
	playerState_t* ps = &cg.snap->ps;

	s = va("%i", ps->stats[STAT_HEALTH]);

  CG_SHUDTextPrint(s, &sbhc->position);
}

void CG_SHUDElementSBHCDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
