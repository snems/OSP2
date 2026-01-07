#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t lastLocation;
	int time;
} shudElementLocation_t;

void* CG_SHUDElementLocationCreate(const superhudConfig_t* config)
{
	shudElementLocation_t* element;

	SHUD_ELEMENT_INIT(element, config);

	if (!element->config.time.isSet)
	{
		element->config.time.isSet = qtrue;
		element->config.time.value = 2000;
	}

	CG_SHUDTextMakeContext(&element->config, &element->lastLocation);

	return element;
}

void CG_SHUDElementLocationRoutine(void* context)
{
	shudElementLocation_t* element = (shudElementLocation_t*)context;
	clientInfo_t* ci;
	const char* newLocation;

	ci = &cgs.clientinfo[cg.snap->ps.clientNum];

	if (customLocationsEnabled != 0)
	{
		newLocation = CG_CustomLocationsGetName(ci->customLocation);
	}
	else
	{
		newLocation = CG_ConfigString(CS_LOCATIONS + ci->location);
	}
	if (!newLocation || *newLocation == 0)
	{
		return;
	}

	if (element->lastLocation.text != newLocation && Q_stricmp(newLocation, "unknown"))
	{
		element->time = cg.time;
	}
	else if (!CG_SHUDGetFadeColor(element->lastLocation.color_origin, element->lastLocation.color, &element->config, element->time))
	{
		element->time = 0;
		return;
	}

	element->lastLocation.text = newLocation;

	CG_SHUDFill(&element->config);
	CG_SHUDTextPrint(&element->config, &element->lastLocation);
}

void CG_SHUDElementLocationDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}


