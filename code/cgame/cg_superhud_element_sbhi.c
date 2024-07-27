#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarHealthIcon;

void* CG_SHUDElementSBHICreate(const superhudConfig_t* config)
{
	shudElementStatusbarHealthIcon* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	if (config->image.isSet)
	{
		element->ctx.image = trap_R_RegisterShader(element->config.image.value);
	}

	if (!element->ctx.image)
	{
		element->ctx.image = trap_R_RegisterShader("icons/iconh_yellow.tga");
	}

	return element;
}

void CG_SHUDElementSBHIRoutine(void* context)
{
	shudElementStatusbarHealthIcon* element = (shudElementStatusbarHealthIcon*)context;
	CG_SHUDFill(&element->config);
	CG_SHUDDrawStretchPicCtx(&element->config, &element->ctx);
}

void CG_SHUDElementSBHIDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
