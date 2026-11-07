/*
 * g_osp_maps.c: the OSP map list (maps.txt, map_autoload, mapsession<n> cvars), map
 * rotation and the map list commands.
 */

#include "g_local.h"

// OSP: g_bot.c's arena list (OSP_LoadMaplist's map_autoload; G_LoadArenas is not
// static in OSP), declared here to keep g_bot.c's bss order
extern int      g_numArenas;
void            G_LoadArenas(void);
const char*      G_GetArenaInfoByMap(const char* map);
extern  void    LogExit(const char* string);         // g_main.c

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t    mapsessioncount;
vmCvar_t    mapsessionauto;
vmCvar_t    map_rotate;
vmCvar_t    map_random;
vmCvar_t    map_queue;
vmCvar_t    map_once;
vmCvar_t    map_nocount;
vmCvar_t    map_halt;
vmCvar_t    map_file;
vmCvar_t    map_debug;
vmCvar_t    map_autoload;
// OSP: the OSP globals declared in g_local.h (see there); osp_maps comes
// above the cvars because g_local.h declares it first.
ospMap_t    osp_maps[MAX_OSP_MAPS];
int     osp_nextMapIndex;
char    osp_mapFileList[0x10000];
char    osp_voteMapFileList[0x10000];
int     osp_matchPaused;
int     osp_roundsPlayed;
int     osp_unpausing;
int     osp_timeoutEndTime;
int     osp_matchTime;
char    osp_matchResultPrint[120];
char    osp_matchResultCP[120];

// OSP: initialised data
int     osp_numMaps = 0;            // entries of the map list (OSP_LoadMaplist / OSP_LoadMapSession)
int     osp_mapIndex = 0;           // osp_maps index of the current / next map (OSP_NextMap)
int     osp_mapIndexSet = 0;        // next map chosen (OSP_SetMapIndex sets it, OSP_NextMap keeps it; CheckVote clears it on a failed vote)
int     osp_mapChangeReason = 0;    // OSP_NextMap's argument (1 voted map, 2 referee map change, 3 default mode rotation); 0 = the map ended normally
int     osp_mapFileListState = 0;   // 0 not checked yet, 1 the map_autoload file list is loaded, -1 none
// statics and initialised locals of the functions below


