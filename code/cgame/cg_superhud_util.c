#include "./cg_superhud_private.h"
#include "cg_local.h"

typedef struct
{
	vec4_t bar1_value;
	vec4_t bar1_bottom;
	vec4_t bar2_value;
	vec4_t bar2_bottom;
} drawBarCoords_t;


static void CG_SHUDConfigPickColor(const superhudConfig_t* config, float* color, qboolean alphaOverride)
{
	const superhudColor_t* in = &config->color.value;
	const float* target = colorWhite;
	team_t team;

	if (!config->color.isSet)
	{
		if (alphaOverride)
		{
			Vector4Copy(colorWhite, color);
		}
		else
		{
			VectorCopy(colorWhite, color);
		}
		return;
	}

	switch (in->type)
	{
		case SUPERHUD_COLOR_RGBA:
			target = in->rgba;
			break;
		case SUPERHUD_COLOR_T:
			team = CG_SHUDGetOurActiveTeam();
			if (team == TEAM_RED)
			{
				target = colorRed;
				break;
			}
			else if (team == TEAM_BLUE)
			{
				target = colorBlue;
				break;
			}
			target = colorRed;
			break;
		case SUPERHUD_COLOR_E:
			team = CG_SHUDGetOurActiveTeam();
			if (team == TEAM_RED)
			{
				target = colorBlue;
				break;
			}
			else if (team == TEAM_BLUE)
			{
				target = colorRed;
				break;
			}
			target = colorBlue;
			break;
		case SUPERHUD_COLOR_I:
			target = colorWhite;
			break;
	}
	if (alphaOverride)
	{
		Vector4Copy(target, color);
	}
	else
	{
		VectorCopy(target, color);
	}
}

static void CG_SHUDConfigDefaultsCheck(superhudConfig_t* config)
{
	if (!config->rect.isSet)
	{
		config->rect.value[0] = 0.0f;
		config->rect.value[1] = 0.0f;
		config->rect.value[2] = 320.0f;
		config->rect.value[3] = 12.0f;
		config->rect.isSet = qtrue;
	}

	if (!config->textAlign.isSet)
	{
		config->textAlign.value = SUPERHUD_ALIGNH_LEFT;
		config->textAlign.isSet = qtrue;
	}

	/* if fontsize wasn't set, find max size for rect */
	if (!config->fontsize.isSet)
	{
		config->fontsize.value[1] = config->rect.value[3];
		config->fontsize.value[0] = config->fontsize.value[1] / 1.618f;
		config->fontsize.isSet = qtrue;
	}

	if (!config->textStyle.isSet)
	{
		config->textStyle.value = 0; //drop shadow
		config->textStyle.isSet = qtrue;
	}

	if (!config->color.isSet)
	{
		Vector4Copy(colorWhite, config->color.value.rgba);
		config->color.value.type = SUPERHUD_COLOR_RGBA;
		config->color.isSet = qtrue;
	}
}

static void CG_SHUDTextMakeAdjustCoords(const superhudConfig_t* in, float* out_x, float* out_y)
{
	superhudAlignH_t h;
	superhudAlignV_t v;

	if (!in->rect.isSet)
	{
		return;
	}

	if (!in->alignH.isSet)
	{
		h = SUPERHUD_ALIGNH_LEFT;
	}
	else
	{
		h = in->alignH.value;
	}

	if (!in->alignV.isSet)
	{
		v = SUPERHUD_ALIGNV_CENTER;
	}
	else
	{
		v = in->alignV.value;
	}

	switch (h)
	{
		case SUPERHUD_ALIGNH_LEFT:
			//allready x
			*out_x = in->rect.value[0];
			break;
		case SUPERHUD_ALIGNH_CENTER:
			// x + width/2
			*out_x = in->rect.value[0] + in->rect.value[2] / 2.0f;
			break;
		case SUPERHUD_ALIGNH_RIGHT:
			// x + width
			*out_x = in->rect.value[0] + in->rect.value[2];
			break;
	}

	switch (v)
	{
		case SUPERHUD_ALIGNV_TOP:
			*out_y = in->rect.value[1];
			break;
		case SUPERHUD_ALIGNV_CENTER:
			*out_y = in->rect.value[1] + in->rect.value[3] / 2.0f;
			break;
		case SUPERHUD_ALIGNV_BOTTOM:
			*out_y = in->rect.value[1] + in->rect.value[3];
			break;
	}

}

