#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementNGP_t;

void* CG_SHUDElementNGPCreate(superhudConfig_t* config)
{
	shudElementNGP_t* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

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
		ping = (int)((float)cgs.osp.pingTotalTime / (float)cgs.osp.pingCount);
	}

	CG_SHUDFill(&element->config);
	CG_SHUDTextPrint(va("%ims", ping), &element->ctx);
}

void CG_SHUDElementNGPDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}


