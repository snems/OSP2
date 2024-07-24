#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementStatusbarAmmoIcon;

void* CG_SHUDElementSBAmICreate(superhudConfig_t* config)
{
	shudElementStatusbarAmmoIcon* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

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
	int wp = cg.weaponSelect;

	if (wp == WP_NONE) return;

	if ((element->ctx.image = cg_weapons[wp].ammoIcon))
	{
		CG_SHUDFill(&element->config);
	  CG_SHUDDrawStretchPicCtx(&element->ctx);
	}

}

void CG_SHUDElementSBAmIDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
