/*
 * g_osp_config.c: modes (osp_modes), their default settings and map lists,
 * custom modes (mode_customfile), the start / default mode (mode_start, z_m_default) and the
 * "settings" command. OSP_ReadConfigLine is the line reader of every OSP config file loader.
 */

#include "g_local.h"

// OSP: functions of other files used here
extern  void    G_UpdateCvars(void);     // g_main.c

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t    server_availmodes;
vmCvar_t    mode_start;
vmCvar_t    z_m_default;
vmCvar_t    mode_customfile;
vmCvar_t    z_m_current;

// OSP: g_main.c's cvar table (OSP_Cmd_Settings_f walks it; not static in OSP).
// The entry layout is g_main.c's cvarTable_t.
typedef struct {
	vmCvar_t*    vmCvar;
	char*        cvarName;
	char*        defaultString;
	int         cvarFlags;
	int         modificationCount;
	qboolean    trackChange;
	qboolean    osp_setDefault;
	qboolean    teamShader;
} cvarTable_t;
extern  cvarTable_t gameCvarTable[];
extern  int         gameCvarTableSize;

// OSP's modes, indexed by g_gametype (name used in the log lines)
ospMode_t   osp_modes[] = {
	{ GT_FFA,       "Free-For-All" },
	{ GT_TOURNAMENT,    "1v1" },
	{ GT_TEAM,      "ClanBase TeamDM" },
	{ GT_TEAM,      "TeamplayDM" },
	{ GT_CTF,       "CTF" },
	{ 5,            "ClanArena" },
	{ GT_TEAM,      "FreezeTag TDM" },
	{ GT_TEAM,      "FreezeTag TDM (Vanilla)" }
};
int         osp_numModes = sizeof(osp_modes) / sizeof(osp_modes[0]);      // ExitLevel: modes above it are custom modes

// OSP: per-mode settings, applied to every mode whose bit (1 << mode) is in
// modeMask
typedef struct {
	int     modeMask;
	char*    cvar;
	char*    value;
} ospModeSetting_t;

// map settings (OSP_LoadModeMaplist)
ospModeSetting_t    osp_modeMapSettings[] = {
	{ 0x01, "map_rotate", "1" },
	{ 0x3e, "map_rotate", "0" },
	{ 0x01, "map_file", "cfg-maps/ffamaps.txt" },
	{ 0x02, "map_file", "cfg-maps/1v1maps.txt" },
	{ 0x04, "map_file", "cfg-maps/cbmaps.txt" },
	{ 0xc8, "map_file", "cfg-maps/teammaps.txt" },
	{ 0x10, "map_file", "cfg-maps/ctfmaps.txt" },
	{ 0x20, "map_file", "cfg-maps/clanarenamaps.txt" }
};

