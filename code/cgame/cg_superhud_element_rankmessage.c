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

void* CG_SHUDElementRankMessageCreate(const superhudConfig_t* config)
{
	shudElementRankMessage_t* element;
	superhudGlobalContext_t* gctx;

	SHUD_ELEMENT_INIT(element, config);

	if (!element->config.time.isSet)
	{
		element->config.time.isSet = qtrue;
		element->config.time.value = 2000;
	}

	gctx = CG_SHUDGetContext();

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	element->time = &gctx->rankmessage.time;
	element->ctx.text = gctx->rankmessage.message;

	return element;
}

void CG_SHUDElementRankMessageRoutine(void* context)
{
	shudElementRankMessage_t* element = (shudElementRankMessage_t*)context;

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

	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementRankMessageDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
