#include "cg_local.h"
#include "../qcommon/qcommon.h"


static superhudElement_t *CG_SHUDElementAddChild(superhudElement_t *parent, const char *name, const superhudConfig_t *config, const superhudElementContent_t *content)
{
  int len;
  superhudElement_t *newElem;

  newElem = Z_Malloc(sizeof(*newElem));
	if (!newElem)
	{
		OSP_MEMORY_EXCEPTION();
	}

	if (parent)
	{
	  parent->next = newElem;
	}

	Q_strncpyz(newElem->name, name, MAX_QPATH);

  memcpy(&newElem->content, &content, sizeof(newElem->content));
  memcpy(&newElem->config, &content, sizeof(newElem->config));

  return newElem;
}

static void CG_SHUDConfigElementListDestroy(superhudElement_t **root)
{
  superhudElement_t *lroot = *root;
  superhudElement_t *tmp;

  while(lroot)
  {
    tmp = lroot;
    lroot = lroot->next;
    Z_Free(tmp);
  }
  *root = NULL;
}


superhudElement_t* CG_SHUDLoadConfig(const char *data)
{
  //prepare data:
  
  // 1. cut comments

  // 2. 
  
  while (*data)
  {
    //skip spaces or newlines
    while (*data == ' ' || *data == '\n')
    {
      ++data;
    }

  }

  //read fileneame into local buffer
  
  //split fileneame into sections

  //
}

