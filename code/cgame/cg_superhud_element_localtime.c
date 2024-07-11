#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
	int timePrev;
	char s[MAX_QPATH];
  superhudTextContext ctx;
} shudElementLocalTime_t;

void* CG_SHUDElementLocalTimeCreate(superhudConfig_t* config)
{
  shudElementLocalTime_t *element;

  element = Z_Malloc(sizeof(*element));
  OSP_MEMORY_CHECK(element);

  memset(element,0,sizeof(*element));

  memcpy(&element->config, config, sizeof(element->config));

  CG_SHUDTextMakeContext(&element->config, &element->ctx);
  element->ctx.maxchars = 9;

  return element;
}

void CG_SHUDElementLocalTimeRoutine(void *context)
{
  shudElementLocalTime_t *element = (shudElementLocalTime_t *)context;
	qtime_t qtime;

  if (cg.time - element->timePrev > 1000)
  {
    element->timePrev = cg.time;
	  trap_RealTime(&qtime);
	  Com_sprintf(element->s, MAX_QPATH, "%02d%s%02d", qtime.tm_hour, (cg.time & 512) ? ":" : " ", qtime.tm_min);
  }

  CG_SHUDTextPrint(element->s, &element->ctx);
}

void CG_SHUDElementLocalTimeDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
