#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

#define FPS_MAX_FRAMES  32
typedef struct
{
  superhudConfig_t config;
  float timeAverage;
  int framesNum;
	int timePrev;
  superhudTextContext ctx;
} shudElementFPS_t;

void* CG_SHUDElementFPSCreate(superhudConfig_t* config)
{
  shudElementFPS_t *fps;

  fps = Z_Malloc(sizeof(*fps));
  OSP_MEMORY_CHECK(fps);

  memset(fps,0,sizeof(*fps));

  memcpy(&fps->config, config, sizeof(fps->config));

  CG_SHUDTextMakeContext(&fps->config, &fps->ctx);
  fps->ctx.maxchars = 6;

  return fps;
}

void CG_SHUDElementFPSRoutine(void *context)
{
  shudElementFPS_t *fps = (shudElementFPS_t *)context;
	char*   s;
	int     fps_val;
	int     t;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	if (fps->timePrev == 0)
	{
	  // skip first measure result
    fps->timePrev = t;
	  return;
	}
	fps->timeAverage *= fps->framesNum;
	fps->timeAverage += t - fps->timePrev;
	fps->timeAverage /= ++fps->framesNum;
  fps->timePrev = t;

  if (fps->framesNum > FPS_MAX_FRAMES)
  {
    fps->framesNum = FPS_MAX_FRAMES;
  }

  fps_val = 1000.0f/fps->timeAverage;

	s = va("%ifps", fps_val);

  CG_SHUDTextPrint(s, &fps->ctx);
}

void CG_SHUDElementFPSDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
