#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudTextContext ctx;
  int *time;
  const char *msg;
} shudElementFragMessage_t;

void* CG_SHUDElementFragMessageCreate(superhudConfig_t* config)
{
  shudElementFragMessage_t *fm;
	superhudGlobalContext_t *gctx;

  fm = Z_Malloc(sizeof(*fm));
  OSP_MEMORY_CHECK(fm);

  memset(fm,0,sizeof(*fm));

  memcpy(&fm->config, config, sizeof(fm->config));

  if (!config->time.isSet)
  {
  	config->time.isSet = qtrue;
  	config->time.value = 200;
  }

 	gctx = CG_SHUDGetContext();
	fm->time = &gctx->fragmessage.time;
	fm->msg = gctx->fragmessage.message;

  CG_SHUDTextMakeContext(&fm->config, &fm->ctx);

  return fm;
}

void CG_SHUDElementFragMessageRoutine(void *context)
{
  shudElementFragMessage_t *fm = (shudElementFragMessage_t *)context;
	char    s[1024];
	float* fade;
	clientInfo_t* ci;

	if (!*fm->time)
	{
		return;
	}

	if (cg.time - *fm->time > fm->config.time.value)
	{
		*fm->time = 0;
		return;
	}

	fade = CG_FadeColor(*fm->time, 1000);
	if (fade == NULL)
	{
		trap_R_SetColor(NULL);
		return;
	}

	fm->ctx.color[3] = fade[3];

  CG_SHUDTextPrint(fm->msg, &fm->ctx);
}

void CG_SHUDElementFragMessageDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}


