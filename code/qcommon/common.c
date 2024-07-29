#include "q_shared.h"
#include "qcommon.h"

#ifndef Q3_VM
#include <stdint.h>
#endif

#define	ZONEID	0x1d4a11
#define MINFRAGMENT	64

#ifdef USE_MULTI_SEGMENT
#if 1 // forward lookup, faster allocation
#define DIRECTION next
// we may have up to 4 lists to group free blocks by size
//#define TINY_SIZE	32
#define SMALL_SIZE	64
#define MEDIUM_SIZE	128
#else // backward lookup, better free space consolidation
#define DIRECTION prev
#define TINY_SIZE	64
#define SMALL_SIZE	128
#define MEDIUM_SIZE	256
#endif
#endif

#define USE_STATIC_TAGS
#define USE_TRASH_TEST

#ifdef ZONE_DEBUG
typedef struct zonedebug_s {
	const char *label;
	const char *file;
	int line;
	int allocSize;
} zonedebug_t;
#endif

typedef struct memblock_s {
	struct memblock_s	*next, *prev;
	int			size;	// including the header and possibly tiny fragments
	memtag_t	tag;	// a tag of 0 is a free block
	int			id;		// should be ZONEID
#ifdef ZONE_DEBUG
	zonedebug_t d;
#endif
} memblock_t;

typedef struct freeblock_s {
	struct freeblock_s *prev;
	struct freeblock_s *next;
} freeblock_t;

typedef struct memzone_s {
	int		size;			// total bytes malloced, including header
	int		used;			// total bytes used
	memblock_t	blocklist;	// start / end cap for linked list
#ifdef USE_MULTI_SEGMENT
	memblock_t	dummy0;		// just to allocate some space before freelist
	freeblock_t	freelist_tiny;
	memblock_t	dummy1;
	freeblock_t	freelist_small;
	memblock_t	dummy2;
	freeblock_t	freelist_medium;
	memblock_t	dummy3;
	freeblock_t	freelist;
#else
	memblock_t	*rover;
#endif
} memzone_t;
static int minfragment = MINFRAGMENT; // may be adjusted at runtime
static memzone_t *mainzone;
static memzone_t *smallzone;

static int mainzone_static[MAINZONE_STATIC_SIZE/sizeof(int)]; //static mainzone memory
//
static const char *tagName[ TAG_COUNT ] = {
	"FREE",
	"GENERAL",
	"PACK",
	"SEARCH-PATH",
	"SEARCH-PACK",
	"SEARCH-DIR",
	"BOTLIB",
	"RENDERER",
	"CLIENTS",
	"SMALL",
	"STATIC"
};

/*
========================
Zone_Stats
========================
*/
static void Zone_Stats( const char *name, const memzone_t *z, qboolean printDetails, zone_stats_t *stats )
{
	const memblock_t *block;
	const memzone_t *zone;
	zone_stats_t st;

	memset( &st, 0, sizeof( st ) );
	zone = z;
	st.zoneSegments = 1;
	st.freeSmallest = 0x7FFFFFFF;

	//if ( printDetails ) {
	//	Com_Printf( "---------- %s zone segment #%i ----------\n", name, zone->segnum );
	//}

	for ( block = zone->blocklist.next ; ; ) {
		if ( printDetails ) {
			int tag = block->tag;
			Com_Printf( "block:%p  size:%8i  tag: %s\n", (void *)block, block->size,
				(unsigned)tag < TAG_COUNT ? tagName[ tag ] : va( "%i", tag ) );
		}
		if ( block->tag != TAG_FREE ) {
			st.zoneBytes += block->size;
			st.zoneBlocks++;
			if ( block->tag == TAG_BOTLIB ) {
				st.botlibBytes += block->size;
			} else if ( block->tag == TAG_RENDERER ) {
				st.rendererBytes += block->size;
			}
		} else {
			st.freeBytes += block->size;
			st.freeBlocks++;
			if ( block->size > st.freeLargest )
				st.freeLargest = block->size;
			if ( block->size < st.freeSmallest )
				st.freeSmallest = block->size;
		}
		if ( block->next == &zone->blocklist ) {
			break; // all blocks have been hit
		}
		if ( (byte *)block + block->size != (byte *)block->next) {
#ifdef USE_MULTI_SEGMENT
			const memblock_t *next = block->next;
			if ( next->size == 0 && next->id == -ZONEID && next->tag == TAG_GENERAL ) {
				st.zoneSegments++;
				if ( printDetails ) {
					Com_Printf( "---------- %s zone segment #%i ----------\n", name, st.zoneSegments );
				}
				block = next->next;
				continue;
			} else
#endif
				Com_Printf( "ERROR: block size does not touch the next block\n" );
		}
		if ( block->next->prev != block) {
			Com_Printf( "ERROR: next block doesn't have proper back link\n" );
		}
		if ( block->tag == TAG_FREE && block->next->tag == TAG_FREE ) {
			Com_Printf( "ERROR: two consecutive free blocks\n" );
		}
		block = block->next;
	}

	// export stats
	if ( stats ) {
		memcpy( stats, &st, sizeof( *stats ) );
	}
}

