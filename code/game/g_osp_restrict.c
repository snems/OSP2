/*
 * g_osp_restrict.c: player name restrictions (player_file: the names allowed
 * to play, with an optional team name; player_always: a reserved tag), ban filters
 * (filter_file: banplayer / banname, bantag, banaddr, banpass lines), name normalisation
 * (color codes removed) and file-name sanitising.
 */

#include "g_local.h"

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t	player_always;
vmCvar_t	player_file;
vmCvar_t	filter_file;
ospFilter_t	osp_filters[MAX_OSP_FILTERS];

// OSP: the player_file entries
typedef struct {
	int		team;		// TEAM_RED / TEAM_BLUE from the team name after a TAB, 0 = any team
	char	name[MAX_NETNAME];	// normalized player name
} ospPlayerName_t;

#define	MAX_OSP_PLAYERNAMES	450

// "^Xrrggbb" hex colors (as g_cmds.c's SanitizeString)
#define OSP_IS_HEX( c )	( ( ( c ) >= '0' && ( c ) <= '9' ) || ( ( c ) >= 'A' && ( c ) <= 'F' ) || ( ( c ) >= 'a' && ( c ) <= 'f' ) )

ospPlayerName_t	osp_playerNames[MAX_OSP_PLAYERNAMES];

int		osp_numPlayerNames = 0;		// entries loaded from player_file (OSP_ReadPlayerNameFile; team names "-vs-" in OSP_DemoName)

// OSP: what OSP_CheckFilters passes to the OSP_FilterMatch* functions
typedef struct {
	int			clientNum;
	char		*infoName;		// normalized userinfo "name"
	char		*name;			// normalized name being checked
	char		*ip;			// sess.ip, else userinfo "ip"
	char		*password;		// userinfo "password"
	char		*ospPassword;	// userinfo "osp_password"
	char		*userinfo;
	ospFilter_t	*filter;		// the filter being tested
	char		*banType;		// admin log: "NameBan", "TagBan", "AddrBan", "PasswordBan"
	char		*banValue;		// admin log: the filter's name / address / password
} ospFilterCheck_t;

void	OSP_ReadPlayerNameFile( void );
void	OSP_LoadFilters( void );
int		OSP_ParseFilterLine( char *buf, int size, ospFilter_t *filter );
int		OSP_CheckFilters( char *name, int clientNum, qboolean nameOnly );
int		OSP_FilterMatchName( ospFilterCheck_t *check );
int		OSP_FilterMatchTag( ospFilterCheck_t *check );
int		OSP_FilterMatchAddr( ospFilterCheck_t *check );
int		OSP_FilterMatchPass( ospFilterCheck_t *check );

/*
==================
OSP_LoadPlayerNames

"playernames" console command and OSP_InitGame
==================
*/
void OSP_LoadPlayerNames( void ) {
	int		i;

	OSP_ReadPlayerNameFile();
	if ( g_gametype.integer >= GT_TEAM && !log_default.integer ) {
		for ( i = TEAM_RED; i <= TEAM_BLUE; i++ ) {
			G_LogPrintf( "TeamName: %d %s\n", i, osp_teamInfo[i].teamName );
		}
	}
}