/*
==================
OSP_NextMap

Picks the next map of the map list and sets "nextmap" (map_queue, map_rotate, map_random,
map_once, map_nocount; reason as osp_mapChangeReason)
==================
*/
void OSP_NextMap(int reason) {
	int     players;
	int     found;
	int     tries;
	char    mapname[MAX_STRING_CHARS];
	char    info[MAX_INFO_STRING];

	players = 0;
	found = 0;
	tries = 0;

	if (!map_queue.integer || (!map_rotate.integer && !reason)) {
		return;
	}

	if (osp_mapChangeReason == 1) {
		trap_SendServerCommand(-1, va("print \"Voted map: ^3%s\n\"", osp_maps[osp_mapIndex].name));
		return;
	}
	if (osp_mapChangeReason == 2) {
		trap_SendServerCommand(-1, va("print \"Referee changed to map: ^3%s\n\"", osp_maps[osp_mapIndex].name));
		return;
	}
	if (osp_mapChangeReason == 4) {
		return;
	}

	osp_mapChangeReason = reason;
	if (!osp_numMaps) {
		OSP_LoadMaplist();
	}

	if (osp_numMaps && !osp_mapIndexSet) {
		int     i;

		trap_GetServerinfo(info, sizeof(info));
		Q_strncpyz(mapname, Info_ValueForKey(info, "mapname"), sizeof(mapname));

		for (i = 0; i < osp_numMaps; i++) {
			if (!strcmp(mapname, osp_maps[i].name)) {
				osp_mapIndex = i;
				osp_maps[i].visited = 1;
				break;
			}
		}

		if (!map_once.integer) {
			for (i = 0; i < osp_numMaps; i++) {
				osp_maps[i].visited = 0;
			}
		}

		do {
			int     j;

			if (map_random.integer) {
				osp_mapIndex = (int)(random() * osp_numMaps) % osp_numMaps;
				if (map_debug.integer) {
					G_Printf("Random Map %d %s\n", osp_mapIndex, osp_maps[osp_mapIndex].name);
				}
			}

			if (!map_once.integer && !map_random.integer) {
				j = (osp_mapIndex + 1) % osp_numMaps;
			} else {
				j = osp_mapIndex % osp_numMaps;
			}

			players = level.numConnectedClients;
			do {
				if (!osp_maps[j].visited) {
					if ((osp_maps[j].minPlayers <= players && osp_maps[j].maxPlayers >= players) ||
					        (!osp_maps[j].minPlayers && (!osp_maps[j].maxPlayers || !players)) ||
					        (!players && osp_maps[j].minPlayers == 1) || map_nocount.integer) {
						osp_mapIndex = j;
						j = -1;
						found = 1;
						if (map_debug.integer) {
							G_Printf("Map Found %s [fVisited = %d]\n", osp_maps[osp_mapIndex].name, osp_maps[osp_mapIndex].visited);
						}
					} else {
						j = (j + 1) % osp_numMaps;
					}
				} else {
					j = (j + 1) % osp_numMaps;
				}
			} while (j != -1 && j != osp_mapIndex);

			if (j == osp_mapIndex) {
				if (map_debug.integer) {
					G_Printf("Map could not be found\n");
				}
				if (map_once.integer > 0) {
					if (map_debug.integer) {
						G_Printf("Clearing Visited flags\n");
					}
					for (i = 0; i < osp_numMaps; i++) {
						osp_maps[i].visited = 0;
					}
				}

				if (map_random.integer)
					j = rand() % osp_numMaps;
				else
					j = 0;
				if (j < 0) {
					j = 0 - j;
				}

				for (i = 0; i < osp_numMaps; i++, j++) {
					if (j >= osp_numMaps) {
						j = 0;
					}
					if ((osp_maps[j].minPlayers <= players && osp_maps[j].maxPlayers >= players) ||
					        !osp_maps[j].minPlayers || (!players && osp_maps[j].minPlayers == 1) || map_nocount.integer) {
						osp_mapIndex = j;
						break;
					}
				}
				found = 1;
			}
			tries++;
		} while (!found && tries < 2);
	} else if (osp_numMaps && osp_mapIndexSet) {
		found = 1;
		osp_mapIndexSet = 0;
		osp_mapIndex = osp_nextMapIndex;
	} else {
		osp_mapIndex = 0;
	}

	if (found) {
		if (map_once.integer > 0) {
			osp_maps[osp_mapIndex].visited = 1;
		}
		trap_SendServerCommand(-1, va("print \"Next map: ^3%s\n\"", osp_maps[osp_mapIndex].name));

		if (osp_maps[osp_mapIndex].config[0]) {
			strcpy(level.map_config, osp_maps[osp_mapIndex].config);
		} else {
			level.map_config[0] = 0;
			if (osp_maps[osp_mapIndex].fraglimit >= 0) {
				trap_Cvar_Set("fraglimit", va("%d", osp_maps[osp_mapIndex].fraglimit));
			}
			if (osp_maps[osp_mapIndex].capturelimit >= 0) {
				trap_Cvar_Set("capturelimit", va("%d", osp_maps[osp_mapIndex].capturelimit));
			}
			if (osp_maps[osp_mapIndex].timelimit >= 0) {
				trap_Cvar_Set("timelimit", va("%d", osp_maps[osp_mapIndex].timelimit));
			}
			if (osp_maps[osp_mapIndex].roundlimit >= 0) {
				trap_Cvar_Set("roundlimit", va("%d", osp_maps[osp_mapIndex].roundlimit));
			}
		}

		trap_SendConsoleCommand(EXEC_APPEND, va("set nextmap \"map %s\"\n", osp_maps[osp_mapIndex].name));
		if (map_debug.integer) {
			G_Printf("MAP CHANGE: %d %s", osp_mapIndex, osp_maps[osp_mapIndex].name);
			G_Printf(" [min = %d,max = %d, players = %d]\n", osp_maps[osp_mapIndex].minPlayers, osp_maps[osp_mapIndex].maxPlayers, players);
		}
	}
	OSP_SaveMapSession();
}

/*
==================
OSP_MapCheckThink

Think of the "delay_mapcheck" entity: loads the map list once the map is running
==================
*/
void OSP_MapCheckThink(gentity_t* ent) {
	osp_mapFileListState = 1;
	OSP_LoadMaplist();
}

