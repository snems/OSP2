#include "./cg_superhud_private.h"
#include "cg_local.h"

static const float *CG_SHUDConfigPickColor(const superhudColor_t *in)
{
  clientInfo_t *ci;

  switch(in->type)
  {
    case SUPERHUD_COLOR_RGBA:
      return in->rgba;
    case SUPERHUD_COLOR_T:
	    ci = &cgs.clientinfo[ cg.clientNum ];
	    if (ci->rt == TEAM_RED)
	    {
	      return colorRed;
	    }
	    else if (ci->rt == TEAM_BLUE)
	    {
	      return colorBlue;
	    }
	    return colorWhite;
    case SUPERHUD_COLOR_E:
	    ci = &cgs.clientinfo[ cg.clientNum ];
	    if (ci->rt == TEAM_RED)
	    {
	      return colorBlue;
	    }
	    else if (ci->rt == TEAM_BLUE)
	    {
	      return colorRed;
	    }
	    return colorWhite;
    case SUPERHUD_COLOR_I:
      return colorWhite;
  }
	return colorWhite;
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
    config->fontsize.value[0] = config->fontsize.value[1]/1.618f;
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

void CG_SHUDTextMakeContext(const superhudConfig_t *in, superhudTextContext *out)
{
  superhudConfig_t config;
  memset(out, 0, sizeof(*out));
  memcpy(&config, in, sizeof(config));

  CG_SHUDConfigDefaultsCheck(&config);

  switch(config.textAlign.value)
  {
    default:
    case SUPERHUD_ALIGNH_LEFT:
      out->textX = config.rect.value[0];
      out->textY = config.rect.value[1];
      out->flags |= DS_LEFT;
      break;
    case SUPERHUD_ALIGNH_CENTER:
      out->textX = (config.rect.value[2] - config.rect.value[0]) / 2.0f;
      out->textY = config.rect.value[1];
      out->flags |= DS_CENTER;
      break;
    case SUPERHUD_ALIGNH_RIGHT:
      out->textX = config.rect.value[0] + config.rect.value[2];
      out->textY = config.rect.value[1];
      out->flags |= DS_RIGHT;
      break;
  }

  out->fontW = config.fontsize.value[0];
  out->fontH = config.fontsize.value[1];

  if (!config.monospace.isSet)
  {
    out->flags |= DS_PROPORTIONAL;
  }
  if (config.textStyle.value&1)
  {
    out->flags |= DS_SHADOW;
  }

  out->fontIndex = 0;

  Vector4Copy(CG_SHUDConfigPickColor(&config.color.value), out->color);
}

void CG_SHUDDrawMakeContext(const superhudConfig_t *in, superhudDrawContext *out)
{
  superhudConfig_t config;
  memset(out, 0, sizeof(*out));
  memcpy(&config, in, sizeof(config));

  CG_SHUDConfigDefaultsCheck(&config);

  out->x = config.rect.value[0];
  out->y = config.rect.value[1];
  out->w = config.rect.value[2];
  out->h = config.rect.value[3];

  CG_AdjustFrom640(&out->x, &out->y, &out->w, &out->h);

  Vector4Copy(CG_SHUDConfigPickColor(&config.color.value), out->color);
}

void CG_SHUDTextPrint(const char *text, const superhudTextContext *ctx)
{
  CG_SelectFont(ctx->fontIndex);
  CG_OSPDrawString(ctx->textX, 
                   ctx->textY, 
                   text, 
                   ctx->color, 
                   ctx->fontW, 
                   ctx->fontH, 
                   ctx->maxchars,
                   ctx->flags);
}