/*
==================
OSP_ReadPlayerNameFile

player_file: one player name per line, in team modes optionally followed by a TAB and the
team name (the first team name seen becomes the red team's name, any other the blue one's).
Also loads the ban filters.
==================
*/
void OSP_ReadPlayerNameFile( void ) {
	char			*p;
	int				len;
	char			buf[MAX_OSP_FILE_CHARS];
	char			filename[MAX_QPATH];
	fileHandle_t	f;

	osp_numPlayerNames = 0;
	filename[0] = 0;
	strcpy( osp_teamInfo[TEAM_RED].teamName, "RED" );
	strcpy( osp_teamInfo[TEAM_BLUE].teamName, "BLUE" );
	OSP_LoadFilters();

	if ( player_file.string[0] && Q_stricmp( player_file.string, "none" ) ) {
		strcpy( filename, player_file.string );
	} else {
		G_Printf( "\n*** Player name restrictions DISABLED.\n\n" );
		return;
	}

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		G_Printf( "\n*** ERROR: Could not open PlayerName file \"%s\"\n", filename );
		G_Printf( "*** Player name restrictions DISABLED.\n\n" );
		return;
	}
	if ( len >= sizeof( buf ) ) {
		G_Printf( "\n*** PlayerName file \"%s\" too big! (%d bytes, max = %d)\n", filename, len, sizeof( buf ) - 1 );
		G_Printf( "*** Player name restrictions DISABLED.\n\n" );
		trap_FS_FCloseFile( f );
		return;
	}

	if ( f ) {
		char	line[MAX_STRING_CHARS];

		G_Printf( "\nLoading Player Names from \"%s\"\n", filename );
		trap_FS_Read( buf, len, f );
		p = buf;
		buf[len] = 0;
		do {
			line[0] = 0;
			p = OSP_ReadConfigLine( p, len - ( p - buf ), line, sizeof( line ) );
			if ( line[0] ) {
				osp_playerNames[osp_numPlayerNames].team = 0;
				if ( g_gametype.integer >= GT_TEAM ) {
					char	*s;

					s = strchr( line, '\t' );
					if ( s ) {
						*s = 0;
						s++;
						if ( !osp_teamInfo[TEAM_RED].teamName[0] || !Q_stricmp( osp_teamInfo[TEAM_RED].teamName, "RED" ) ||
							!Q_stricmp( s, osp_teamInfo[TEAM_RED].teamName ) ) {
							Q_strncpyz( osp_teamInfo[TEAM_RED].teamName, s, sizeof( osp_teamInfo[TEAM_RED].teamName ) );
							osp_playerNames[osp_numPlayerNames].team = TEAM_RED;
						} else {
							Q_strncpyz( osp_teamInfo[TEAM_BLUE].teamName, s, sizeof( osp_teamInfo[TEAM_BLUE].teamName ) );
							osp_playerNames[osp_numPlayerNames].team = TEAM_BLUE;
						}
					}
				}
				Q_strncpyz( osp_playerNames[osp_numPlayerNames].name, line, sizeof( osp_playerNames[0].name ) );
				G_Printf( "Playername: %s (team = %s)\n", osp_playerNames[osp_numPlayerNames].name,
					( !osp_playerNames[osp_numPlayerNames].team ) ? "NONE" : osp_teamInfo[osp_playerNames[osp_numPlayerNames].team].teamName );
				osp_numPlayerNames++;
			}
		} while ( p && osp_numPlayerNames < MAX_OSP_PLAYERNAMES );
	}

	if ( player_always.string[0] ) {
		G_Printf( ">>> Reserved playertag is: \"%s\"\n", player_always.string );
	}
	G_Printf( "%d PlayerNames added to accept list.\n\n", osp_numPlayerNames );
	trap_FS_FCloseFile( f );
}

/*
==================
OSP_ClientFilterResult

6 when a connected client changes his name again within 5 seconds, else result
==================
*/
int OSP_ClientFilterResult( int clientNum, int result ) {
	gclient_t	*cl;

	cl = &level.clients[clientNum];
	if ( cl->pers.connected == CON_CONNECTED ) {
		if ( cl->pers.cmdDelayTime >= level.time ) {
			return 6;
		}
	}
	level.clients[clientNum].pers.cmdDelayTime = level.time + 5000;
	return result;
}

/*
==================
OSP_CheckClientFilters

0 refused, 1 accepted, 2 accepted as a spectator only (QTV / GTV relays, the reserved
player_always tag, "speconly" players, "scoreboard"), > 2 refused by a filter (see
OSP_CheckFilters), 6 name changed too fast. With player_file set, only the listed names
may play. nameOnly skips the address and password filters.
==================
*/
int OSP_CheckClientFilters( char *name, int clientNum, qboolean nameOnly ) {
	int		i;
	char	buf[256];

	// SRV-01: bound the copy by the output buffer, not by the (client-controlled) input length
	OSP_NormalizeName( name, sizeof( buf ), buf, qfalse );
	if ( strstr( buf, "QTV-" ) == buf || strstr( buf, "GTV-" ) == buf ||
		( player_always.string[0] && strstr( buf, player_always.string ) == buf ) ) {
		return 2;
	}
	if ( level.clients[clientNum].pers.connected && level.clients[clientNum].sess.specOnly ) {
		return 2;
	}

	i = OSP_CheckFilters( buf, clientNum, nameOnly );
	if ( i == 1 ) {
		if ( !osp_numPlayerNames || !player_file.string[0] || !Q_stricmp( player_file.string, "none" ) ) {
			return OSP_ClientFilterResult( clientNum, 1 );
		}
		if ( !Q_stricmp( buf, "scoreboard" ) ) {
			return OSP_ClientFilterResult( clientNum, 2 );
		}
		for ( i = 0; i < osp_numPlayerNames; i++ ) {
			if ( !Q_stricmp( buf, osp_playerNames[i].name ) ) {
				return OSP_ClientFilterResult( clientNum, 1 );
			}
		}
		return 0;
	}
	return i;
}

