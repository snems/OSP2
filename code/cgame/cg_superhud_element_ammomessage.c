#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementAmmoMessage_t;

void* CG_SHUDElementAmmoMessageCreate(const superhudConfig_t* config)
{
	shudElementAmmoMessage_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementAmmoMessageRoutine(void* context)
{
	shudElementAmmoMessage_t* element = (shudElementAmmoMessage_t*)context;

	if (cg_drawAmmoWarning.integer == 0)
	{
		return;
	}

	if (!cg.lowAmmoWarning)
	{
		return;
	}

	if (cg.lowAmmoWarning == 2)
	{
		element->ctx.text = "OUT OF AMMO";
	}
	else
	{
		element->ctx.text = "LOW AMMO WARNING";
	}
	CG_SHUDFill(&element->config);

	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementAmmoMessageDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

