#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudTextContext ctx;
  int *time;
  const char *msg;
} shudElementRankMessage_t;

void* CG_SHUDElementRankMessageCreate(superhudConfig_t* config)
{
  shudElementRankMessage_t *rm;
	superhudGlobalContext_t *gctx;

  rm = Z_Malloc(sizeof(*rm));
  OSP_MEMORY_CHECK(rm);

  memset(rm,0,sizeof(*rm));

  memcpy(&rm->config, config, sizeof(rm->config));

  if (!config->time.isSet)
  {
  	config->time.isSet = qtrue;
  	config->time.value = 2000;
  }

 	gctx = CG_SHUDGetContext();
	rm->time = &gctx->rankmessage.time;
	rm->msg = gctx->rankmessage.message;

  CG_SHUDTextMakeContext(&rm->config, &rm->ctx);

  return rm;
}

void CG_SHUDElementRankMessageRoutine(void *context)
{
  shudElementRankMessage_t *rm = (shudElementRankMessage_t *)context;
	char    s[1024];
	float* fade;
	clientInfo_t* ci;

	if (!*rm->time)
	{
		return;
	}

	if (cg.time - *rm->time > rm->config.time.value)
	{
		*rm->time = 0;
		return;
	}

	fade = CG_FadeColor(*rm->time, 1000);
	if (fade == NULL)
	{
		trap_R_SetColor(NULL);
		return;
	}

	rm->ctx.color[3] = fade[3];

  CG_SHUDTextPrint(rm->msg, &rm->ctx);
}

void CG_SHUDElementRankMessageDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
