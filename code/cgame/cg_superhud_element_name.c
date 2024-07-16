#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

enum shudElementNameType_t
{
  SHUDENAME_TYPE_OWN,
  SHUDENAME_TYPE_NME,
};

typedef struct
{
  superhudConfig_t config;
  superhudTextContext_t ctx;
  enum shudElementNameType_t type;
} shudElementName_t;

static void* CG_SHUDElementNameCreate(superhudConfig_t* config, enum shudElementNameType_t type)
{
  shudElementName_t *element;

  element = Z_Malloc(sizeof(*element));
  OSP_MEMORY_CHECK(element);

  memset(element,0,sizeof(*element));

  memcpy(&element->config, config, sizeof(element->config));

  element->type = type;

  CG_SHUDTextMakeContext(&element->config, &element->ctx);
  element->ctx.maxchars = MAX_QPATH;

  return element;
}

void* CG_SHUDElementNameOWNCreate(superhudConfig_t* config)
{
  return CG_SHUDElementNameCreate(config, SHUDENAME_TYPE_OWN);
}

void* CG_SHUDElementNameNMECreate(superhudConfig_t* config)
{
  return CG_SHUDElementNameCreate(config, SHUDENAME_TYPE_NME);
}

static const char* CG_SHUDElementNameGetSelf(void)
{
	return cgs.clientinfo[cg.snap->ps.clientNum].name;
}

static const char* CG_SHUDElementNameGetLastAttacker(void)
{
  return NULL;
}

static const char* CG_SHUDElementNameGetEnemy1vs1(void)
{

  return NULL;
}

void CG_SHUDElementNameRoutine(void *context)
{
  shudElementName_t *element = (shudElementName_t *)context;
  const float *fade;
	int         t;
	const char*  info;
	const char*  name;
	int         clientNum;

  if (element->type == SHUDENAME_TYPE_OWN)
  {
    name = CG_SHUDElementNameGetSelf();
  }
  else if (cgs.gametype == GT_TOURNAMENT) 
  {
    name = CG_SHUDElementNameGetEnemy1vs1();
  }
  else
  {
    name = CG_SHUDElementNameGetLastAttacker();
  }

  if (name)
  {
    CG_SHUDTextPrint(name, &element->ctx);
  }
}

void CG_SHUDElementNameDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
