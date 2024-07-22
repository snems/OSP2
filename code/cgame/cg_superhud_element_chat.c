
#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
	superhudGlobalContext_t* gctx;
	int index;
} shudElementChat_t;

static void* CG_SHUDElementChatCreate(superhudConfig_t* config, int line)
{
	shudElementChat_t* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	element->gctx = CG_SHUDGetContext();
	element->index = line;
	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.maxchars = MAX_SAY_TEXT;

	return element;
}

void* CG_SHUDElementChat1Create(superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 1);
}

void* CG_SHUDElementChat2Create(superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 2);
}

void* CG_SHUDElementChat3Create(superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 3);
}

void* CG_SHUDElementChat4Create(superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 4);
}

void* CG_SHUDElementChat5Create(superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 5);
}

void* CG_SHUDElementChat6Create(superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 6);
}

void* CG_SHUDElementChat7Create(superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 7);
}

void* CG_SHUDElementChat8Create(superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 8);
}

void CG_SHUDElementChatRoutine(void* context)
{
	shudElementChat_t* element = (shudElementChat_t*)context;
	float* fade;
	int fadedelay;
	superhudChatEntry_t* entry;
	int index;

	index = ((element->gctx->chat.index - 1) - (element->index - 1)) % SHUD_MAX_CHAT_LINES;

	entry = &element->gctx->chat.line[index];

	if (entry->message[0] == 0)
	{
		return;
	}

	if (!CG_SHUDGetFadeColor(element->ctx.color_origin, element->ctx.color, &element->config, entry->time))
	{
		return;
	}

	CG_SHUDTextPrint(entry->message, &element->ctx);
}

void CG_SHUDElementChatDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
