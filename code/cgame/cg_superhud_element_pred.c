#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudDrawContext ctx;
} shudElementStatusbarPreDecorate;

void* CG_SHUDElementPreDCreate(superhudConfig_t* config)
{
  shudElementStatusbarPreDecorate *sbai;

  sbai = Z_Malloc(sizeof(*sbai));
  OSP_MEMORY_CHECK(sbai);

  memset(sbai,0,sizeof(*sbai));

  memcpy(&sbai->config, config, sizeof(sbai->config));

  CG_SHUDDrawMakeContext(&sbai->config, &sbai->ctx);

  if (config->image.isSet)
  {
    sbai->ctx.image = trap_R_RegisterShader(sbai->config.image.value);
  }

  return sbai;
}

void CG_SHUDElementPreDRoutine(void *context)
{
  shudElementStatusbarPreDecorate *sbai = (shudElementStatusbarPreDecorate *)context;

	trap_R_SetColor(sbai->ctx.color);
	trap_R_DrawStretchPic(sbai->ctx.x, sbai->ctx.y, sbai->ctx.w+1, sbai->ctx.h, 0, 0, 1, 1, 
	     sbai->ctx.image ? sbai->ctx.image: cgs.media.whiteShader );
	trap_R_SetColor(NULL);

}

void CG_SHUDElementPreDDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
