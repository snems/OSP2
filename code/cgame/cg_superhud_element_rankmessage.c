#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
	int* time;
	const char* msg;
} shudElementRankMessage_t;

void* CG_SHUDElementRankMessageCreate(superhudConfig_t* config)
{
	shudElementRankMessage_t* rm;
	superhudGlobalContext_t* gctx;

	rm = Z_Malloc(sizeof(*rm));
	OSP_MEMORY_CHECK(rm);

	memset(rm, 0, sizeof(*rm));

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

void CG_SHUDElementRankMessageRoutine(void* context)
{
	shudElementRankMessage_t* element = (shudElementRankMessage_t*)context;
	char    s[1024];
	float* fade;
	clientInfo_t* ci;

	if (!*element->time)
	{
		return;
	}

	if (!CG_SHUDGetFadeColor(element->ctx.color_origin, element->ctx.color, &element->config, *element->time))
	{
		*element->time = 0;
		return;
	}

	CG_SHUDFill(&element->config);

	CG_SHUDTextPrint(element->msg, &element->ctx);
}

void CG_SHUDElementRankMessageDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