// everything else (OSP_SetGametype; OSP_Cmd_Settings_f colours the values
// that differ from them)
ospModeSetting_t    osp_modeSettings[] = {
	{ 0x10, "capturelimit", "0" },
	{ 0x20, "dmflags", "8" },
	{ 0x01, "fraglimit", "30" },
	{ 0xde, "fraglimit", "0" },
	{ 0x20, "fraglimit", "7" },
	{ 0x40, "freezetag_respawn", "7.0" },
	{ 0x80, "freezetag_respawn", "8.0" },
	{ 0x02, "g_forcerespawn", "10" },
	{ 0xc4, "g_forcerespawn", "1" },
	{ 0xb0, "g_friendlyFire", "0" },
	{ 0xde, "g_warmup", "0" },
	{ 0x20, "g_warmup", "60" },
	{ 0x40, "g_weaponteamrespawn", "10" },
	{ 0x80, "g_weaponteamrespawn", "5" },
	{ 0xce, "item_disable", "4" },
	{ 0x20, "item_disable", "1023" },
	{ 0x01, "map_rotate", "1" },
	{ 0x3e, "map_rotate", "0" },
	{ 0x40, "match_dropitems", "0" },
	{ 0x80, "match_dropitems", "1" },
	{ 0x01, "match_latejoin", "1" },
	{ 0x06, "match_mutespecs", "1" },
	{ 0x4c, "match_overtime", "1" },
	{ 0x93, "match_overtime", "0" },
	{ 0xcc, "match_overtimecount", "0" },
	{ 0xcc, "match_overtimelength", "5" },
	{ 0xfe, "match_readypercent", "100" },
	{ 0x02, "match_timeoutcount", "3" },
	{ 0xdc, "match_timeoutcount", "3" },
	{ 0xdc, "match_timeoutlength", "210" },
	{ 0x20, "match_timeoutcount", "1" },
	{ 0x01, "powerup_disable", "64" },
	{ 0x22, "powerup_disable", "255" },
	{ 0x10, "powerup_disable", "16" },
	{ 0x20, "roundlimit", "7" },
	{ 0x04, "server_customclient", "15" },
	{ 0x3f, "server_freezetag", "0" },
	{ 0x40, "server_freezetag", "1" },
	{ 0x80, "server_freezetag", "2" },
	{ 0xdc, "server_maxpacketsmin", "30" },
	{ 0x04, "server_maxpacketsmax", "60" },
	{ 0xd8, "server_maxpacketsmax", "125" },
	{ 0xc0, "server_realping", "1" },
	{ 0xdc, "server_timenudgemin", "-20" },
	{ 0x04, "server_timenudgemax", "20" },
	{ 0xcc, "start_bullets", "50" },
	{ 0x20, "start_bullets", "25" },
	{ 0x20, "start_armor", "200" },
	{ 0x20, "start_shells", "20" },
	{ 0x20, "start_rockets", "80" },
	{ 0x20, "start_grenades", "15" },
	{ 0x20, "start_lightning", "150" },
	{ 0x20, "start_slugs", "15" },
	{ 0x20, "start_cells", "100" },
	{ 0x20, "start_bfg", "0" },
	{ 0xff, "sv_allowdownload", "1" },
	{ 0xff, "sv_floodprotect", "0" },
	{ 0x20, "team_armordamage", "0" },
	{ 0x0c, "team_maxplayers", "4" },
	{ 0xc0, "team_maxplayers", "5" },
	{ 0x01, "timelimit", "20" },
	{ 0x02, "timelimit", "15" },
	{ 0xdc, "timelimit", "20" },
	{ 0x20, "timelimit", "0" },
	{ 0xcc, "warmup_armor", "100" },
	{ 0x20, "weapon_deaddrop", "0" },
	{ 0x20, "weapon_have", "127" },
	{ 0x20, "weapon_start", "8" }
};

// map lists used when a mode's map_file gives none (OSP_LoadModeMaplist);
// session is a "mapsession<n>" value: min players, max players, fraglimit, capturelimit,
// timelimit, roundlimit, visited, config, map
typedef struct {
	int     modeMask;
	char*    session;
} ospDefaultMap_t;

