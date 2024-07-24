#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarArmorIcon;

void* CG_SHUDElementSBAICreate(const superhudConfig_t* config)
{
	shudElementStatusbarArmorIcon* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	if (config->image.isSet)
	{
		element->ctx.image = trap_R_RegisterShader(element->config.image.value);
	}

	return element;
}

void CG_SHUDElementSBAIRoutine(void* context)
{
	shudElementStatusbarArmorIcon* element = (shudElementStatusbarArmorIcon*)context;
	CG_SHUDFill(&element->config);
	CG_SHUDDrawStretchPicCtx(&element->ctx);
}

void CG_SHUDElementSBAIDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