/*
==================
OSP_DelayMapCheck
==================
*/
void OSP_DelayMapCheck(int time) {
	gentity_t*   te;

	te = G_Spawn();
	te->s.eType = ET_ITEM;
	te->r.svFlags |= SVF_NOCLIENT;
	te->s.eFlags |= EF_NODRAW;
	te->classname = "delay_mapcheck";
	te->nextthink = time;
	te->think = OSP_MapCheckThink;
	trap_LinkEntity(te);
}

/*
==================
OSP_LoadMaplist

Map list from map_file (maps.txt), plus the maps\*.bsp files whose .arena type fits the
gametype (map_autoload); a session saved in the mapsession<n> cvars is used instead
==================
*/
void OSP_LoadMaplist(void) {
	static int  delayed = 0;
	int             numFiles;
	int             len;
	char            buf[MAX_OSP_FILE_CHARS];
	fileHandle_t    f;
	char            filename[MAX_QPATH];

	osp_numMaps = 0;
	filename[0] = 0;
	osp_mapIndexSet = 0;
	osp_mapChangeReason = 0;

	if (OSP_LoadMapSession(qfalse)) {
		return;
	}

	if (map_file.string[0]) {
		strcpy(filename, map_file.string);
	} else {
		strcpy(filename, "maps.txt");
	}

	// the file list isn't available right after the map start
	if (!osp_mapFileListState && level.time - level.startTime < 1000) {
		if (!delayed) {
			delayed = 1;
			OSP_DelayMapCheck(level.time + 1000);
		}
		return;
	}

	if (OSP_CheckEngineVersion(buf) && map_autoload.integer > 0) {
		numFiles = OSP_GetMapFileList(osp_mapFileList, sizeof(osp_mapFileList) - 2);
		osp_mapFileListState = 1;
	} else {
		numFiles = 0;
		osp_mapFileListState = -1;
	}

	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if (!numFiles && (len < 0 || !f)) {
		G_Printf("\nERROR: Could not open maps list file \"%s\" (%d, %d)\n\n", filename, len, f);
		return;
	}
	// the size check must not depend on the .bsp auto-list: the file is read into a fixed
	// buffer whatever else was found (and the terminating NUL needs one byte of room)
	if (f && len >= sizeof(buf)) {
		G_Printf("\nMap file \"%s\" too big!  Maplist not loaded!! (file_size: %d, max_size: %d)\n", filename, len, sizeof(buf) - 1);
		trap_FS_FCloseFile(f);
		return;
	}

	if (f) {
		ospMap_t    map;
		int         ret;

		G_Printf("\nLoading maplist from \"%s\"\n", filename);
		trap_FS_Read(buf, len, f);
		buf[len] = 0;

		do {
			map.config[0] = 0;
			map.name[0] = 0;
			map.minPlayers = 0;
			map.maxPlayers = 0;
			map.fraglimit = -1;
			map.capturelimit = -1;
			map.timelimit = -1;
			map.roundlimit = -1;
			map.visited = 0;

			ret = OSP_ParseMapEntry(buf, len, &map);
			if (ret >= 1) {
				if (osp_mapFileListState <= 0 || OSP_MapExists(osp_mapFileList, strlen(osp_mapFileList), numFiles, map.name)) {
					if (map.maxPlayers == -32145) {
					} else if (map.config[0]) {
						G_Printf("Map entry: \"%s\" (min:%2d, max:%2d, config: %s)\n", map.name, map.minPlayers, map.maxPlayers, map.config);
					} else {
						G_Printf("Map entry: \"%s\" (min:%2d, max:%2d, fl:%2d, cl:%2d, tl:%2d rl:%d)\n", map.name, map.minPlayers, map.maxPlayers, map.fraglimit, map.capturelimit, map.timelimit, map.roundlimit);
					}
					if (map.name[0] && map.maxPlayers != -32145) {
						memcpy(&osp_maps[osp_numMaps], &map, sizeof(ospMap_t));
						osp_numMaps++;
					}
				} else {
					G_Printf("Unknown map: \"%s\" - removed from map list\n", map.name);
				}
			}
		} while (ret != 0 && ret != 100 && osp_numMaps < MAX_OSP_MAPS);

		if (osp_numMaps >= MAX_OSP_MAPS) {
			G_Printf(va("WARNING: Maplist at maximum %d entries!\n", MAX_OSP_MAPS));
		}
		trap_FS_FCloseFile(f);
	} else {
		G_Printf("\nERROR: Could not open maps list file \"%s\"\n\n", filename);
	}

	if (osp_mapFileListState && numFiles && map_autoload.integer > 0) {
		ospMap_t    map;
		char*        p;
		int         i;
		const char*  info;

		if (!g_numArenas) {
			G_LoadArenas();
		}

		p = osp_mapFileList;
		for (i = 0; i < numFiles && osp_numMaps < MAX_OSP_MAPS; i++) {
			if (p && *p > 1) {
				map.config[0] = 0;
				map.name[0] = 0;
				map.minPlayers = 0;
				map.maxPlayers = 0;
				map.fraglimit = -1;
				map.capturelimit = -1;
				map.timelimit = -1;
				map.roundlimit = -1;
				map.visited = 0;

				Q_strncpyz(map.name, p, sizeof(map.name));
				*strstr(map.name, ".bsp") = 0;
				if (map.name[0]) {
					int     gt = (g_gametype.integer > 4 || g_gametype.integer == 2) ? 3 : g_gametype.integer;
					char*    types[6] = { "ffa", "tourney", "single", "team", "ctf", "clanarena" };
					char*    found = NULL;
					char*    type;

					info = G_GetArenaInfoByMap(map.name);
					if ((type = Info_ValueForKey(info, "type"))) {
						found = strstr(type, types[gt]);
						if (!found && g_gametype.integer == 5) {
							found = strstr(type, types[5]);
						}
					}

					if (found || (map_autoload.integer == 2 && !type) || map_autoload.integer > 2) {
						G_Printf("Map entry: \"%s\" (autofind add)\n", map.name);
						memcpy(&osp_maps[osp_numMaps], &map, sizeof(ospMap_t));
						osp_numMaps++;
					} else {
						G_Printf("Map entry: \"%s\" (autofind) - IGNORED - [type: %s] (looking for: %s)\n", map.name, Info_ValueForKey(info, "type"), types[gt]);
					}
				}
			}
			p += strlen(p) + 1;
		}
	}

	trap_Cvar_Set("mapsessionauto", va("%d", numFiles));
	trap_Cvar_Update(&mapsessionauto);
	OSP_SaveMapSession();
	G_Printf("%d maps added to maplist.\n\n", osp_numMaps);
}

