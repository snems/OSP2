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

	return sbai;
}

void CG_SHUDElementDecorRoutine(void* context)
{
	shudElementStatusbarDecorate* element = (shudElementStatusbarDecorate*)context;

	if (element->config.fill.isSet)
	{
		if (element->config.bgcolor.isSet)
		{
			CG_FillRect(element->ctx.x, element->ctx.y, element->ctx.w, element->ctx.h, element->config.bgcolor.value);
		}
		else if (element->ctx.image)
		{
			trap_R_SetColor(element->ctx.color);
			trap_R_DrawStretchPic(element->ctx.x, element->ctx.y, element->ctx.w, element->ctx.h, 0, 0, 0, 0, element->ctx.image);
			trap_R_SetColor(NULL);
		}
	}
	else if (element->ctx.image)
	{
		trap_R_SetColor(element->ctx.color);
		trap_R_DrawStretchPic(element->ctx.x, element->ctx.y, element->ctx.w, element->ctx.h, 0, 0, 1, 1, element->ctx.image);
		trap_R_SetColor(NULL);
	}
}

void CG_SHUDElementDecorDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