void CG_SHUDTextMakeContext(const superhudConfig_t* in, superhudTextContext_t* out)
{
	superhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_SHUDConfigDefaultsCheck(&config);

	CG_SHUDTextMakeAdjustCoords(in, &out->coord.named.x, &out->coord.named.y);

	switch (config.textAlign.value)
	{
		default:
		case SUPERHUD_ALIGNH_LEFT:
			out->flags |= DS_HLEFT;
			break;
		case SUPERHUD_ALIGNH_CENTER:
			out->flags |= DS_HCENTER;
			break;
		case SUPERHUD_ALIGNH_RIGHT:
			out->flags |= DS_HRIGHT;
			break;
	}

	out->coord.named.w = config.fontsize.value[0];
	out->coord.named.h = config.fontsize.value[1];

	if (config.alignV.isSet)
	{
		switch (config.alignV.value)
		{
			default:
			case SUPERHUD_ALIGNV_TOP:
				out->flags |= DS_VTOP;
				break;
			case SUPERHUD_ALIGNV_CENTER:
				out->flags |= DS_VCENTER;
				break;
			case SUPERHUD_ALIGNV_BOTTOM:
				out->flags |= DS_VBOTTOM;
				break;
		}
	}

	if (!config.monospace.isSet)
	{
		out->flags |= DS_PROPORTIONAL;
	}
	if (config.textStyle.value & 1)
	{
		out->flags |= DS_SHADOW;
	}

	out->fontIndex = CG_FontIndexFromName(config.font.isSet ? config.font.value : "sansman");
	out->width = SCREEN_WIDTH;

	CG_SHUDConfigPickColor(&config, out->color, qtrue);
	Vector4Copy(out->color, out->color_origin);
}

void CG_SHUDDrawMakeContext(const superhudConfig_t* in, superhudDrawContext_t* out)
{
	superhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_SHUDConfigDefaultsCheck(&config);

	out->coord.named.x = config.rect.value[0];
	out->coord.named.y = config.rect.value[1];
	out->coord.named.w = config.rect.value[2];
	out->coord.named.h = config.rect.value[3];

	out->coordPicture.named.x = 0.0f;
	out->coordPicture.named.y = 0.0f;
	out->coordPicture.named.w = 1.0f;
	out->coordPicture.named.h = 1.0f;

	CG_SHUDConfigPickColor(&config, out->color, qtrue);
	Vector4Copy(out->color, out->color_origin);
}

