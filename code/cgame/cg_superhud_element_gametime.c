#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
	int timePrev;
  superhudTextContext_t ctx;
} shudElementGameTime_t;

void* CG_SHUDElementGameTimeCreate(superhudConfig_t* config)
{
  shudElementGameTime_t *element;

  element = Z_Malloc(sizeof(*element));
  OSP_MEMORY_CHECK(element);

  memset(element,0,sizeof(*element));

  memcpy(&element->config, config, sizeof(element->config));

  CG_SHUDTextMakeContext(&element->config, &element->ctx);
  element->ctx.maxchars = 9;

  return element;
}

void CG_SHUDElementGameTimeRoutine(void *context)
{
  shudElementGameTime_t *element = (shudElementGameTime_t *)context;


	if (cg_drawTimer.integer)
	{
	  int         mins, seconds, tens;
	  int         msec;
	  msec = cg.time - cgs.levelStartTime;

	  seconds = msec / 1000;
	  mins = seconds / 60;
	  seconds -= mins * 60;
	  tens = seconds / 10;
	  seconds -= tens * 10;

    CG_SHUDTextPrint(va("%i:%i%i", mins, tens, seconds), &element->ctx);
	}

}

void CG_SHUDElementGameTimeDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}

