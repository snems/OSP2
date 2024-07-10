#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudDrawContext ctx;
  superhudTextContext tctx;
} shudElementNG_t;

void* CG_SHUDElementNGCreate(superhudConfig_t* config)
{
  shudElementNG_t *ng;

  ng = Z_Malloc(sizeof(*ng));
  OSP_MEMORY_CHECK(ng);

  memset(ng,0,sizeof(*ng));

  memcpy(&ng->config, config, sizeof(ng->config));

  CG_SHUDDrawMakeContext(&ng->config, &ng->ctx);

  if (config->image.isSet)
  {
    ng->ctx.image = trap_R_RegisterShader(ng->config.image.value);
  }

  CG_SHUDTextMakeContext(&ng->config, &ng->tctx);

  ng->ctx.x = 320;
  ng->ctx.y = 240;

  return ng;
}

void CG_SHUDElementNGRoutine(void *context)
{
  shudElementNG_t *ng = (shudElementNG_t*) context;
  float x,y,w,h;
  float ax,ay,aw,ah;
  int a;
  int i;
  int v;

	float   mid, range;
	int     color;
	float   vscale;

  x = ax = ng->config.rect.value[0];
  y = ay = ng->config.rect.value[1];
  w = aw = ng->config.rect.value[2];
  h = ah = ng->config.rect.value[3];

	trap_R_SetColor(NULL);

	CG_AdjustFrom640(&ax, &ay, &aw, &ah);
	if (ng->config.fill.isSet)
	{
	  if (ng->config.bgcolor.isSet)
	  {
	    trap_R_SetColor(ng->config.bgcolor.value);
	    trap_R_DrawStretchPic(ax, ay, aw, ah, 0, 0, 1, 1, cgs.media.whiteShader);
	    trap_R_SetColor(NULL);
	  }
	  else
	  {
	    trap_R_SetColor(ng->config.bgcolor.value);
	    trap_R_DrawStretchPic(ax, ay, aw, ah, 0, 0, 1, 1, cgs.media.whiteShader);
	    trap_R_SetColor(NULL);
	  }
	}
	else if (ng->config.image.isSet)
	{
	  trap_R_DrawStretchPic(ax, ay, aw, ah, 0, 0, 1, 1, ng->ctx.image);
	}
	else
	{
	  trap_R_DrawStretchPic(ax, ay, aw, ah, 0, 0, 1, 1, cgs.media.lagometerShader);
	}

  range = ah / 3;
  mid = ay + range;

  vscale = range / MAX_LAGOMETER_RANGE;

  // draw the frame interpoalte / extrapolate graph
  for (a = 0 ; a < aw ; a++)
  {
  	i = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
  	v = lagometer.frameSamples[i];
  	v *= vscale;
  	if (v > 0)
  	{
  		if (color != 1)
  		{
  			color = 1;
  			trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
  		}
  		if (v > range)
  		{
  			v = range;
  		}
  		trap_R_DrawStretchPic(ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
  	}
  	else if (v < 0)
  	{
  		if (color != 2)
  		{
  			color = 2;
  			trap_R_SetColor(g_color_table[ColorIndex(COLOR_BLUE)]);
  		}
  		v = -v;
  		if (v > range)
  		{
  			v = range;
  		}
  		trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
  	}
  }

  for (a = 0 ; a < aw ; a++)
  {
  	i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
  	v = lagometer.snapshotSamples[i];
  	if (v > 0)
  	{
  		if (lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED)
  		{
  			if (color != 5)
  			{
  				color = 5;  // YELLOW for rate delay
  				trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
  			}
  		}
  		else
  		{
  			if (color != 3)
  			{
  				color = 3;
  				trap_R_SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
  			}
  		}
  		v = v * vscale;
  		if (v > range)
  		{
  			v = range;
  		}
  		trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
  	}
  	else if (v < 0)
  	{
  		if (color != 4)
  		{
  			color = 4;      // RED for dropped snapshots
  			trap_R_SetColor(g_color_table[ColorIndex(COLOR_RED)]);
  		}
  		trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader);
  	}
  }

  trap_R_SetColor(NULL);

  {
	  int         cmdNum;
	  usercmd_t   cmd;

	  // draw the phone jack if we are completely past our buffers
	  cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	  trap_GetUserCmd(cmdNum, &cmd);
	  if (cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time)    // special check for map_restart // bk 0102165 - FIXME
	  {
	  	return ;
	  }

	  // also add text in center of screen
    CG_SHUDTextPrint("^1Connection Interrupted", &ng->tctx);

	  // blink the icon
	  if ((cg.time >> 9) & 1)
	  {
	  	return;
	  }

    trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 1, 1, trap_R_RegisterShader("gfx/2d/net.tga"));
	}

}

void CG_SHUDElementNGDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}