/*
========================
Z_Stats
========================
*/
void Z_Stats(zone_stats_t *stats )
{
  Zone_Stats("Mainzone", (memzone_t*)mainzone_static, qtrue, stats );
}

/*
========================
Z_ClearZone
========================
*/
static void Z_ClearZone( memzone_t *zone, memzone_t *head, int size, int segnum ) {
	memblock_t	*block;
	int min_fragment;

#ifdef USE_MULTI_SEGMENT
	min_fragment = sizeof( memblock_t ) + sizeof( freeblock_t );
#else
	min_fragment = sizeof( memblock_t );
#endif

	if ( minfragment < min_fragment ) {
		// in debug mode size of memblock_t may exceed MINFRAGMENT
		minfragment = PAD( min_fragment, sizeof( intptr_t ) );
	}

	// set the entire zone to one free block
	zone->blocklist.next = zone->blocklist.prev = block = (memblock_t *)( zone + 1 );
	zone->blocklist.tag = TAG_GENERAL; // in use block
	zone->blocklist.id = -ZONEID;
	zone->blocklist.size = 0;
#ifndef USE_MULTI_SEGMENT
	zone->rover = block;
#endif
	zone->size = size;
	zone->used = 0;

	block->prev = block->next = &zone->blocklist;
	block->tag = TAG_FREE;	// free block
	block->id = ZONEID;

	block->size = size - sizeof(memzone_t);

#ifdef USE_MULTI_SEGMENT
	InitFree( &zone->freelist );
	zone->freelist.next = zone->freelist.prev = &zone->freelist;

	InitFree( &zone->freelist_medium );
	zone->freelist_medium.next = zone->freelist_medium.prev = &zone->freelist_medium;

	InitFree( &zone->freelist_small );
	zone->freelist_small.next = zone->freelist_small.prev = &zone->freelist_small;

	InitFree( &zone->freelist_tiny );
	zone->freelist_tiny.next = zone->freelist_tiny.prev = &zone->freelist_tiny;

	InsertFree( zone, block );
#endif
}


/*
========================
Z_AvailableZoneMemory
========================
*/
static int Z_AvailableZoneMemory( const memzone_t *zone ) {
#ifdef USE_MULTI_SEGMENT
	return (1024*1024*1024); // unlimited
#else
	return zone->size - zone->used;
#endif
}


/*
========================
Z_AvailableMemory
========================
*/
int Z_AvailableMemory( void ) {
	return Z_AvailableZoneMemory( mainzone );
}


static void MergeBlock( memblock_t *curr_free, const memblock_t *next )
{
	curr_free->size += next->size;
	curr_free->next = next->next;
	curr_free->next->prev = curr_free;
}


