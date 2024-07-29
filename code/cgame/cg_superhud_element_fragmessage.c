#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
	int* time;
} shudElementFragMessage_t;

void* CG_SHUDElementFragMessageCreate(const superhudConfig_t* config)
{
	shudElementFragMessage_t* element;
	superhudGlobalContext_t* gctx;

	SHUD_ELEMENT_INIT(element, config);

	if (!element->config.time.isSet)
	{
		element->config.time.isSet = qtrue;
		element->config.time.value = 2000;
	}

	gctx = CG_SHUDGetContext();
	element->time = &gctx->fragmessage.time;

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.text = gctx->fragmessage.message;

	return element;
}

void CG_SHUDElementFragMessageRoutine(void* context)
{
	shudElementFragMessage_t* element = (shudElementFragMessage_t*)context;

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

void CG_SHUDElementFragMessageDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}