void CG_SHUDBarMakeContext(const superhudConfig_t* in, superhudBarContext_t* out, float max)
{
	float x = 0, y = 0;
	float bar_height, bar_width;
	superhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_SHUDConfigDefaultsCheck(&config);

	if (!config.direction.isSet)
	{
		config.direction.isSet = qtrue;
		config.direction.value = SUPERHUD_DIR_LEFT_TO_RIGHT;
	}

	if (!config.style.isSet) // set default style
	{
		config.style.isSet = qtrue;
		config.style.value = 1;
	}
	else if (config.style.value < 1 || config.style.value > 2)
	{
		config.style.value = 1;
	}

	out->direction = config.direction.value;

	x = config.rect.value[0];
	y = config.rect.value[1];

	if (config.doublebar.isSet)
	{
		static const float bar_gap = 4;
		out->two_bars = qtrue;
		if (out->direction == SUPERHUD_DIR_LEFT_TO_RIGHT || out->direction == SUPERHUD_DIR_RIGHT_TO_LEFT)
		{
			if (config.style.value == 1)    // style 1 - default: split into two bars
			{

				bar_height = config.rect.value[3] / 2 - bar_gap / 2; //split horizontal
				bar_width = config.rect.value[2];

				out->bar[0][0] = x;//x
				out->bar[0][1] = y;//y
				out->bar[0][2] = bar_width;//w
				out->bar[0][3] = bar_height;   // height is half of rect and minus half of the gap between two bars
				//
				out->bar[1][0] = x;//x
				out->bar[1][1] = y + bar_height + bar_gap;
				out->bar[1][2] = bar_width;//w
				out->bar[1][3] = bar_height;//h height is same as in first bar
			}
			else if (config.style.value == 2) // style 2 - same start point for both bars
			{
				// all the same for same bars
				out->bar[1][0] = out->bar[0][0] = x; // x
				out->bar[1][1] = out->bar[0][1] = y; // y
				out->bar[1][2] = out->bar[0][2] = config.rect.value[2]; // w
				out->bar[1][3] = out->bar[0][3] = config.rect.value[3]; // h
			}
			CG_AdjustFrom640(&out->bar[1][0], &out->bar[1][1], &out->bar[1][2], &out->bar[1][3]);
			CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
			out->max = out->bar[1][2];
			out->koeff = 2 * out->bar[1][2] / max;
		}
		else
		{
			if (config.style.value == 1)    // style 1 - default: split into two bars
			{
				bar_height = config.rect.value[3];
				bar_width = config.rect.value[2] / 2 - bar_gap / 2;

				out->bar[0][0] = x;//x
				out->bar[0][1] = y;//y
				out->bar[0][2] = bar_width;//w
				out->bar[0][3] = bar_height;   //h
				//
				out->bar[1][0] = x + bar_width + bar_gap;
				out->bar[1][1] = y;//y
				out->bar[1][2] = bar_width;//w
				out->bar[1][3] = bar_height;//h
			}
			else if (config.style.value == 2)   // style 2 - same start point for both bars
			{
				// all the same for same bars
				out->bar[1][0] = out->bar[0][0] = x; // x
				out->bar[1][1] = out->bar[0][1] = y; // y
				out->bar[1][2] = out->bar[0][2] = config.rect.value[2]; // w
				out->bar[1][3] = out->bar[0][3] = config.rect.value[3]; // h
			}
			CG_AdjustFrom640(&out->bar[1][0], &out->bar[1][1], &out->bar[1][2], &out->bar[1][3]);
			CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
			out->max = out->bar[1][3];
			out->koeff = 2 * out->bar[1][3] / max;
		}
	}
	else
	{
		// single bar
		out->bar[0][0] = x;
		out->bar[0][1] = y;
		out->bar[0][2] = config.rect.value[2];
		out->bar[0][3] = config.rect.value[3];
		CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
		if (out->direction == SUPERHUD_DIR_LEFT_TO_RIGHT || out->direction == SUPERHUD_DIR_RIGHT_TO_LEFT)
		{
			out->max = out->bar[0][2]; // max / width
		}
		else
		{
			out->max = out->bar[0][3]; // max / height
		}
		out->koeff = out->max / max;
	}

	CG_SHUDConfigPickColor(&config, out->color_top, qtrue);
	if (config.bgcolor.isSet)
	{
		Vector4Copy(config.bgcolor.value, out->color_back);
	}
	else
	{
		Vector4Set(out->color_back, 0, 0, 0, 0);
	}
}

qboolean CG_SHUDIsTimeOut(const superhudConfig_t* cfg, int startTime)
{
	if (!startTime)
	{
		return qtrue;
	}
	if (cfg->time.isSet)
	{
		if ((cg.time - startTime) < cfg->time.value)
		{
			//fade time is not started;
			return qfalse;
		}
	}
	return qtrue;
}

/*
   *  Затухание
   *  возвращает qfalse если элемент потух
   */