/*
==================
OSP_MapExists

Map is in the maps\*.bsp file list (count names); the entry is marked so that map_autoload
skips it
==================
*/
int OSP_MapExists(char* list, int size, int count, char* map) {
	char*    p;
	int     i;
	char    filename[MAX_STRING_CHARS];

	p = list;
	Q_strncpyz(filename, va("%s.bsp", map), sizeof(filename));
	for (i = 0; i < count && osp_numMaps < MAX_OSP_MAPS; i++) {
		if (p && *p > 1 && !Q_stricmp(p, filename)) {
			*p = 1;
			return 1;
		}
		p += strlen(p) + 1;
	}
	return 0;
}

/*
==================
OSP_ParseMapEntry

Next maps.txt line ("<map> <min> <max> <fl> <cl> <tl> <rl>", "configmap <map> <min> <max>
<config>", "deletemap <map>"): 1 entry, -1 no entry, 100 entry on the last line, 0 end
==================
*/
int OSP_ParseMapEntry(char* buf, int size, ospMap_t* map) {
	static char* ptr = NULL;
	char    line[MAX_STRING_CHARS];
	int     len;

	if (!ptr) {
		ptr = buf;
	}
	line[0] = 0;
	ptr = OSP_ReadConfigLine(ptr, size - (ptr - buf), line, sizeof(line));

	if (line[0]) {
		char*    s1 = NULL;
		char*    s2 = NULL;
		char*    s3 = NULL;
		char*    s4 = NULL;
		char*    s5 = NULL;
		char*    s6 = NULL;
		char*    s7 = NULL;

		len = strlen(line);
		s1 = line;

		if (strstr(s1, "deletemap") == s1) {
			s1 = OSP_ParseTabs(line, len, qtrue, qtrue);
			s2 = OSP_ParseTabs(line, len, qfalse, qtrue);
			Q_strncpyz(map->name, s1, sizeof(map->name));
			map->maxPlayers = -32145;
			G_Printf("Delete Map: %s\n", map->name);
		} else if (strstr(s1, "configmap") == s1) {
			s1 = OSP_ParseTabs(line, len, qtrue, qtrue);
			s2 = OSP_ParseTabs(line, len, qfalse, qtrue);
			if (s1) {
				Q_strncpyz(map->name, s1, sizeof(map->name));
			}
			s3 = OSP_ParseTabs(line, len, qfalse, qtrue);
			if (s2) {
				map->minPlayers = atoi(s2);
			}
			s4 = OSP_ParseTabs(line, len, qfalse, qtrue);
			if (s3) {
				map->maxPlayers = atoi(s3);
			}
			if (s4) {
				Q_strncpyz(map->config, s4, sizeof(map->config));
			}
		} else {
			s2 = OSP_ParseTabs(line, len, qtrue, qtrue);
			if (s1) {
				Q_strncpyz(map->name, s1, sizeof(map->name));
			}
			s3 = OSP_ParseTabs(line, len, qfalse, qtrue);
			if (s2) {
				map->minPlayers = atoi(s2);
			}
			s4 = OSP_ParseTabs(line, len, qfalse, qtrue);
			if (s3) {
				map->maxPlayers = atoi(s3);
			}
			s5 = OSP_ParseTabs(line, len, qfalse, qtrue);
			if (s4) {
				map->fraglimit = atoi(s4);
			}
			s6 = OSP_ParseTabs(line, len, qfalse, qtrue);
			if (s5) {
				map->capturelimit = atoi(s5);
			}
			s7 = OSP_ParseTabs(line, len, qfalse, qtrue);
			if (s6) {
				map->timelimit = atoi(s6);
			}
			if (s7) {
				map->roundlimit = atoi(s7);
			}
		}
	}

	if (!ptr) {
		if (map->name[0]) {
			return 100;
		}
		return 0;
	}
	if (!map->name[0]) {
		return -1;
	}
	return 1;
}

