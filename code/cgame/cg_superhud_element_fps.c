#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

#define FPS_MAX_FRAMES  4
typedef struct
{
	superhudConfig_t config;
	float timeAverage;
	int framesNum;
	int timePrev;
	superhudTextContext_t ctx;
} shudElementFPS_t;

void* CG_SHUDElementFPSCreate(const superhudConfig_t* config)
{
	shudElementFPS_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementFPSRoutine(void* context)
{
	shudElementFPS_t* element = (shudElementFPS_t*)context;
	float     fps_val;
	int     fps_val_int;
	int     t;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	if (element->timePrev == 0)
	{
		// skip first measure result
		element->timePrev = t;
		return;
	}
	element->timeAverage *= element->framesNum;
	element->timeAverage += t - element->timePrev;
	element->timeAverage /= ++element->framesNum;
	element->timePrev = t;

	if (element->framesNum > FPS_MAX_FRAMES)
	{
		element->framesNum = FPS_MAX_FRAMES;
	}

	fps_val = 1000.0f / element->timeAverage;
	fps_val_int = (int)fps_val;
	if (fps_val - (float)fps_val_int > 0.5f)
	{
		++fps_val_int;
	}

	element->ctx.text = va("%ifps", fps_val_int);

	CG_SHUDFill(&element->config);

	CG_SHUDTextPrint(&element->config, &element->ctx);
}

void CG_SHUDElementFPSDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