qboolean CG_SHUDGetFadeColor(const vec4_t from_color, vec4_t out, const superhudConfig_t* cfg, int startTime)
{
	int time = 0;

	Vector4Copy(from_color, out);

	if (!CG_SHUDIsTimeOut(cfg, startTime))
	{
		return qtrue;
	}

	if (cfg->time.isSet)
	{
		time = cfg->time.value;
	}

	if (cfg->fade.isSet)
	{
		int fadetime;
		float fadedelay = SUPERHUD_DEFAULT_FADEDELAY;

		fadetime = cg.time - startTime - time;

		if (cfg->fadedelay.isSet)
		{
			fadedelay = (float)cfg->fadedelay.value;
		}

		if (fadetime > 0 && fadetime < fadedelay)
		{
			vec4_t tmpfade;
			float k = (float)fadetime / fadedelay;
			Vector4Copy(cfg->fade.value, tmpfade);
			Vector4Subtract(tmpfade, from_color, tmpfade);
			Vector4MA(from_color, k, tmpfade, out);
			return qtrue;
		}
		else
		{
			return qfalse;
		}
	}

	return qfalse;
}

void CG_SHUDTextPrint(const superhudConfig_t* cfg, superhudTextContext_t* ctx)
{
	if (!ctx->text || !ctx->text[0])
	{
		return;
	}

	CG_SHUDConfigPickColor(cfg, ctx->color, qfalse);

	CG_FontSelect(ctx->fontIndex);
	CG_OSPDrawString(ctx->coord.named.x,
	                 ctx->coord.named.y,
	                 ctx->text,
	                 ctx->color,
	                 ctx->coord.named.w,
	                 ctx->coord.named.h,
	                 ctx->width,
	                 ctx->flags,
	                 ctx->background);
}


static void CG_SHUDBarPreparePrintLTR(const superhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	if (ctx->two_bars)
	{
		float width;

		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		Vector4Copy(ctx->bar[1], coords->bar2_value);
		Vector4Copy(ctx->bar[1], coords->bar2_bottom);
		//bar1
		width = value * ctx->koeff;
		if (width > ctx->max)
		{
			width = ctx->max;
		}
		if (width < 0)
		{
			width = 0;
		}
		coords->bar1_value[2] = width;

		//bar2
		width = value * ctx->koeff;
		width -= ctx->max;

		if (width > ctx->max)
		{
			width = ctx->max;
		}
		if (width < 0)
		{
			width = 0;
		}

		coords->bar2_value[2] = width;
	}
	else
	{
		float width;
		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		width = value * ctx->koeff;
		if (width > ctx->max)
		{
			width = ctx->max;
		}
		coords->bar1_value[2] = width;
	}
}

static void CG_SHUDBarPreparePrintRTL(const superhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	//Just make left-to-right and mirror it
	CG_SHUDBarPreparePrintLTR(ctx, value, coords);

	coords->bar1_value[0] = coords->bar1_value[0] + ctx->bar[0][2] - coords->bar1_value[2]; //x = x + max_width - width
	coords->bar2_value[0] = coords->bar2_value[0] + ctx->bar[1][2] - coords->bar2_value[2]; //x = x + max_width - width
}

static void CG_SHUDBarPreparePrintTTB(const superhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	if (ctx->two_bars)
	{
		float height;

		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		Vector4Copy(ctx->bar[1], coords->bar2_value);
		Vector4Copy(ctx->bar[1], coords->bar2_bottom);
		//bar1
		height = value * ctx->koeff;
		if (height > ctx->max)
		{
			height = ctx->max;
		}
		if (height < 0)
		{
			height = 0;
		}
		coords->bar1_value[3] = height;

		//bar2
		height = value * ctx->koeff;
		height -= ctx->max;

		if (height > ctx->max)
		{
			height = ctx->max;
		}
		if (height < 0)
		{
			height = 0;
		}

		coords->bar2_value[3] = height;
	}
	else
	{
		float height;
		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		height = value * ctx->koeff;
		if (height > ctx->max)
		{
			height = ctx->max;
		}
		if (height < 0)
		{
			height = 0;
		}
		coords->bar1_value[2] = height;
	}
}

static void CG_SHUDBarPreparePrintBTT(const superhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	//Just make top to bottom and mirror it
	CG_SHUDBarPreparePrintTTB(ctx, value, coords);

	coords->bar1_value[1] += ctx->bar[0][3] - coords->bar1_value[3]; //y = y + max_height - height
	coords->bar2_value[1] += ctx->bar[1][3] - coords->bar2_value[3]; //y = y + max_height - height
}