/*
==================
OSP_ParseTabs

Tokenizer of a line: reset starts at buf, every call cuts the current token at the next tab
(or space) and returns the following one; NULL at the end
==================
*/
char* OSP_ParseTabs(char* buf, int size, qboolean reset, qboolean spaces) {
	static char* ptr = NULL;
	static char* last = NULL;
	char*    p, *q;

	if (!ptr || reset) {
		ptr = buf;
		last = NULL;
	}

	if (ptr - buf >= size) {
		return NULL;
	}
	while (*ptr == '\n' || *ptr == '\r') {
		*ptr = 0;
		ptr++;
		if (ptr - buf >= size) {
			return NULL;
		}
	}

	p = strchr(ptr, '\t');
	if (spaces) {
		if (!p) {
			p = strchr(ptr, ' ');
		} else {
			q = strchr(ptr, ' ');
			if (q < p) {
				p = q;
			}
		}
	}

	if (p) {
		*p = 0;
		p++;
		ptr = p;
		while (ptr - buf < size && (*ptr == '\t' || (spaces && *ptr == ' '))) {
			ptr++;
		}
	}

	if (ptr - buf >= size) {
		return NULL;
	}
	if (ptr == last) {
		ptr = buf + size;
		return NULL;
	}
	last = ptr;
	return ptr;
}

/*
==================
OSP_SaveMapSession

Saves the map list in the mapsessioncount / mapsession<n> cvars (OSP_LoadMapSession)
==================
*/
void OSP_SaveMapSession(void) {
	int     i;
	char    buf[MAX_STRING_CHARS];
	char*    cvar;

	OSP_SetMapSessionCount(osp_numMaps);
	for (i = 0; i < osp_numMaps; i++) {
		Com_sprintf(buf, sizeof(buf), "%i %i %i %i %i %i %i %s %s",
		            osp_maps[i].minPlayers, osp_maps[i].maxPlayers, osp_maps[i].fraglimit, osp_maps[i].capturelimit,
		            osp_maps[i].timelimit, osp_maps[i].roundlimit, osp_maps[i].visited,
		            osp_maps[i].config[0] ? osp_maps[i].config : "--none--", osp_maps[i].name);
		cvar = va("mapsession%i", i);
		trap_Cvar_Set(cvar, buf);
	}
}

/*
==================
OSP_GetMapFileList
==================
*/
int OSP_GetMapFileList(char* buf, int size) {
	return trap_FS_GetFileList("maps/", "bsp", buf, size);
}

