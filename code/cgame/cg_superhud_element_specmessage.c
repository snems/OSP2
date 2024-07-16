#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudTextContext_t ctx;
} shudElementSpecMessage_t;

void* CG_SHUDElementSpecMessageCreate(superhudConfig_t* config)
{
  shudElementSpecMessage_t *element;

  element = Z_Malloc(sizeof(*element));
  OSP_MEMORY_CHECK(element);

  memset(element,0,sizeof(*element));

  memcpy(&element->config, config, sizeof(element->config));

  CG_SHUDTextMakeContext(&element->config, &element->ctx);

  return element;
}

void CG_SHUDElementSpecMessageRoutine(void *context)
{
  shudElementSpecMessage_t *element = (shudElementSpecMessage_t *)context;

  if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
  {
    CG_SHUDTextPrint("SPECTATOR", &element->ctx);
  }
}

void CG_SHUDElementSpecMessageDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