void CG_SHUDBarPrint(const superhudConfig_t* cfg, superhudBarContext_t* ctx, float value)
{
	drawBarCoords_t coords;

	memset(&coords, 0, sizeof(coords));

	switch (ctx->direction)
	{
		default:
		case SUPERHUD_DIR_LEFT_TO_RIGHT:
			CG_SHUDBarPreparePrintLTR(ctx, value, &coords);
			break;
		case SUPERHUD_DIR_RIGHT_TO_LEFT:
			CG_SHUDBarPreparePrintRTL(ctx, value, &coords);
			break;
		case SUPERHUD_DIR_TOP_TO_BOTTOM:
			CG_SHUDBarPreparePrintTTB(ctx, value, &coords);
			break;
		case SUPERHUD_DIR_BOTTOM_TO_TOP:
			CG_SHUDBarPreparePrintBTT(ctx, value, &coords);
			break;
	}

	trap_R_SetColor(ctx->color_back);
	trap_R_DrawStretchPic(coords.bar1_bottom[0], coords.bar1_bottom[1], coords.bar1_bottom[2], coords.bar1_bottom[3],
	                      0, 0, 0, 0,
	                      cgs.media.whiteShader);
	if (ctx->two_bars)
	{
		trap_R_DrawStretchPic(coords.bar2_bottom[0], coords.bar2_bottom[1], coords.bar2_bottom[2], coords.bar2_bottom[3],
		                      0, 0, 0, 0,
		                      cgs.media.whiteShader);
	}
	trap_R_SetColor(ctx->color_top);
	trap_R_DrawStretchPic(coords.bar1_value[0], coords.bar1_value[1], coords.bar1_value[2], coords.bar1_value[3],
	                      0, 0, 0, 0,
	                      cgs.media.whiteShader);
	if (ctx->two_bars)
	{
		if (cfg->style.value == 2)
		{
			trap_R_SetColor(ctx->color2_top); // 2nd bar color
		}

		trap_R_DrawStretchPic(coords.bar2_value[0], coords.bar2_value[1], coords.bar2_value[2], coords.bar2_value[3],
		                      0, 0, 0, 0,
		                      cgs.media.whiteShader);
	}
	trap_R_SetColor(NULL);
}

team_t CG_SHUDGetOurActiveTeam(void)
{
	const qboolean is_freeze = CG_OSPIsGameTypeFreeze();

	team_t our_team = TEAM_FREE;
	if (is_freeze)
	{
		our_team = cgs.clientinfo[cg.clientNum].rt;
	}
	else
	{
		our_team = cgs.clientinfo[cg.clientNum].team;
	}

	if (our_team == TEAM_SPECTATOR)
	{
		if (is_freeze)
		{
			our_team = cgs.clientinfo[cg.snap->ps.clientNum].rt;
		}
		else
		{
			our_team = cgs.clientinfo[cg.snap->ps.clientNum].team;
		}
	}
	return our_team;
}

void CG_SHUDFillWithColor(const superhudCoord_t* coord, const float* color)
{
	float x, y, w, h;

	x = coord->named.x;
	y = coord->named.y;
	w = coord->named.w;
	h = coord->named.h;
	CG_AdjustFrom640(&x, &y, &w, &h);
	trap_R_SetColor(color);
	trap_R_DrawStretchPic(x, y, w, h, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_SetColor(NULL);
}

qboolean CG_SHUDFill(const superhudConfig_t* cfg)
{
	float x, y, w, h;
	if (!cfg->fill.isSet || !cfg->rect.isSet)
	{
		return qfalse;
	}

	x = cfg->rect.value[0];
	y = cfg->rect.value[1];
	w = cfg->rect.value[2];
	h = cfg->rect.value[3];

	CG_AdjustFrom640(&x, &y, &w, &h);
	if (cfg->bgcolor.isSet)
	{
		trap_R_SetColor(cfg->bgcolor.value);
		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 0, 0, cgs.media.whiteShader);
		trap_R_SetColor(NULL);
		return qtrue;
	}
	if (cfg->image.isSet)
	{
		vec4_t color;
		qhandle_t image = trap_R_RegisterShader(cfg->image.value);
		if (!image)
		{
			return qfalse;
		}

		CG_SHUDConfigPickColor(cfg, color, qfalse);

		trap_R_SetColor(color);
		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, image);
		trap_R_SetColor(NULL);
		return qtrue;
	}
	return qfalse;
}