/*
==================
OSP_FindMapFile

maps/<map>.bsp exists (callvote map)
==================
*/
int OSP_FindMapFile(char* map) {
	static int  numFiles = 0;
	char*    p;
	int     i;
	char    filename[MAX_OSPATH];

	if (!numFiles) {
		numFiles = OSP_GetMapFileList(osp_voteMapFileList, sizeof(osp_voteMapFileList) - 2);
	}
	p = osp_voteMapFileList;
	Q_strncpyz(filename, va("%s.bsp", map), sizeof(filename));
	for (i = 0; i < numFiles; i++) {
		if (p && *p && !Q_stricmp(p, filename)) {
			return 1;
		}
		p += strlen(p) + 1;
	}
	return 0;
}

/*
==================
OSP_LoadMapSession

Map list from the mapsessioncount / mapsession<n> cvars (written by OSP_SaveMapSession); 0 when
there is none or the maps\*.bsp file count changed (map_autoload)
==================
*/
int OSP_LoadMapSession(qboolean verbose) {
	char*    p;
	char*    s;
	int     i;
	char    buf[MAX_STRING_CHARS];
	char*    cvar;
	int     autoload;
	int     numFiles;

	autoload = trap_Cvar_VariableIntegerValue("map_autoload");
	numFiles = trap_Cvar_VariableIntegerValue("mapsessionauto");
	trap_Cvar_VariableStringBuffer("mapsessioncount", buf, sizeof(buf));
	if (!Q_stricmp(buf, "none") || (autoload && numFiles != OSP_GetMapFileList(osp_mapFileList, sizeof(osp_mapFileList)))) {
		return 0;
	}

	osp_numMaps = atoi(buf);
	for (i = 0; i < osp_numMaps; i++) {
		cvar = va("mapsession%i", i);
		trap_Cvar_VariableStringBuffer(cvar, buf, sizeof(buf));
		s = buf;

		p = strchr(s, ' ');
		*p = 0;
		p++;
		osp_maps[i].minPlayers = atoi(s);
		s = p;
		p = strchr(s, ' ');
		*p = 0;
		p++;
		osp_maps[i].maxPlayers = atoi(s);
		s = p;
		p = strchr(s, ' ');
		*p = 0;
		p++;
		osp_maps[i].fraglimit = atoi(s);
		s = p;
		p = strchr(s, ' ');
		*p = 0;
		p++;
		osp_maps[i].capturelimit = atoi(s);
		s = p;
		p = strchr(s, ' ');
		*p = 0;
		p++;
		osp_maps[i].timelimit = atoi(s);
		s = p;
		p = strchr(s, ' ');
		*p = 0;
		p++;
		osp_maps[i].roundlimit = atoi(s);
		s = p;
		p = strchr(s, ' ');
		*p = 0;
		p++;
		osp_maps[i].visited = atoi(s);
		s = p;
		p = strchr(s, ' ');
		*p = 0;
		p++;
		strcpy(osp_maps[i].config, s);
		s = p;
		if (!Q_stricmp(osp_maps[i].config, "--none--")) {
			osp_maps[i].config[0] = 0;
		}
		strcpy(osp_maps[i].name, s);

		if (verbose) {
			G_Printf("map(%d): %s (config: %s), min:%d, max:%d, fl:%d, cl:%d, tl:%d, rl: %d, visited:%d\n",
			         i, osp_maps[i].name, osp_maps[i].config[0] ? osp_maps[i].config : "none",
			         osp_maps[i].minPlayers, osp_maps[i].maxPlayers, osp_maps[i].fraglimit, osp_maps[i].capturelimit,
			         osp_maps[i].timelimit, osp_maps[i].roundlimit, osp_maps[i].visited);
		}
	}
	return 1;
}

/*
==================
OSP_ValidateMap

Nonzero when map (a name, or a number of the "maplist") is in the map list; a number is
replaced by the name. setIndex makes it the next map (OSP_SetMapIndex)
==================
*/
int OSP_ValidateMap(gentity_t* ent, char* map, qboolean setIndex) {
	int     i;

	if (!OSP_LoadMapSession(qfalse)) {
		OSP_LoadMaplist();
	}

	if (!osp_numMaps) {
		if (ent) {
			trap_SendServerCommand(ent - g_entities, "print \"Sorry, no maps available!\n\"");
		}
		return 0;
	}

	for (i = 0; i < osp_numMaps; i++) {
		if (!strcmp(map, osp_maps[i].name)) {
			if (setIndex) {
				OSP_SetMapIndex(i);
			}
			return 1;
		}
	}

	i = atoi(map);
	if (i > 0 && i <= osp_numMaps) {
		strcpy(map, osp_maps[i - 1].name);
		if (setIndex) {
			OSP_SetMapIndex(i - 1);
		}
		return 1;
	}

	if (ent && ent->client) {
		trap_SendServerCommand(ent - g_entities, va("print \"Sorry, map: ^3%s^7 is not available!\n\"", map));
	}
	return 0;
}

