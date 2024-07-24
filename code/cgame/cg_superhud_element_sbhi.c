#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarHealthIcon;

void* CG_SHUDElementSBHICreate(superhudConfig_t* config)
{
	shudElementStatusbarHealthIcon* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

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
	trap_R_SetColor(element->ctx.color);
	trap_R_DrawStretchPic(element->ctx.x, element->ctx.y, element->ctx.w, element->ctx.h, 0, 0, 1, 1, element->ctx.image);
	trap_R_SetColor(NULL);

}

void CG_SHUDElementSBHIDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
