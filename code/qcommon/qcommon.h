#ifndef QCOMMON_H
#define QCOMMON_H

#include "q_shared.h"
#ifdef __cplusplus
extern "C" {
#endif


typedef struct zone_stats_s {
	int	zoneSegments;
	int zoneBlocks;
	int	zoneBytes;
	int	botlibBytes;
	int	rendererBytes;
	int freeBytes;
	int freeBlocks;
	int freeSmallest;
	int freeLargest;
} zone_stats_t;

typedef enum {
	TAG_FREE,
	TAG_GENERAL,
	TAG_PACK,
	TAG_SEARCH_PATH,
	TAG_SEARCH_PACK,
	TAG_SEARCH_DIR,
	TAG_BOTLIB,
	TAG_RENDERER,
	TAG_CLIENTS,
	TAG_SMALL,
	TAG_STATIC,
	TAG_COUNT
} memtag_t;

#define MAINZONE_STATIC_SIZE (4 *1024 * 1024)

void Com_InitZoneMemory( void );
void *Z_Malloc( int size );
void Z_Free(const void *ptr );
int Z_AvailableMemory( void );
void Z_Stats(zone_stats_t *stats);
char* Z_StrDup(const char *str);

int Q_sscanf( const char *buffer, const char *fmt, ... );

#ifdef __cplusplus
}
#endif

#endif