void CG_SHUDElementCompileTeamOverlayConfig(int fontWidth, shudTeamOverlay_t* configOut)
{
	configOut->powerupOffsetChar = 0;
	configOut->powerupOffsetPix = configOut->powerupOffsetChar * fontWidth;
	configOut->powerupLenChar = 1;
	configOut->powerupLenPix = configOut->powerupLenChar * fontWidth;

	configOut->nameOffsetChar = 1;
	configOut->nameOffsetPix = configOut->nameOffsetChar * fontWidth;
	configOut->nameLenChar = 12;
	configOut->nameLenPix = configOut->nameLenChar * fontWidth;

	configOut->healthAndArmorOffsetChar = 14;
	configOut->healthAndArmorOffsetPix = configOut->healthAndArmorOffsetChar * fontWidth;
	configOut->healthAndArmorLenChar = 7; // 200/200
	configOut->healthAndArmorLenPix = configOut->healthAndArmorLenChar * fontWidth;

	configOut->weaponOffsetChar = 21;
	configOut->weaponOffsetPix = configOut->weaponOffsetChar * fontWidth;
	configOut->weaponLenChar = 1;
	configOut->weaponLenPix = configOut->weaponLenChar * fontWidth;

	configOut->locationOffsetChar = 23;
	configOut->locationOffsetPix = configOut->locationOffsetChar * fontWidth;
	configOut->locationLenChar = cg_MaxlocationWidth.integer;
	configOut->locationLenPix = configOut->locationLenChar * fontWidth;

	configOut->overlayWidthChar = configOut->locationOffsetChar + cg_MaxlocationWidth.integer;
	configOut->overlayWidthPix = configOut->overlayWidthChar * fontWidth;
}

//
// trap_R_DrawStretchPic Wrapper
//
// float x       X coord of result image
// float y       Y coord of result image
// float w       Width of result image
// float h       Height of result image
// float s1      X coord in the shader (0.0f...1.0f)
// float t1      Y coord in the shader (0.0f...1.0f)
// float s2      Width of image in the shader (0.0f...1.0f)
// float t2      Height of image in the shader (0.0f...1.0f)
// float *color  Use this color
// qhandle_t shader Shader
//
void CG_SHUDDrawStretchPic(superhudCoord_t coord, const superhudCoord_t coordPicture, const float* color, qhandle_t shader)
{
	if (!shader) return;

	trap_R_SetColor(color);
	CG_AdjustFrom640(&coord.named.x, &coord.named.y, &coord.named.w, &coord.named.h);
	trap_R_DrawStretchPic(coord.named.x,
	                      coord.named.y,
	                      coord.named.w,
	                      coord.named.h,
	                      coordPicture.named.x,
	                      coordPicture.named.y,
	                      coordPicture.named.w,
	                      coordPicture.named.h,
	                      shader);
	trap_R_SetColor(NULL);
}

void CG_SHUDDrawStretchPicCtx(const superhudConfig_t* cfg, superhudDrawContext_t* ctx)
{
	// we have to pick color again, because team could changed
	CG_SHUDConfigPickColor(cfg, ctx->color, qfalse);
	CG_SHUDDrawStretchPic(ctx->coord, ctx->coordPicture, ctx->color, ctx->image);
}

int CG_SHUDGetAmmo(int wpi)
{
	int ammo = cg.snap->ps.ammo[wpi];

	if (ammo < 0) ammo = 0;
	if (ammo > 999) ammo = 999;
	return ammo;
}

