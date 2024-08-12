#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementSpecMessage_t;

void* CG_SHUDElementSpecMessageCreate(const superhudConfig_t* config)
{
	shudElementSpecMessage_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	element->ctx.text = "^1SPECTATOR";
	return element;
}

void CG_SHUDElementSpecMessageRoutine(void* context)
{
	shudElementSpecMessage_t* element = (shudElementSpecMessage_t*)context;

	if (CG_IsSpectator())
	{
		CG_SHUDFill(&element->config);
		CG_SHUDTextPrint(&element->config, &element->ctx);
	}
}

void CG_SHUDElementSpecMessageDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
