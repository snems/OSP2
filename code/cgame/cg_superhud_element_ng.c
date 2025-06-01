#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
	superhudTextContext_t tctx;
} shudElementNG_t;

void* CG_SHUDElementNGCreate(const superhudConfig_t* config)
{
	shudElementNG_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	if (config->image.isSet)
	{
		element->ctx.image = trap_R_RegisterShader(element->config.image.value);
	}

	CG_SHUDTextMakeContext(&element->config, &element->tctx);

	element->tctx.coord.named.x = 320;
	element->tctx.coord.named.y = 232;

	element->tctx.coord.named.h = 16;
	element->tctx.coord.named.w = 12;

	element->tctx.flags |= DS_HCENTER;
	element->tctx.text = "^1Connection Interrupted";

	Vector4Set(element->tctx.color, 1.0f, 1.0f, 1.0f, 1.0f);

	return element;
}

void CG_SHUDElementNGRoutine(void* context)
{
	shudElementNG_t* element = (shudElementNG_t*) context;
	float ax, ay, aw, ah;
	int a;
	int i;
	int v;

	float   mid, range;
	int     color = 0;
	float   vscale;

	ax = element->config.rect.value[0];
	ay = element->config.rect.value[1];
	aw = element->config.rect.value[2];
	ah = element->config.rect.value[3];

	CG_SHUDFill(&element->config);

	trap_R_SetColor(NULL);

	CG_AdjustFrom640(&ax, &ay, &aw, &ah);
	if (element->config.fill.isSet)
	{
		if (element->config.bgcolor.isSet)
		{
			trap_R_SetColor(element->config.bgcolor.value);
			trap_R_DrawStretchPic(ax, ay, aw, ah, 0, 0, 1, 1, cgs.media.whiteShader);
			trap_R_SetColor(NULL);
		}
	}
	else if (element->config.image.isSet)
	{
		trap_R_DrawStretchPic(ax, ay, aw, ah, 0, 0, 1, 1, element->ctx.image);
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
		static int  delay = 0;

		// draw the phone jack if we are completely past our buffers
		cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
		trap_GetUserCmd(cmdNum, &cmd);
		if (cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time)    // special check for map_restart // bk 0102165 - FIXME
		{
			delay = cg.time;
			return ;
		}

		if (delay + 500 > cg.time)
		{
			return;
		}

		// also add text in center of screen
		CG_SHUDTextPrint(&element->config, &element->tctx);

		// blink the icon
		if ((cg.time >> 9) & 1)
		{
			return;
		}

		trap_R_DrawStretchPic(ax, ay, aw, ah, 0, 0, 1, 1, cgs.media.connectionShader);
	}
}

void CG_SHUDElementNGDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}



