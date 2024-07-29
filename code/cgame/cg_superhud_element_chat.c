#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
	superhudGlobalContext_t* gctx;
	int index;
} shudElementChat_t;

static void* CG_SHUDElementChatCreate(const superhudConfig_t* config, int line)
{
	shudElementChat_t* element;

	SHUD_ELEMENT_INIT(element, config);

	element->gctx = CG_SHUDGetContext();
	element->index = line;
	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.maxchars = MAX_SAY_TEXT;

	return element;
}

void* CG_SHUDElementChat1Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 1);
}

void* CG_SHUDElementChat2Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 2);
}

void* CG_SHUDElementChat3Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 3);
}

void* CG_SHUDElementChat4Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 4);
}

void* CG_SHUDElementChat5Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 5);
}

void* CG_SHUDElementChat6Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 6);
}

void* CG_SHUDElementChat7Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 7);
}

void* CG_SHUDElementChat8Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 8);
}

void* CG_SHUDElementChat9Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 9);
}

void* CG_SHUDElementChat10Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 10);
}

void* CG_SHUDElementChat11Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 11);
}

void* CG_SHUDElementChat12Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 12);
}

void* CG_SHUDElementChat13Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 13);
}

void* CG_SHUDElementChat14Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 14);
}

void* CG_SHUDElementChat15Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 15);
}

void* CG_SHUDElementChat16Create(const superhudConfig_t* config)
{
	return CG_SHUDElementChatCreate(config, 16);
}

void CG_SHUDElementChatRoutine(void* context)
{
	shudElementChat_t* element = (shudElementChat_t*)context;
	superhudChatEntry_t* entry;
	int index;

	index = ((element->gctx->chat.index - 1) - (element->index - 1)) % SHUD_MAX_CHAT_LINES;

	entry = &element->gctx->chat.line[index];

	if (entry->time == 0 || entry->message[0] == 0)
	{
		return;
	}

	if (!CG_SHUDGetFadeColor(element->ctx.color_origin, element->ctx.color, &element->config, entry->time))
	{
		entry->time = 0;
		return;
	}

	CG_SHUDFill(&element->config);

	element->ctx.text = entry->message;
	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementChatDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
