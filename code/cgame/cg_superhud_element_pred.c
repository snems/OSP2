#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarDecorate;

void* CG_SHUDElementDecorCreate(const superhudConfig_t* config)
{
	shudElementStatusbarDecorate* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	if (config->image.isSet)
	{
		element->ctx.image = trap_R_RegisterShader(element->config.image.value);
		if (!element->ctx.image)
		{
			CG_Printf("^2Decorate image %s is not found\n", element->config.image.value);
		}
	}

	return element;
}

void CG_SHUDElementDecorRoutine(void* context)
{
	shudElementStatusbarDecorate* element = (shudElementStatusbarDecorate*)context;

	if (!CG_SHUDFill(&element->config) && element->ctx.image)
	{
		CG_SHUDDrawStretchPicCtx(&element->config, &element->ctx);
	}
}

void CG_SHUDElementDecorDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