/*
==================
OSP_NormalizeName

Copies at most len characters of in to out without color codes ("^c", "^Xrrggbb") and
control characters, lowercased with toLower; returns the length of out.
==================
*/
int OSP_NormalizeName( char *in, int len, char *out, qboolean toLower ) {
	int		i;
	char	*start;

	// SRV-01: len is the capacity of out (including the terminating NUL); never write past it.
	// The input bound (i < len) is kept so existing callers behave as before.
	if ( len <= 0 ) {
		return 0;
	}
	i = 0;
	start = out;
	while ( *in && i < len && ( out - start ) < len - 1 ) {
		if ( *in == 27 || *in == Q_COLOR_ESCAPE ) {
			in++;
			i++;
			if ( ( *in == 'X' || *in == 'x' ) &&
				in[1] && OSP_IS_HEX( in[1] ) && in[2] && OSP_IS_HEX( in[2] ) &&
				in[3] && OSP_IS_HEX( in[3] ) && in[4] && OSP_IS_HEX( in[4] ) &&
				in[5] && OSP_IS_HEX( in[5] ) && in[6] && OSP_IS_HEX( in[6] ) ) {
				in += 7;
				i += 7;
				continue;
			}
			if ( *in ) {
				in++;
				i++;
				continue;
			}
			break;
		}
		if ( *in < 32 ) {
			in++;
			i++;
			continue;
		}
		if ( toLower )
			*out++ = tolower( *in++ );
		else
			*out++ = *in++;
		i++;
	}

	*out = 0;
	return out - start;
}

/*
==================
OSP_SanitizeFilename

Like OSP_NormalizeName, but also drops the characters that are invalid in file names and
spaces, and doubles quotes
==================
*/
void OSP_SanitizeFilename( char *in, int len, char *out ) {
	int		i;

	i = 0;
	while ( *in && i < len ) {
		if ( *in == 27 || *in == Q_COLOR_ESCAPE ) {
			in++;
			i++;
			if ( ( *in == 'X' || *in == 'x' ) &&
				in[1] && OSP_IS_HEX( in[1] ) && in[2] && OSP_IS_HEX( in[2] ) &&
				in[3] && OSP_IS_HEX( in[3] ) && in[4] && OSP_IS_HEX( in[4] ) &&
				in[5] && OSP_IS_HEX( in[5] ) && in[6] && OSP_IS_HEX( in[6] ) ) {
				in += 7;
				i += 7;
				continue;
			}
			if ( *in ) {
				in++;
				i++;
				continue;
			}
			break;
		}
		if ( *in == '\\' || *in == '/' || *in == ':' || *in == '"' || *in == '*' ||
			*in == '?' || *in == '<' || *in == '>' || *in == '|' ) {
			in++;
			i++;
			continue;
		}
		if ( *in == '\'' ) {
			*out++ = '\'';
			*out++ = '\'';
			in++;
			i++;
			continue;
		}
		if ( *in <= ' ' ) {
			in++;
			i++;
			continue;
		}
		*out++ = *in++;
		i++;
	}

	*out = 0;
}

/*
==================
OSP_StripSpaces

Copies in to out without spaces and control characters (unused)
==================
*/
void OSP_StripSpaces( char *in, char *out ) {
	while ( *in ) {
		if ( *in <= ' ' ) {
			in++;
			continue;
		}
		*out++ = *in++;
	}
	*out = 0;
}

