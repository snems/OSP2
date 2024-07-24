#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarDecorate;

void* CG_SHUDElementDecorCreate(superhudConfig_t* config)
{
	shudElementStatusbarDecorate* sbai;

	sbai = Z_Malloc(sizeof(*sbai));
	OSP_MEMORY_CHECK(sbai);

	memset(sbai, 0, sizeof(*sbai));

	memcpy(&sbai->config, config, sizeof(sbai->config));

	CG_SHUDDrawMakeContext(&sbai->config, &sbai->ctx);

	if (config->image.isSet)
	{
		sbai->ctx.image = trap_R_RegisterShader(sbai->config.image.value);
		if (!sbai->ctx.image)
		{
			CG_Printf("^2Decorate image %s is not found\n", sbai->config.image.value);
		}
	}

	sbai->ctx.coordPicture.arr[0] = 0;
	sbai->ctx.coordPicture.arr[1] = 0;
	sbai->ctx.coordPicture.arr[2] = 0;
	sbai->ctx.coordPicture.arr[3] = 0;

	return sbai;
}

void CG_SHUDElementDecorRoutine(void* context)
{
	shudElementStatusbarDecorate* element = (shudElementStatusbarDecorate*)context;

	if (!CG_SHUDFill(&element->config) && element->ctx.image)
	{
	  CG_SHUDDrawStretchPicCtx(&element->ctx);
	}
}

void CG_SHUDElementDecorDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

