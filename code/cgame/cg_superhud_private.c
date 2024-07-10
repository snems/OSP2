#include "cg_superhud_private.h"

static superhudGlobalContext_t superhudGlobalContext;

superhudGlobalContext_t* CG_SHUDGetContext(void)
{
  return &superhudGlobalContext;
}