/*
========================
Z_Free
========================
*/
void Z_Free(const void *ptr ) {
	memblock_t	*block, *other;
	memzone_t *zone;

	if (!ptr) {
		Com_Error( ERR_DROP, "Z_Free: NULL pointer" );
	}

	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID) {
		Com_Error( ERR_FATAL, "Z_Free: freed a pointer without ZONEID" );
	}

	if (block->tag == TAG_FREE) {
		Com_Error( ERR_FATAL, "Z_Free: freed a freed pointer" );
	}

	// if static memory
#ifdef USE_STATIC_TAGS
	if (block->tag == TAG_STATIC) {
		return;
	}
#endif

	// check the memory trash tester
#ifdef USE_TRASH_TEST
	if ( *(int *)((byte *)block + block->size - 4 ) != ZONEID ) {
		Com_Error( ERR_FATAL, "Z_Free: memory block wrote past end" );
	}
#endif

	if ( block->tag == TAG_SMALL ) {
		zone = smallzone;
	} else {
		zone = mainzone;
	}

	zone->used -= block->size;

	// set the block to something that should cause problems
	// if it is referenced...
	Com_Memset( (void*)ptr, 0xaa, block->size - sizeof( *block ) );

	block->tag = TAG_FREE; // mark as free
	block->id = ZONEID;

	other = block->prev;
	if ( other->tag == TAG_FREE ) {
#ifdef USE_MULTI_SEGMENT
		RemoveFree( other );
#endif
		// merge with previous free block
		MergeBlock( other, block );
#ifndef USE_MULTI_SEGMENT
		if ( block == zone->rover ) {
			zone->rover = other;
		}
#endif
		block = other;
	}

#ifndef USE_MULTI_SEGMENT
	zone->rover = block;
#endif

	other = block->next;
	if ( other->tag == TAG_FREE ) {
#ifdef USE_MULTI_SEGMENT
		RemoveFree( other );
#endif
		// merge the next free block onto the end
		MergeBlock( block, other );
	}

#ifdef USE_MULTI_SEGMENT
	InsertFree( zone, block );
#endif
}


/*
================
Z_FreeTags
================
*/
int Z_FreeTags( memtag_t tag ) {
	int			count;
	memzone_t	*zone;
	memblock_t	*block, *freed;

	if ( tag == TAG_STATIC ) {
		Com_Error( ERR_FATAL, "Z_FreeTags( TAG_STATIC )" );
		return 0;
	} else if ( tag == TAG_SMALL ) {
		zone = smallzone;
	} else {
		zone = mainzone;
	}

	count = 0;
	for ( block = zone->blocklist.next ; ; ) {
		if ( block->tag == tag && block->id == ZONEID ) {
			if ( block->prev->tag == TAG_FREE )
				freed = block->prev;  // current block will be merged with previous
			else
				freed = block; // will leave in place
			Z_Free( (void*)( block + 1 ) );
			block = freed;
			count++;
		}
		if ( block->next == &zone->blocklist ) {
			break;	// all blocks have been hit
		}
		block = block->next;
	}

	return count;
}