/*
==================
OSP_LoadFilters

filter_file (see OSP_ParseFilterLine)
==================
*/
void OSP_LoadFilters( void ) {
	int				len;
	char			buf[MAX_OSP_FILE_CHARS];
	fileHandle_t	f;
	char			filename[MAX_QPATH];

	level.numFilters = 0;
	filename[0] = 0;

	if ( filter_file.string[0] && Q_stricmp( filter_file.string, "none" ) ) {
		strcpy( filename, filter_file.string );
	} else {
		return;
	}

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len < 0 || !f ) {
		G_Printf( "\nERROR: Could not open player filter file \"%s\"\n", filename );
		G_Printf( "*** Player filters DISABLED.\n\n" );
		return;
	}
	if ( len >= sizeof( buf ) ) {	// one byte is needed for the terminating NUL
		G_Printf( "\nPlayer filter file \"%s\" too big! (file_size: %d, max_size: %d)\n", filename, len, sizeof( buf ) - 1 );
		G_Printf( "*** Player filters DISABLED.\n\n" );
		trap_FS_FCloseFile( f );
		return;
	}

	if ( f ) {
		ospFilter_t	filter;
		int			res;

		G_Printf( "\nLoading player filters from \"%s\"\n", filename );
		trap_FS_Read( buf, len, f );
		buf[len] = 0;
		do {
			filter.osp_unk_0 = 0;
			filter.type = 0;
			strcpy( filter.name, "none" );
			strcpy( filter.ip, "none" );
			strcpy( filter.password, "none" );
			res = OSP_ParseFilterLine( buf, len, &filter );
			if ( res >= 1 && filter.type ) {
				if ( filter.type == 1 ) {
					G_Printf( "Ban entry: [NAME] name=%s, addr=%s, pass=%s\n", filter.name, filter.ip, filter.password );
				} else if ( filter.type == 2 ) {
					G_Printf( "Ban entry: [TAG]   tag=%s, addr=%s, pass=%s\n", filter.name, filter.ip, filter.password );
				} else if ( filter.type == 3 ) {
					G_Printf( "Ban entry: [ADDR] name=%s, addr=%s, pass=%s\n", filter.name, filter.ip, filter.password );
				} else if ( filter.type == 5 ) {
					G_Printf( "Ban entry: [PASS] name=%s, addr=%s, pass=%s\n", filter.name, filter.ip, filter.password );
				} else {
					G_Printf( "Ban entry: [UNKNOWN] name=%s, addr=%s, pass=%s\n", filter.name, filter.ip, filter.password );
				}
				memcpy( &osp_filters[level.numFilters], &filter, sizeof( ospFilter_t ) );
				level.numFilters++;
			}
		} while ( res != 0 && res != 100 && level.numFilters < MAX_OSP_FILTERS );

		if ( level.numFilters >= MAX_OSP_FILTERS ) {
			G_Printf( va( "WARNING: Filter list at maximum %d entries!\n", MAX_OSP_FILTERS ) );
		}
		trap_FS_FCloseFile( f );
		G_Printf( "%d player filters added.\n\n", level.numFilters );
	} else {
		G_Printf( "\nERROR: Could not open maps list file \"%s\"\n", filename );
		G_Printf( "*** Player filters DISABLED.\n\n" );
	}
}

