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
	shudElementNGP_t* ngp;

	ngp = Z_Malloc(sizeof(*ngp));
	OSP_MEMORY_CHECK(ngp);

	memset(ngp, 0, sizeof(*ngp));

	memcpy(&ngp->config, config, sizeof(ngp->config));

	CG_SHUDTextMakeContext(&ngp->config, &ngp->ctx);
	ngp->ctx.maxchars = MAX_QPATH;

	return ngp;
}

void CG_SHUDElementNGPRoutine(void* context)
{
	shudElementNGP_t* ngp = (shudElementNGP_t*)context;
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

	CG_SHUDTextPrint(va("%ims", ping), &ngp->ctx);
}

void CG_SHUDElementNGPDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}