/*
==================
OSP_Cmd_MapList_f

"maplist": the map list in three columns
==================
*/
void OSP_Cmd_MapList_f(gentity_t* ent) {
	int     i;
	int     rows;

	if (!OSP_LoadMapSession(qfalse)) {
		OSP_LoadMaplist();
	}

	if (!osp_numMaps) {
		if (ent && ent->client) {
			trap_SendServerCommand(ent - g_entities, "print \"Sorry, no maps available!\n\"");
		}
		return;
	}

	trap_SendServerCommand(ent - g_entities, "print \"\n^3Available maps:\n---------------^7\n\"");
	rows = osp_numMaps / 3;
	if (osp_numMaps % 3) {
		rows++;
	}

	for (i = 0; i < rows; i++) {
		if (i + rows * 2 + 1 <= osp_numMaps) {
			trap_SendServerCommand(ent - g_entities, va("print \"%2d. ^2%-12s^7 %2d. ^2%-12s^7 %2d. ^2%-12s\n\"",
			                       i + 1, osp_maps[i].name, i + rows + 1, osp_maps[i + rows].name, i + rows * 2 + 1, osp_maps[i + rows * 2].name));
		} else if (i + rows + 1 <= osp_numMaps) {
			trap_SendServerCommand(ent - g_entities, va("print \"%2d. ^2%-12s^7 %2d. ^2%-12s\n\"",
			                       i + 1, osp_maps[i].name, i + rows + 1, osp_maps[i + rows].name));
		} else {
			trap_SendServerCommand(ent - g_entities, va("print \"%2d. ^2%-12s\n\"", i + 1, osp_maps[i].name));
		}
	}
	trap_SendServerCommand(ent - g_entities, "print \"\n\"");
}

/*
==================
OSP_MapRestart

Restarts the map (map_restart), switches the mode first when one was voted (level.nextMode),
or reloads it ("vstr nextmap") when level.mapReloadNeeded is set
==================
*/
void OSP_MapRestart(void) {
	int     i;
	char    buf[MAX_STRING_CHARS];

	for (i = 0; i < level.maxclients; i++) {
		level.clients[i].readyToExit = 0;
		level.clients[i].ps.stats[STAT_CLIENTS_READY] = 0;
		level.clients[i].ps.weaponTime = 0;
		level.clients[i].ps.weaponstate = 0;
	}

	if (level.nextMode > 0) {
		char*    s;

		s = va("MapRestart:\tConfig_Change\tSetting to mode %d\n", level.nextMode - 1);
		if (!level.startMode) {
			level.voteTime = 0;
			LogExit(s);
			return;
		}
		OSP_AdminLogPrintf(s);
		OSP_SetGametype(level.nextMode - 1);
	}

	OSP_HidePlayersInServerinfo(0);
	if (!level.mapReloadNeeded) {
		OSP_AdminLogPrintf("MapRestart:\tSameMap (0)");
		trap_SendConsoleCommand(EXEC_APPEND, "map_restart 0\n");
	} else {
		trap_Cvar_VariableStringBuffer("nextmap", buf, sizeof(buf));
		OSP_AdminLogPrintf(va("MapRestart:\tvstr nextmap\t%s", buf));
		trap_SendConsoleCommand(EXEC_APPEND, "vstr nextmap\n");
	}
}

/*
==================
OSP_SetMapIndex

Makes osp_maps[index] the next map (OSP_NextMap)
==================
*/
void OSP_SetMapIndex(int index) {
	osp_nextMapIndex = index;
	osp_mapIndexSet = 1;
}

/*
==================
OSP_SetMapSessionCount
==================
*/
void OSP_SetMapSessionCount(int count) {
	char*    s;

	if (!count) {
		s = "none";
	} else {
		s = va("%d", count);
	}
	trap_Cvar_Set("mapsessioncount", s);
	trap_Cvar_Update(&mapsessioncount);
}
