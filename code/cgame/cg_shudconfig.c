#include "cg_local.h"


typedef struct superhudConfigFileSection_t
{
  char name[MAX_QPATH];
  char *content;
  qboolean empty;
  struct superhudConfigFileSection_s *next;
} superhudConfigFileSection_t;

static superhudConfigFileSection_t *CG_SHUDConfigFileSeictonInit(superhudConfigFileSection_t *parent)
{

}