/*
==================
OSP_ParseFilterLine

Parses the next line of buf into filter: "banplayer" / "banname" (type 1), "bantag" (2),
"banaddr" (3) or "banpass" (5), then TAB separated name, address and password ("none" or
missing = not tested). Returns 1 for a parsed line, 0 / 100 at the end of the buffer
(100 with a last entry), -1 for a bad line.
==================
*/
int OSP_ParseFilterLine( char *buf, int size, ospFilter_t *filter ) {
	static char	*p = NULL;
	char		line[MAX_STRING_CHARS];
	int			ret;
	int			type;
	int			len;

	ret = 0;
	if ( !p ) {
		p = buf;
	}
	line[0] = 0;
	type = 0;
	p = OSP_ReadConfigLine( p, size - ( p - buf ), line, sizeof( line ) );
	if ( line[0] ) {
		char	*cmd;
		char	*name, *ip, *password;

		len = strlen( line );
		cmd = line;
		if ( strstr( cmd, "banplayer" ) == cmd || strstr( cmd, "banname" ) == cmd ) {
			type = 1;
		} else if ( strstr( cmd, "bantag" ) == cmd ) {
			type = 2;
		} else if ( strstr( cmd, "banaddr" ) == cmd ) {
			type = 3;
		} else if ( strstr( cmd, "banpass" ) == cmd ) {
			type = 5;
		} else if ( *cmd ) {
			G_Printf( ">>> Unknown filter command: \"%s\"\n", cmd );
			return -1;
		}
		filter->type = type;

		name = OSP_ParseTabs( line, len, qtrue, qfalse );
		if ( name == line ) {
			G_Printf( ">>> No paramters for command \"%s\" (missing TABs?)\n", line );
			return -1;
		}
		ip = OSP_ParseTabs( line, len, qfalse, qfalse );
		if ( name ) {
			OSP_NormalizeName( name, sizeof( filter->name ), filter->name, qtrue );
		}
		password = OSP_ParseTabs( line, len, qfalse, qfalse );
		if ( ip ) {
			Q_strncpyz( filter->ip, ip, sizeof( filter->ip ) );
		}
		if ( password ) {
			Q_strncpyz( filter->password, password, sizeof( filter->password ) );
		}
	}

	if ( !p && !type ) {
		return 0;
	}
	if ( !line[0] ) {
		return -1;
	}

	ret = -1;
	if ( !Q_stricmp( filter->name, "none" ) && !Q_stricmp( filter->ip, "none" ) && !Q_stricmp( filter->password, "none" ) ) {
		G_Printf( ">>> No paramters for command \"%s\"\n", line );
		return ret;
	}
	if ( filter->type == 1 && ( !filter->name[0] || !Q_stricmp( filter->name, "none" ) ) ) {
		G_Printf( ">>> NAME not defined for name filter: \"%s\"\n", line );
		return ret;
	}
	if ( filter->type == 2 && ( !filter->name[0] || !Q_stricmp( filter->name, "none" ) ) ) {
		G_Printf( ">>> TAG not defined for tag filter: \"%s\"\n", line );
		return ret;
	}
	if ( filter->type == 3 && ( !filter->ip[0] || !Q_stricmp( filter->ip, "none" ) ) ) {
		G_Printf( ">>> ADDRESS not defined for address filter: \"%s\"\n", line );
		return ret;
	}
	if ( filter->type == 5 && ( !filter->password[0] || !Q_stricmp( filter->password, "none" ) ) ) {
		G_Printf( ">>> PASSWORD not defined for password filter: \"%s\"\n", line );
		return ret;
	}

	if ( !p ) {
		if ( Q_stricmp( filter->name, "none" ) || Q_stricmp( filter->ip, "none" ) || Q_stricmp( filter->password, "none" ) ) {
			return 100;
		}
		return 0;
	}
	return 1;
}

