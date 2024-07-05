#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudDrawContext ctx;
} shudElementStatusbarArmorIcon;

void* CG_SHUDElementSBAICreate(superhudConfig_t* config)
{
  shudElementStatusbarArmorIcon *sbai;

  sbai = Z_Malloc(sizeof(*sbai));
  OSP_MEMORY_CHECK(sbai);

  memset(sbai,0,sizeof(*sbai));

  memcpy(&sbai->config, config, sizeof(sbai->config));

  CG_SHUDDrawMakeContext(&sbai->config, &sbai->ctx);

  if (config->image.isSet)
  {
    sbai->ctx.image = trap_R_RegisterShader(sbai->config.image.value);
  }

  if (!sbai->ctx.image)
  {
    sbai->ctx.image = cgs.media.armorIcon[cg.snap->ps.stats[STAT_OSP_9]];
  }

  return sbai;
}

void CG_SHUDElementSBAIRoutine(void *context)
{
  shudElementStatusbarArmorIcon *sbai = (shudElementStatusbarArmorIcon *)context;

	trap_R_SetColor(sbai->ctx.color);
	trap_R_DrawStretchPic(sbai->ctx.x, sbai->ctx.y, sbai->ctx.w+1, sbai->ctx.h, 0, 0, 1, 1, sbai->ctx.image);
	trap_R_SetColor(NULL);

}

void CG_SHUDElementSBAIDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
