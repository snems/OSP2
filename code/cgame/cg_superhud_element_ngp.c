#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementNGP_t;

void* CG_SHUDElementNGPCreate(const superhudConfig_t* config)
{
	shudElementNGP_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	element->ctx.maxchars = MAX_QPATH;

	return element;
}

void CG_SHUDElementNGPRoutine(void* context)
{
	shudElementNGP_t* element = (shudElementNGP_t*)context;
	int ping;

	if (cg.demoPlayback) return;

	if (cgs.osp.pingCount == 0)
	{
		ping = 0;
	}
	else
	{
		ping = (int)(cgs.osp.pingMs);
	}

	CG_SHUDFill(&element->config);
	element->ctx.text = va("%ims", ping);
	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementNGPDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}