ospDefaultMap_t osp_defaultMaps[] = {
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm1" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm2" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm3" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm4" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm5" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm6" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm7" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm8" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm9" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm10" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm11" },
	{ 0x01, "0 0 30 0 20 -1 1 --none-- ospdm12" },
	{ 0x01, "0 7 25 0 20 -1 0 --none-- q3dm2" },
	{ 0x01, "0 8 25 0 20 -1 0 --none-- q3dm3" },
	{ 0x01, "0 6 25 0 15 -1 0 --none-- q3dm4" },
	{ 0x01, "0 6 25 0 15 -1 0 --none-- q3dm5" },
	{ 0x01, "4 14 25 0 20 -1 0 --none-- q3dm6" },
	{ 0x01, "4 99 30 0 20 -1 0 --none-- q3dm7" },
	{ 0x01, "2 14 -1 -1 -1 -1 0 --none-- q3dm8" },
	{ 0x01, "2 14 -1 -1 -1 -1 0 --none-- q3dm9" },
	{ 0x01, "3 14 -1 -1 -1 -1 0 --none-- q3dm10" },
	{ 0x01, "3 14 -1 -1 -1 -1 0 --none-- q3dm11" },
	{ 0x01, "4 99 25 0 20 -1 0 --none-- q3dm12" },
	{ 0x01, "4 10 30 0 20 -1 0 --none-- q3dm13" },
	{ 0x01, "4 14 -1 -1 -1 -1 0 --none-- q3dm14" },
	{ 0x01, "4 14 -1 -1 -1 -1 0 --none-- q3dm15" },
	{ 0x01, "3 99 -1 -1 -1 -1 0 --none-- q3dm16" },
	{ 0x01, "4 99 -1 -1 -1 -1 0 --none-- q3dm17" },
	{ 0x01, "99 99 20 0 20 -1 0 --none-- q3dm18" },
	{ 0x01, "99 99 20 0 20 -1 0 --none-- q3dm19" },
	{ 0x01, "0 6 20 0 10 -1 0 --none-- q3tourney2" },
	{ 0x01, "0 7 20 0 10 -1 0 --none-- q3tourney3" },
	{ 0x01, "0 7 20 0 10 -1 0 --none-- q3tourney4" },
	{ 0x01, "0 5 20 0 10 -1 0 --none-- q3tourney5" },
	{ 0x01, "0 14 30 0 20 -1 0 --none-- pro-q3dm6" },
	{ 0x01, "0 14 30 0 20 -1 0 --none-- pro-q3dm13" },
	{ 0x01, "0 14 30 0 20 -1 0 --none-- pro-q3tourney2" },
	{ 0x01, "0 14 30 0 20 -1 0 --none-- pro-q3tourney4" },
	{ 0x02, "0 0 0 0 15 -1 1 --none-- ospdm1" },
	{ 0x02, "0 0 0 0 15 -1 1 --none-- ospdm2" },
	{ 0x02, "0 0 0 0 15 -1 1 --none-- ospdm3" },
	{ 0x02, "0 0 0 0 15 -1 1 --none-- ospdm4" },
	{ 0x02, "0 0 0 0 15 -1 1 --none-- ospdm7" },
	{ 0x02, "0 0 0 0 15 -1 1 --none-- ospdm8" },
	{ 0x02, "0 0 0 0 15 -1 1 --none-- ospdm9" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3dm13" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3dm6" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3dm17" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- pro-q3dm6" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- pro-q3dm13" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- pro-q3tourney2" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- pro-q3tourney4" },
	{ 0x02, "99 99 0 0 15 -1 0 --none-- q3dm1" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3dm2" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3dm3" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3dm4" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3dm5" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3dm7" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3tourney2" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3tourney3" },
	{ 0x02, "0 0 0 0 15 -1 0 --none-- q3tourney4" },
	{ 0x10, "0 12 0 0 20 -1 1 --none-- ospctf1" },
	{ 0x10, "0 12 0 0 20 -1 1 --none-- ospctf2" },
	{ 0x10, "0 10 0 0 20 -1 1 --none-- q3ctf1" },
	{ 0x10, "0 12 0 0 20 -1 0 --none-- q3ctf2" },
	{ 0x10, "0 12 0 0 20 -1 0 --none-- q3ctf3" },
	{ 0x10, "99 99 0 0 20 -1 0 --none-- q3ctf4" },
	{ 0x10, "0 20 0 0 20 -1 0 --none-- q3wctf1" },
	{ 0x10, "0 20 0 0 20 -1 0 --none-- q3wctf2" },
	{ 0x10, "0 20 0 0 20 -1 0 --none-- q3wctf3" },
	{ 0x10, "99 99 0 0 20 -1 0 --none-- q3tourney6_ctf" },
	{ 0x04, "0 20 -1 -1 20 -1 0 --none-- cpm4" },
	{ 0x04, "0 20 -1 -1 20 -1 0 --none-- ospdm5" },
	{ 0x04, "0 20 -1 -1 20 -1 0 --none-- ospdm6" },
	{ 0x04, "0 20 -1 -1 20 -1 0 --none-- ospdm12" },
	{ 0x04, "0 20 -1 -1 20 -1 0 --none-- q3dm14" },
	{ 0x04, "0 20 -1 -1 20 -1 0 --none-- q3dm14tmp" },
	{ 0x20, "0 30 -1 -1 -1 7 0 --none-- ospca1" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- ospdm1" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- ospdm2" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- ospdm4" },
	{ 0xe8, "0 20 -1 -1 20 -1 0 --none-- ospdm5" },
	{ 0xe8, "0 20 -1 -1 20 -1 0 --none-- ospdm6" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- ospdm11" },
	{ 0xe8, "0 20 -1 -1 20 -1 0 --none-- ospdm12" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- pro-q3dm6" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm6" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm7" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm8" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm9" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm10" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm11" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm12" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm13" },
	{ 0xe8, "0 20 -1 -1 20 -1 0 --none-- q3dm14" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm15" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm16" },
	{ 0xec, "0 20 -1 -1 20 -1 0 --none-- q3dm17" }
};

void OSP_PrintAvailableModes(qboolean noCustom);


/*
==================
OSP_LoadCustomModes

Lists the standard modes enabled by server_availmodes and reads the custom modes of
mode_customfile ("<name><TAB><config file>" lines, at most MAX_OSP_CUSTOMMODES) for voting
==================
*/
void OSP_LoadCustomModes(void) {
	int             len;
	char*            p;
	int             count;
	char            buf[MAX_OSP_FILE_CHARS];
	char            filename[MAX_QPATH];
	fileHandle_t    f;

	buf[0] = 0;
	filename[0] = 0;
	level.numVoteModes = 0;
	level.numStdModes = 0;
	strcpy(level.modeList, "\n   DISABLED");

	for (len = 0; len < osp_numModes; len++) {
		if (server_availmodes.integer & (1 << len)) {
			if (!level.numVoteModes) {
				strcpy(level.modeList, va("\n   %d - %s\n", len, osp_modes[len].name));
			} else {
				strcat(level.modeList, va("   %d - %s\n", len, osp_modes[len].name));
			}
			level.numVoteModes++;
			level.numStdModes++;
		}
	}

	if (mode_customfile.string[0] && Q_stricmp(mode_customfile.string, "none")) {
		strcpy(filename, mode_customfile.string);
	} else {
		OSP_PrintAvailableModes(qtrue);
		return;
	}

	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if (len <= 0) {
		G_Printf("\n*** ERROR: Could not open custom mode file \"%s\"\n", filename);
		OSP_PrintAvailableModes(qtrue);
		return;
	}
	if (len >= sizeof(buf)) {
		G_Printf("\n*** ERROR: Custom mode file \"%s\" too big! (%d bytes, max = %d)\n", filename, len, sizeof(buf) - 1);
		trap_FS_FCloseFile(f);
		OSP_PrintAvailableModes(qtrue);
		return;
	}

	if (f) {
		G_Printf("\nLoading custom modes from \"%s\"\n", filename);
		trap_FS_Read(buf, len, f);
		p = buf;
		buf[len] = 0;
		count = 0;

		do {
			char    line[MAX_STRING_CHARS];

			line[0] = 0;
			p = OSP_ReadConfigLine(p, len - (p - buf), line, sizeof(line));
			if (line[0]) {
				char*    s;

				s = strchr(line, '\t');
				if (!s) {
					G_Printf("** Missing TAB delimiter on line: \"%s\" ... skipping.\n", line);
					continue;
				}
				*s = 0;
				s++;
				if (strchr(line, ' ')) {
					G_Printf("** Space found in \"%s\" ... skipping.\n", line);
					continue;
				}

				Q_strncpyz(level.customModeName[count], line, sizeof(level.customModeName[0]));
				Q_strncpyz(level.customModeConfig[count], s, sizeof(level.customModeConfig[0]));
				if (!level.numVoteModes) {
					strcpy(level.modeList, va("\n   %d - %s [CUSTOM]\n", count + osp_numModes, level.customModeName[count]));
				} else {
					strcat(level.modeList, va("   %d - %s [CUSTOM]\n", count + osp_numModes, level.customModeName[count]));
				}
				G_Printf("Custom mode entry: %-12s (%s)\n", va("\"%s\"", level.customModeName[count]), level.customModeConfig[count]);
				count++;
				level.numVoteModes++;
			}
		} while (p && level.numVoteModes < level.numStdModes + MAX_OSP_CUSTOMMODES);
	}

	G_Printf("%d alternate modes available for voting.\n", level.numVoteModes);
	OSP_PrintAvailableModes(qfalse);
	trap_FS_FCloseFile(f);
}

/*
==================
OSP_ReadConfigLine

Copies the next line of buf (size bytes left) that is not empty or a "#" comment into line
(lineSize bytes, CR LF or LF line ends). Returns the position after the line, NULL at the end
of buf. A line that does not fit is skipped (line stays empty).
==================
*/
char* OSP_ReadConfigLine(char* buf, int size, char* line, int lineSize) {
	char*    p;
	char*    s;
	int     len;

	len = size;
	s = buf;
	p = buf;
	if (len < 1) {
		return NULL;
	}

	while (*p == '#' || *p == '\n') {
		p = strchr(s, '\n');
		if (p) {
			s = p + 1;
			len -= p - s;
			if (strchr(s, '\r') == s) {
				s++;
				len--;
			}
		} else {
			return NULL;
		}
		p = s;
		if (len < 1) {
			return NULL;
		}
	}

	p = strchr(s, '\n');
	if (!p) {
		p = s;
	}

	memset(line, 0, lineSize);
	if (p - s > lineSize) {
		G_Printf("Line too long.. skipping\n");
	} else if (p == s) {
		if (len > lineSize) {
			G_Printf("Last line too long.. skipping\n");
		} else {
			memcpy(line, s, len);
		}
	} else {
		if (p < s) {
			return NULL;
		}
		memcpy(line, s, p - s);
		line[p - s] = 0;
		if (line[p - s - 1] == '\r') {
			line[p - s - 1] = 0;
		}
	}

	if (p == buf) {
		return NULL;
	}
	p++;
	if (*p == '\r') {
		p++;
	}
	return p;
}

/*
==================
OSP_SetGametype

Applies the default settings of mode (osp_modeSettings) after resetting the cvars
==================
*/
void OSP_SetGametype(int mode) {
	ospModeSetting_t*    def;
	int                 i;
	int                 num;
	int                 mask;

	OSP_ResetCvarDefaults();
	if (mode >= osp_numModes || mode < 0) {
		return;
	}

	mask = 1 << mode;
	num = sizeof(osp_modeSettings) / sizeof(osp_modeSettings[0]);
	for (i = 0, def = osp_modeSettings; i < num; i++, def++) {
		if (def->modeMask & mask) {
			trap_Cvar_Set(def->cvar, def->value);
			G_Printf("set %s %s\n", def->cvar, def->value);
		}
	}

	OSP_HidePlayersInServerinfo(0);
	G_UpdateCvars();
	OSP_SaveDefaultSettings();

	if (osp_modes[mode].gametype != g_gametype.integer) {
		G_Printf("*** Setting ^3\"g_gametype\"^7 to ^6\"%d\"\n", osp_modes[mode].gametype);
	}
}

/*
==================
OSP_LoadModeMaplist

Map settings and map list of mode (the default list of the mode when its map_file gives
none); the next map is map (NULL or "": the current map) when it is in the list, else the
first map of the list
==================
*/
void OSP_LoadModeMaplist(int mode, char* map) {
	ospModeSetting_t*    def;
	int                 i;
	int                 num;
	int                 mask;
	char*                nextmap;
	char                mapname[MAX_STRING_CHARS];

	OSP_SetMapSessionCount(0);
	if (mode >= osp_numModes) {
		trap_Cvar_Set("z_m_current", va("%d", mode));
		return;
	}

	mask = 1 << mode;
	num = sizeof(osp_modeMapSettings) / sizeof(osp_modeMapSettings[0]);
	for (i = 0, def = osp_modeMapSettings; i < num; i++, def++) {
		if (def->modeMask & mask) {
			trap_Cvar_Set(def->cvar, def->value);
			G_Printf("Setting ^3\"%s\"^7 to ^6\"%s\"\n", def->cvar, def->value);
		}
	}

	G_UpdateCvars();
	OSP_LoadMaplist();

	if (!osp_numMaps) {
		int     autoload;

		autoload = map_autoload.integer;
		G_Printf("****\n**** NULL MAP LIST ... reverting to map_autoload 2\n****\n\n");
		trap_Cvar_Set("map_autoload", "2");
		G_UpdateCvars();
		OSP_LoadMaplist();
		trap_Cvar_Set("map_autoload", va("%d", autoload));
	}

	if (!osp_numMaps) {
		ospDefaultMap_t* dm;
		int             n;

		n = 0;
		G_Printf("\n*** Loading default maplist for: %s\n\n", osp_modes[mode].name);
		num = sizeof(osp_defaultMaps) / sizeof(osp_defaultMaps[0]);
		for (i = 0, dm = osp_defaultMaps; i < num; i++, dm++) {
			if (dm->modeMask & mask) {
				trap_Cvar_Set(va("mapsession%d", n), dm->session);
				n++;
			}
		}
		OSP_SetMapSessionCount(n);
		OSP_LoadMapSession(qtrue);
	}

	nextmap = map;
	if (!nextmap || !*nextmap) {
		char    serverinfo[MAX_INFO_STRING];

		mapname[0] = 0;
		trap_GetServerinfo(serverinfo, sizeof(serverinfo));
		Q_strncpyz(mapname, Info_ValueForKey(serverinfo, "mapname"), sizeof(mapname));
		if (mapname[0]) {
			nextmap = mapname;
		}
	}

	if (nextmap && *nextmap && OSP_ValidateMap(NULL, nextmap, 0)) {
		trap_SendConsoleCommand(EXEC_APPEND, va("set nextmap \"map %s\"\n", nextmap));
	} else if (mode != g_gametype.integer || mode != z_m_current.integer) {
		trap_SendConsoleCommand(EXEC_APPEND, va("set nextmap \"map %s\"\n", osp_maps[0].name));
	}

	if (!level.startMode) {
		level.mapReloadNeeded = 1;
	}
	trap_Cvar_Set("z_m_current", va("%d", mode));
}

/*
==================
OSP_DefaultModeLoad

Think of the "delay_configload" entity: loads the start mode (level.startMode - 1) once,
then checks that the server runs the OSP pak (sv_referencedPaks)
==================
*/
void OSP_DefaultModeLoad(gentity_t* ent) {
	if (level.startMode > 0 && z_m_default.integer < 0) {
		int     mode;

		mode = level.startMode - 1;
		G_Printf("><><>< Default mode \"%s\" enabled. ><><><\n", OSP_ModeName(mode));
		level.nextMode = level.startMode;
		trap_Cvar_Set("z_m_default", va("%d", mode));
		trap_Cvar_Update(&z_m_default);
		OSP_LoadModeMaplist(mode, NULL);

		if (mode >= osp_numModes) {
			if (OSP_CustomModeConfig(mode)[0]) {
				OSP_HidePlayersInServerinfo(0);
				OSP_SetMapSessionCount(0);
				trap_SendConsoleCommand(EXEC_NOW, va("exec %s\n", OSP_CustomModeConfig(mode)));
			} else {
				G_Printf(">> Default mode %d is not a loaded custom mode, ignored.\n", mode);
			}
		} else {
			trap_Cvar_Set("g_gametype", va("%d", osp_modes[mode].gametype));
			trap_Cvar_Update(&g_gametype);
			trap_Cvar_Set("z_m_default", va("%d", mode));
			trap_Cvar_Update(&z_m_default);
			OSP_MapRestart();
		}
	}

	ent->nextthink = 0;
	ent->think = NULL;
	G_FreeEntity(ent);

	// "Invalid server content": sv_referencedPaks must contain " 1690942878 " (both strings
	// are obfuscated by subtracting 0x17 / 7 from every character)
	{
		int     i;
		char    key[32];
		char    cvar[32];
		char    paks[MAX_CVAR_VALUE_STRING];

		strcpy(key, "7HMPGPKIONO7");
		strcpy(cvar, "z}fylmlylujlkWhrz");
		for (i = 0; i < strlen(key); i++)
			key[i] -= 0x17;
		for (i = 0; i < strlen(cvar); i++)
			cvar[i] -= 7;
		trap_Cvar_VariableStringBuffer(cvar, paks, 255);
		paks[255] = 0;
		if (!strstr(paks, key)) {
			G_Error("^3*** Invalid server content^7\n");
		}
	}
}

/*
==================
OSP_DelayConfigLoad

Spawns an invisible "delay_configload" entity whose think (OSP_DefaultModeLoad) runs at time
==================
*/
void OSP_DelayConfigLoad(int time) {
	gentity_t*   te;

	te = G_Spawn();
	te->s.eType = ET_ITEM;
	te->r.svFlags |= SVF_NOCLIENT;
	te->s.eFlags |= EF_NODRAW;
	te->classname = "delay_configload";
	te->nextthink = time;
	te->think = OSP_DefaultModeLoad;
	trap_LinkEntity(te);
}

/*
==================
OSP_PrintAvailableModes

Prints the vote modes and schedules the start mode (mode_start, unless z_m_default is set)
==================
*/
void OSP_PrintAvailableModes(qboolean noCustom) {
	if (!level.numVoteModes) {
		G_Printf("\n*** Alternate mode voting DISABLED.\n\n");
		return;
	}

	if (noCustom) {
		G_Printf("\nAvailable alternate modes:%s(no custom modes defined)\n\n", level.modeList);
	} else {
		G_Printf("\nAvailable alternate modes:%s\n", level.modeList);
	}

	level.startMode = 0;
	if (mode_start.integer >= 0) {
		if (z_m_default.integer == -1) {
			if (!OSP_ModeAvailable(mode_start.integer)) {
				G_Printf(">>> Start/Default mode \"%s\" out of range... ignored\n\n", mode_start.string);
			} else {
				G_Printf(">>> Start/Default mode set to: %s\n\n", OSP_ModeName(mode_start.integer));
				level.startMode = mode_start.integer + 1;
				OSP_DelayConfigLoad(level.time + 2500);
			}
		}
	} else {
		G_Printf(">>> Start/Default mode: DISABLED\n\n");
	}
}

/*
==================
OSP_ModeName
==================
*/
char* OSP_ModeName(int mode) {
	if (mode < 0) {
		return va("%s", "UNKNOWN");
	}
	if (mode < osp_numModes) {
		return va("%s", osp_modes[mode].name);
	}
	if (mode - osp_numModes >= MAX_OSP_CUSTOMMODES) {
		return va("%s", "UNKNOWN");
	}
	return va("%s", level.customModeName[mode - osp_numModes]);
}

/*
==================
OSP_CustomModeConfig

Config file of custom mode (osp_numModes + i); "" when mode is not a loaded custom mode.
==================
*/
char* OSP_CustomModeConfig(int mode) {
	int     i;

	i = mode - osp_numModes;
	if (i < 0 || i >= MAX_OSP_CUSTOMMODES || i >= level.numVoteModes - level.numStdModes) {
		return "";
	}
	return level.customModeConfig[i];
}

/*
==================
OSP_ModeAvailable

Nonzero when mode is a standard mode enabled in server_availmodes or a loaded custom mode.
The mode number comes from a vote argument: no shift by an out-of-range count, and custom
modes are bounded by the loaded count and the table size (the original accepted one too many).
==================
*/
int OSP_ModeAvailable(int mode) {
	if (mode < 0) {
		return 0;
	}
	if (mode < osp_numModes) {
		return (server_availmodes.integer & (1 << mode)) ? 1 : 0;
	}
	return OSP_CustomModeConfig(mode)[0] ? 1 : 0;
}

/*
==================
OSP_Cmd_Settings_f

"settings": the server settings of the current mode, six per "sset" line, at most 20 lines;
red values differ from the mode's defaults
==================
*/
void OSP_Cmd_Settings_f(gentity_t* ent) {
	ospModeSetting_t*    def;
	cvarTable_t*         cv;
	int                 j;
	int                 num;
	int                 mask;
	char*                color;
	int                 i;
	char*                ospauth;
	char*                name;
	char                buf[512];
	int                 count = 0;
	int                 lines = 0;
	int                 mode;

	ospauth = "server_ospauth";
	lines = 0;
	count = 0;
	buf[0] = 0;

	mode = (z_m_current.integer >= 0) ? z_m_current.integer : g_gametype.integer;
	mask = 1 << mode;
	num = sizeof(osp_modeSettings) / sizeof(osp_modeSettings[0]);

	trap_SendServerCommand(ent - g_entities, va("print \"\n^3Current Server Settings for: ^7%s\n====================================================================\n\"", osp_modes[mode].name));

	for (i = 0, cv = gameCvarTable; i < gameCvarTableSize && lines < 20; i++, cv++) {
		if (!cv->vmCvar || (cv->cvarFlags == CVAR_SERVERINFO && Q_stricmp(cv->cvarName, ospauth)) || i < 20 ||
		        strstr(cv->cvarName, "vote_allow") || strstr(cv->cvarName, "map_") == cv->cvarName ||
		        !Q_stricmp(cv->cvarName, "sv_dcount")) {
			continue;
		}

		color = "2";
		for (j = 0, def = osp_modeSettings; j < num; j++, def++) {
			if ((def->modeMask & mask) && !Q_stricmp(cv->cvarName, def->cvar)) {
				if (Q_stricmp(cv->vmCvar->string, def->value)) {
					color = "1";
				}
				break;
			}
		}

		if (j >= num) {
			if (!cv->osp_setDefault && Q_stricmp(cv->cvarName, ospauth)) {
				continue;
			}
			if (!Q_stricmp(cv->cvarName, ospauth)) {
				color = (cv->vmCvar->integer != 1) ? "1" : "3";
			} else if (Q_stricmp(cv->vmCvar->string, cv->defaultString)) {
				color = "1";
			}
		}

		if (!Q_stricmp(cv->cvarName, ospauth)) {
			name = "SERVER_OSPAUTH";
		} else {
			name = cv->cvarName;
			if (strstr(cv->cvarName, "match_")) {
				name += 6;
			} else if (strstr(cv->cvarName, "g_") == cv->cvarName) {
				name += 2;
			}
		}

		strcat(buf, va("^%s%s %s ", color, name, cv->vmCvar->string));
		if (++count == 6) {
			trap_SendServerCommand(ent - g_entities, va("sset %s\n", buf));
			count = 0;
			buf[0] = 0;
			lines++;
		}
	}

	if (buf[0]) {
		trap_SendServerCommand(ent - g_entities, va("sset %s\n", buf));
	}
}