/*
================
Z_TagMalloc
================
*/
#ifdef ZONE_DEBUG
void *Z_TagMallocDebug( int size, memtag_t tag, char *label, char *file, int line ) {
	int		allocSize;
#else
void *Z_TagMalloc( int size, memtag_t tag ) {
#endif
	int		extra;
#ifndef USE_MULTI_SEGMENT
	memblock_t	*start, *rover;
#endif
	memblock_t *base;
	memzone_t *zone;

	if ( tag == TAG_FREE ) {
		Com_Error( ERR_FATAL, "Z_TagMalloc: tried to use with TAG_FREE" );
	}

	if ( tag == TAG_SMALL ) {
		zone = smallzone;
	} else {
		zone = mainzone;
	}

#ifdef ZONE_DEBUG
	allocSize = size;
#endif

#ifdef USE_MULTI_SEGMENT
	if ( size < (sizeof( freeblock_t ) ) ) {
		size = (sizeof( freeblock_t ) );
	}
#endif

	//
	// scan through the block list looking for the first free block
	// of sufficient size
	//
	size += sizeof( *base );	// account for size of block header
#ifdef USE_TRASH_TEST
	size += 4;					// space for memory trash tester
#endif

	size = PAD(size, sizeof(intptr_t));		// align to 32/64 bit boundary
	//
	//
	if (Z_AvailableMemory() < size)
  {
    return NULL;
  }

#ifdef USE_MULTI_SEGMENT
	base = SearchFree( zone, size );

	RemoveFree( base );
#else

	base = rover = zone->rover;
	start = base->prev;

	do {
		if ( rover == start ) {
			// scanned all the way around the list
#ifdef ZONE_DEBUG
			Z_LogHeap();
			Com_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone: %s, line: %d (%s)",
								size, zone == smallzone ? "small" : "main", file, line, label );
#else
			Com_Printf( "Z_Malloc: failed on allocation of %i bytes from the %s zone",
								size, zone == smallzone ? "small" : "main" );
#endif
			return NULL;
		}
		if ( rover->tag != TAG_FREE ) {
			base = rover = rover->next;
		} else {
			rover = rover->next;
		}
	} while (base->tag != TAG_FREE || base->size < size);
#endif

	//
	// found a block big enough
	//
	extra = base->size - size;
	if ( extra >= minfragment ) {
		memblock_t *fragment;
		// there will be a free fragment after the allocated block
		fragment = (memblock_t *)( (byte *)base + size );
		fragment->size = extra;
		fragment->tag = TAG_FREE; // free block
		fragment->id = ZONEID;
		fragment->prev = base;
		fragment->next = base->next;
		fragment->next->prev = fragment;
		base->next = fragment;
		base->size = size;
#ifdef USE_MULTI_SEGMENT
		InsertFree( zone, fragment );
#endif
	}

#ifndef USE_MULTI_SEGMENT
	zone->rover = base->next;	// next allocation will start looking here
#endif
	zone->used += base->size;

	base->tag = tag;			// no longer a free block
	base->id = ZONEID;

#ifdef ZONE_DEBUG
	base->d.label = label;
	base->d.file = file;
	base->d.line = line;
	base->d.allocSize = allocSize;
#endif

#ifdef USE_TRASH_TEST
	// marker for memory trash testing
	*(int *)((byte *)base + base->size - 4) = ZONEID;
#endif

	return (void *) ( base + 1 );
}


