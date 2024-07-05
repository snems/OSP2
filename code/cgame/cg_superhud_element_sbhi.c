#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
  superhudConfig_t config;
  superhudDrawContext ctx;
} shudElementStatusbarHealthIcon;

void* CG_SHUDElementSBHICreate(superhudConfig_t* config)
{
  shudElementStatusbarHealthIcon *sbhi;

  sbhi = Z_Malloc(sizeof(*sbhi));
  OSP_MEMORY_CHECK(sbhi);

  memset(sbhi,0,sizeof(*sbhi));

  memcpy(&sbhi->config, config, sizeof(sbhi->config));

  CG_SHUDDrawMakeContext(&sbhi->config, &sbhi->ctx);

  if (config->image.isSet)
  {
    sbhi->ctx.image = trap_R_RegisterShader(sbhi->config.image.value);
  }

  if (!sbhi->ctx.image)
  {
    sbhi->ctx.image = trap_R_RegisterShader("icons/iconh_yellow.tga");
  }

  return sbhi;
}

void CG_SHUDElementSBHIRoutine(void *context)
{
  shudElementStatusbarHealthIcon *sbhi = (shudElementStatusbarHealthIcon *)context;

	trap_R_SetColor(sbhi->ctx.color);
	trap_R_DrawStretchPic(sbhi->ctx.x, sbhi->ctx.y, sbhi->ctx.w+1, sbhi->ctx.h, 0, 0, 1, 1, sbhi->ctx.image);
	trap_R_SetColor(NULL);

}

void CG_SHUDElementSBHIDestroy(void *context)
{
  if (context)
  {
    Z_Free(context);
  }
}