/*
==================
OSP_CheckFilters

1 accepted, 0 refused (inappropriate name, name or address ban), 4 / 5 refused by an
address / tag filter, password filters: 4 when no listed password matches. Every check
is written to the admin log. nameOnly skips the address and password filters.
==================
*/
int OSP_CheckFilters( char *name, int clientNum, qboolean nameOnly ) {
	// register: matches OSP's frame layout
	register int		i;
	register int		res;
	register int		passRes;
	ospFilterCheck_t	check;
	char				normName[64];
	char				ip[64];
	int					passAccepted;
	int					ret;
	char				password[64];
	char				ospPassword[64];
	char				userinfo[MAX_INFO_STRING];
	char				infoName[64];

	passAccepted = 0;
	OSP_NormalizeName( name, MAX_NETNAME, normName, qtrue );
	if ( !strlen( normName ) ) {
		return 0;
	}
	if ( !level.numFilters ) {
		return 1;
	}
	if ( g_entities[clientNum].r.svFlags & SVF_BOT ) {
		return 1;
	}

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	OSP_NormalizeName( Info_ValueForKey( userinfo, "name" ), MAX_NETNAME, infoName, qtrue );
	Q_strncpyz( ip, level.clients[clientNum].sess.ip, sizeof( ip ) );
	if ( !ip[0] ) {
		Q_strncpyz( ip, Info_ValueForKey( userinfo, "ip" ), sizeof( ip ) );
		if ( !ip[0] ) {
			return 0;	// no address to check against the filters: refuse, do not skip them
		}
	}
	Q_strncpyz( password, Info_ValueForKey( userinfo, "password" ), 64 );
	Q_strncpyz( ospPassword, Info_ValueForKey( userinfo, "osp_password" ), 64 );

	check.infoName = infoName;
	check.name = normName;
	check.ip = ip;
	check.password = password;
	check.ospPassword = ospPassword;
	check.userinfo = userinfo;
	check.clientNum = clientNum;

	if ( strstr( normName, "nigger" ) || strstr( normName, "n1gger" ) || strstr( normName, "kkk" ) ) {
		OSP_AdminLogPrintf( va( "NameBan:\tINAPPROPRIATE\t%s", normName ) );
		return 0;
	}

	ret = 1;
	for ( i = 0; i < level.numFilters && i < MAX_OSP_FILTERS; i++ ) {
		res = 0;
		passRes = 0;
		check.filter = &osp_filters[i];
		check.banType = "Unknown";
		check.banValue = "Unknown";
		if ( osp_filters[i].type == 1 ) {
			res = OSP_FilterMatchName( &check );
			if ( res != 0 && res != 2 ) {
				check.banType = "NameBan";
				check.banValue = osp_filters[i].name;
			}
		} else if ( osp_filters[i].type == 2 ) {
			res = OSP_FilterMatchTag( &check );
			if ( res != 0 && res != 2 ) {
				check.banType = "TagBan";
				check.banValue = osp_filters[i].name;
			}
		} else if ( ( osp_filters[i].type == 3 || osp_filters[i].type == 4 ) && !nameOnly ) {
			res = OSP_FilterMatchAddr( &check );
			if ( res != 0 && res != 2 ) {
				check.banType = "AddrBan";
				check.banValue = osp_filters[i].ip;
			}
		} else if ( osp_filters[i].type == 5 && !nameOnly ) {
			passRes = OSP_FilterMatchPass( &check );
			if ( res != 0 && res != 2 ) {	// OSP bug: tests res, not passRes
				check.banType = "PasswordBan";
				check.banValue = osp_filters[i].password;
			}
		}

		if ( res != 0 ) {
			if ( res == 2 ) {
				OSP_AdminLogPrintf( va( "FilterAccept:\tName=%s\tAddr=%s\tPass=%s\tOSPPass=%s", normName, ip, password, ospPassword ) );
				return 1;
			}
			OSP_AdminLogPrintf( va( "%s:\tFilter=%s\tFilterID=%d\tName=%s\tAddr=%s\tPass=%s\tOSPPass=%s",
				check.banType, check.banValue, i + 1, normName, ip, password, ospPassword ) );
			return res - 1;
		}
		if ( passRes == 2 ) {
			passAccepted = 1;
			ret = 1;
		} else if ( passRes != 0 && !passAccepted ) {
			ret = passRes - 1;
		}
	}

	if ( ret != 1 ) {
		OSP_AdminLogPrintf( va( "FilterDeny:\tName=%s\tAddr=%s\tPass=%s\tOSPPass=%s\t(%d)", normName, ip, password, ospPassword, ret ) );
	} else {
		OSP_AdminLogPrintf( va( "FilterAccept:\tName=%s\tAddr=%s\tPass=%s\tOSPPass=%s", normName, ip, password, ospPassword ) );
	}
	return ret;
}

/*
==================
OSP_FilterMatchName

1 when the name equals the filter's name, unless the address or one of the passwords
matches the filter's
==================
*/
int OSP_FilterMatchName( ospFilterCheck_t *check ) {
	if ( !*check->filter->name || !Q_stricmp( check->filter->name, "none" ) ) {
		return 0;
	}
	if ( !*check->name ) {
		return 1;
	}
	if ( !Q_stricmp( check->filter->name, check->name ) ) {
		if ( !( *check->ip && Q_stricmp( check->filter->ip, "none" ) && strstr( check->ip, check->filter->ip ) == check->ip ) &&
			!( *check->password && Q_stricmp( check->filter->password, "none" ) && !Q_stricmp( check->password, check->filter->password ) ) &&
			!( *check->ospPassword && Q_stricmp( check->filter->password, "none" ) && !Q_stricmp( check->ospPassword, check->filter->password ) ) ) {
			return 1;
		}
	}
	return 0;
}

