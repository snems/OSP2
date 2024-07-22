#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
	int* time;
	const char* msg;
} shudElementFragMessage_t;

void* CG_SHUDElementFragMessageCreate(superhudConfig_t* config)
{
	shudElementFragMessage_t* element;
	superhudGlobalContext_t* gctx;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	if (!config->time.isSet)
	{
		config->time.isSet = qtrue;
		config->time.value = 2000;
	}

	gctx = CG_SHUDGetContext();
	element->time = &gctx->fragmessage.time;
	element->msg = gctx->fragmessage.message;

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementFragMessageRoutine(void* context)
{
	shudElementFragMessage_t* element = (shudElementFragMessage_t*)context;
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

void CG_SHUDElementFragMessageDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}