/*
========================
Z_Malloc
========================
*/
#ifdef ZONE_DEBUG
void *Z_MallocDebug( int size, char *label, char *file, int line ) {
#else
void *Z_Malloc( int size ) {
#endif
	void	*buf;

  //Z_CheckHeap ();	// DEBUG

#ifdef ZONE_DEBUG
	buf = Z_TagMallocDebug( size, TAG_GENERAL, label, file, line );
#else
	buf = Z_TagMalloc( size, TAG_GENERAL );
#endif
  if (buf)
  {
	  Com_Memset( buf, 0, size );
  }

	return buf;
}


/*
========================
S_Malloc
========================
*/
#ifdef ZONE_DEBUG
void *S_MallocDebug( int size, char *label, char *file, int line ) {
	return Z_TagMallocDebug( size, TAG_SMALL, label, file, line );
}
#else
void *S_Malloc( int size ) {
	return Z_TagMalloc( size, TAG_SMALL );
}
#endif

char* Z_StrDup(const char *str)
{
	char *new_str;
	new_str = Z_Malloc(strlen(str) + 1);
	if (new_str)
	{
		strcpy(new_str, str);
	}

	return new_str;
}

/*
=================
Com_InitZoneMemory
=================
*/
void Com_InitZoneMemory( void ) {

	mainzone = (memzone_t*)&mainzone_static[0];

	Z_ClearZone( mainzone, mainzone, MAINZONE_STATIC_SIZE, 1 );
}


static int _atoi( const char **stringPtr ) 
{
	int		sign;
	int		value;
	int		c;
	const char	*string;

	string = *stringPtr;

	if ( !*string )
		return 0;

	// check sign
	switch ( *string ) 
	{
	case '+':
		string++;
		sign = 1;
		break;
	case '-':
		string++;
		sign = -1;
		break;
	default:
		sign = 1;
		break;
	}

	// read digits
	value = 0;
	do 
	{
		c = *string;
		if ( c < '0' || c > '9' ) 
		{
			break;
		}
		c -= '0';
		value = value * 10 + c;
		string++;
	} 
	while ( 1 );

	// not handling 10e10 notation...

	*stringPtr = string;

	return value * sign;
}


static float _atof( const char **stringPtr ) 
{
	const char	*string;
	float sign;
	float value;
	float fraction;
	int	  c = '0'; // uninitialized use possible

	string = *stringPtr;

	if ( !*string )
		return 0;

	// check sign
	switch ( *string ) 
	{
	case '+':
		string++;
		sign = 1;
		break;
	case '-':
		string++;
		sign = -1;
		break;
	default:
		sign = 1;
		break;
	}

	// read digits
	value = 0;
	if ( *string != '.' ) 
	{
		do 
		{
			c = *string;
			if ( c < '0' || c > '9' ) 
			{
				break;
			}
			c -= '0';
			value = value * 10 + c;
			string++;
		} 
		while ( 1 );
	}

	// check for decimal point
	if ( *string == '.' ) 
	{
		fraction = 0.1f;
		string++;
		do 
		{
			c = *string;
			if ( c < '0' || c > '9' ) 
			{
				break;
			}
			c -= '0';
			value += c * fraction;
			fraction *= 0.1f;
			string++;
		}
		while ( 1 );
	}

	// not handling 10e10 notation...
	*stringPtr = string;

	return value * sign;
}


static void _atos( const char **stringPtr, char *buffer, int delimiter, int width ) 
{
	const char	*string;

	string = *stringPtr;

	if ( !delimiter ) 
	{
		// skip whitespace
		while ( *string && *string != ' ' && *string != '\t' && width-- > 0 ) 
		{
			*buffer = *string;
			buffer++;
			string++;
		}
	} 
	else while ( *string && *string != delimiter && width-- > 0 ) 
	{
		*buffer = *string;
		buffer++;
		string++;
	}

	*stringPtr = string;

	*buffer = '\0';
}


int Q_sscanf( const char *buffer, const char *fmt, ... ) 
{
	va_list ap;
	int count;
	int width;
	int cmd;
	const char *p;

	va_start( ap, fmt );
	count = 0;

	while ( *fmt ) 
	{
		// single whitespace char validates any quantity of whitespace characters 
		// extracted from the stream (including none)
		if ( *fmt == ' ' || *fmt == '\t' || *fmt == '\n' ) 
		{
			while ( *buffer == ' ' || *buffer == '\t' || *buffer == '\n' )
				buffer++;
			fmt++;
		}

		if ( *fmt != '%' ) 
		{
			if ( *fmt != *buffer ) 
				break;

			buffer++;
			fmt++;
			continue;
		}

		width = fmt[1];
		fmt++; // %
		if ( width >= '0' && width <= '9' ) 
		{
			width -= '0'; // valid width;
			fmt++;	// ['0'..'9']
			cmd = *fmt;
		}
		else 
		{
			cmd = width;
			width = 1024; // some assumption
		}

		p = buffer;

		fmt++; // switch to delimiter?

		//printf( "cmd=%c buffer=%s width=%i delim='%c'\n", cmd, buffer, width, *fmt );

		switch ( cmd ) 
		{
		case 'i':
		case 'd':
		case 'u':
			*(va_arg(ap, int *)) = _atoi( &buffer );
			break;
		case 'f':
			*(va_arg(ap, float *)) = _atof( &buffer );
			break;
		case 'c':
			*(va_arg(ap, char *)) = *buffer; buffer++;
			break;
		case 's':
			_atos( &buffer, va_arg(ap, char *), *fmt, width );
			break;
		default:
			return count;
		}

		if ( p != buffer )
			count++;
		else
			break;
	}
	va_end( ap );

	return count;
}

