#ifndef CG_SUPERHUD_H
#define CG_SUPERHUD_H

#include "cg_local.h"

#ifdef __cplusplus
extern "C" {
#endif

void CG_SHUDSetConfig(const char* filename);
void CG_SHUDLoadConfig(void);
void CG_SHUDRoutine(void);

void CG_SHUDEventFrag(const char* message);
void CG_SHUDEventChat(const char* message);
void CG_SHUDEventTeamChat(const char* message);

#ifdef __cplusplus
}
#endif

#endif
