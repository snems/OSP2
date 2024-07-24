#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementFollowMessage_t;

void* CG_SHUDElementFollowMessageCreate(const superhudConfig_t* config)
{
	shudElementFollowMessage_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDFill(&element->config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementFollowMessageRoutine(void* context)
{
	shudElementFollowMessage_t* element = (shudElementFollowMessage_t*)context;
	const char* str;

	if (!cg.demoPlayback)
	{
		if (!(cg.snap->ps.pm_flags & PMF_FOLLOW))
		{
			return;
		}
		else if (cg.snap->ps.pm_type == PM_FREEZE || cg.intermissionStarted || cg.snap->ps.clientNum == cg.clientNum)
		{
			return;
		}
	}
	else if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_SPECTATOR)
	{
		return;
	}

	str = cgs.clientinfo[cg.snap->ps.clientNum].name;
	CG_SHUDTextPrint(va("Following %s", str), &element->ctx);
}

void CG_SHUDElementFollowMessageDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
