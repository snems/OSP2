#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarAmmoIcon;

void* CG_SHUDElementSBAmICreate(const superhudConfig_t* config)
{
	shudElementStatusbarAmmoIcon* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	if (config->image.isSet)
	{
		element->ctx.image = trap_R_RegisterShader(element->config.image.value);
	}

	return element;
}

void CG_SHUDElementSBAmIRoutine(void* context)
{
	shudElementStatusbarAmmoIcon* element = (shudElementStatusbarAmmoIcon*)context;
	int wp;

	if (!cg.snap)
	{
		return;
	}

	wp = cg.snap->ps.weapon;

	if (wp == WP_NONE) return;

	if ((element->ctx.image = cg_weapons[wp].ammoIcon))
	{
		CG_SHUDFill(&element->config);
		CG_SHUDDrawStretchPicCtx(&element->config, &element->ctx);
	}

}

void CG_SHUDElementSBAmIDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
