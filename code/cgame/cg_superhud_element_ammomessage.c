#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	int timePrev;
	char s[MAX_QPATH];
	superhudTextContext_t ctx;
} shudElementAmmoMessage_t;

void* CG_SHUDElementAmmoMessageCreate(superhudConfig_t* config)
{
	shudElementAmmoMessage_t* element;

	element = Z_Malloc(sizeof(*element));
	OSP_MEMORY_CHECK(element);

	memset(element, 0, sizeof(*element));

	memcpy(&element->config, config, sizeof(element->config));

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementAmmoMessageRoutine(void* context)
{
	shudElementAmmoMessage_t* element = (shudElementAmmoMessage_t*)context;
	const char*  s;
	int         w;

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
		s = "OUT OF AMMO";
	}
	else
	{
		s = "LOW AMMO WARNING";
	}

	CG_SHUDTextPrint(s, &element->ctx);
}

void CG_SHUDElementAmmoMessageDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}