/*
==================
OSP_FilterMatchTag

6 when the name contains the filter's tag, unless the address or one of the passwords
matches the filter's
==================
*/
int OSP_FilterMatchTag( ospFilterCheck_t *check ) {
	if ( !*check->filter->name || !Q_stricmp( check->filter->name, "none" ) ) {
		return 0;
	}
	if ( !*check->name ) {
		return 6;
	}
	if ( strstr( check->name, check->filter->name ) &&
		!( *check->ip && Q_stricmp( check->filter->ip, "none" ) && strstr( check->ip, check->filter->ip ) == check->ip ) &&
		!( *check->password && Q_stricmp( check->filter->password, "none" ) && !Q_stricmp( check->password, check->filter->password ) ) &&
		!( *check->ospPassword && Q_stricmp( check->filter->password, "none" ) && !Q_stricmp( check->ospPassword, check->filter->password ) ) ) {
		return 6;
	}
	return 0;
}

/*
==================
OSP_FilterMatchAddr

4 when the address starts with the filter's, unless the name or one of the passwords
matches the filter's (automatic bans, type 4, always match)
==================
*/
int OSP_FilterMatchAddr( ospFilterCheck_t *check ) {
	if ( !check->filter->ip[0] || !Q_stricmp( check->filter->ip, "none" ) ) {
		return 0;
	}
	if ( check->ip && strstr( check->ip, check->filter->ip ) == check->ip ) {
		if ( check->filter->type == 4 ) {
			return 4;
		}
		if ( !( check->name && *check->name && Q_stricmp( check->filter->name, "none" ) && !Q_stricmp( check->filter->name, check->name ) ) &&
			!( *check->password && Q_stricmp( check->filter->password, "none" ) && !Q_stricmp( check->password, check->filter->password ) ) &&
			!( *check->ospPassword && Q_stricmp( check->filter->password, "none" ) && !Q_stricmp( check->ospPassword, check->filter->password ) ) ) {
			return 4;
		}
	}
	return 0;
}

/*
==================
OSP_FilterMatchPass

2 (accepted) when a password, the name or the address matches the filter's, else 5
==================
*/
int OSP_FilterMatchPass( ospFilterCheck_t *check ) {
	if ( !*check->filter->password || !Q_stricmp( check->filter->password, "none" ) ) {
		return 0;
	}
	if ( ( *check->password && !Q_stricmp( check->password, check->filter->password ) ) ||
		( *check->ospPassword && !Q_stricmp( check->ospPassword, check->filter->password ) ) ) {
		return 2;
	}
	if ( ( *check->name && Q_stricmp( check->filter->name, "none" ) && !Q_stricmp( check->filter->name, check->name ) ) ||
		( *check->ip && Q_stricmp( check->filter->ip, "none" ) && strstr( check->ip, check->filter->ip ) == check->ip ) ) {
		return 2;
	}
	return 5;
}

/*
==================
OSP_PlayerNameTeam

team of the userinfo's name in player_file (0 when not listed or no team), stored in
client->sess.nameTeam
==================
*/
int OSP_PlayerNameTeam( gclient_t *client, char *userinfo ) {
	int		i;
	char	buf[256];
	char	*s;

	if ( !osp_numPlayerNames ) {
		return 0;
	}

	s = Info_ValueForKey( userinfo, "name" );
	// SRV-01: same overflow as OSP_CheckClientFilters
	OSP_NormalizeName( s, sizeof( buf ), buf, qfalse );
	for ( i = 0; i < osp_numPlayerNames; i++ ) {
		if ( !Q_stricmp( buf, osp_playerNames[i].name ) ) {
			if ( client ) {
				client->sess.nameTeam = osp_playerNames[i].team;
			}
			G_Printf( "Added: %s to team: %d\n", s, osp_playerNames[i].team );
			return osp_playerNames[i].team;
		}
	}
	return 0;
}
