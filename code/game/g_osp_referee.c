/*
 * g_osp_referee.c: referee commands: "ref <password>" makes a referee
 * (OSP_Cmd_Ref_f), "ref <cmd>" / "r_<cmd>" run the referee commands (OSP_RefCommand), and the
 * server console reuses pause / allready / ban. The code is close to Wolfenstein: ET's
 * g_referee.c, which comes from OSP.
 */

#include "g_local.h"

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t	ref_password;

// OSP: this file's initialised data is OSP_Ref_Pause's two statics.

void	LogExit( const char *string );		// g_main.c
void	OSP_ResetAllClientStats( void );	// g_osp_main.c


/*
==================
OSP_RefCommand

Referee commands ("r_<cmd>", or "ref <cmd>" with argOffset 1); nonzero when cmd was one
==================
*/
int OSP_RefCommand( gentity_t *ent, char *cmd, int argOffset ) {
	if ( !Q_stricmp( cmd, "r_pause" ) || !Q_stricmp( cmd, "pause" ) ) {
		OSP_Ref_Pause( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_kick" ) ) {
		OSP_Ref_Kick( ent, argOffset );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_map" ) ) {
		OSP_Ref_Map( ent, argOffset );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_ban" ) || !Q_stricmp( cmd, "ban" ) ) {
		OSP_Ref_Ban( ent, argOffset );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_abort" ) || !Q_stricmp( cmd, "abort" ) ) {
		OSP_Ref_Abort( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_help" ) ) {
		OSP_Ref_Help( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_allcaptain" ) || !Q_stricmp( cmd, "allcaptain" ) ) {
		OSP_Ref_AllCaptain( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_allready" ) || !Q_stricmp( cmd, "allready" ) ) {
		OSP_Ref_AllReady( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_lock" ) ) {
		if ( !osp_teamInfo[TEAM_RED].locked ) {
			OSP_Ref_Lock( ent, TEAM_RED );
		}
		if ( !osp_teamInfo[TEAM_BLUE].locked ) {
			OSP_Ref_Lock( ent, TEAM_BLUE );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_unlock" ) ) {
		if ( osp_teamInfo[TEAM_RED].locked ) {
			OSP_Ref_Lock( ent, TEAM_RED );
		}
		if ( osp_teamInfo[TEAM_BLUE].locked ) {
			OSP_Ref_Lock( ent, TEAM_BLUE );
		}
		return 1;
	}
	// OSP bug kept: these test the team lock, not the spectator lock
	if ( !Q_stricmp( cmd, "r_speclock" ) ) {
		if ( !osp_teamInfo[TEAM_RED].locked ) {
			OSP_Ref_SpecLock( ent, TEAM_RED );
		}
		if ( !osp_teamInfo[TEAM_BLUE].locked ) {
			OSP_Ref_SpecLock( ent, TEAM_BLUE );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_specunlock" ) ) {
		if ( osp_teamInfo[TEAM_RED].locked ) {
			OSP_Ref_SpecLock( ent, TEAM_RED );
		}
		if ( osp_teamInfo[TEAM_BLUE].locked ) {
			OSP_Ref_SpecLock( ent, TEAM_BLUE );
		}
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_lockred" ) || !Q_stricmp( cmd, "r_unlockred" ) || !Q_stricmp( cmd, "lockred" ) || !Q_stricmp( cmd, "unlockred" ) ) {
		OSP_Ref_Lock( ent, TEAM_RED );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_lockblue" ) || !Q_stricmp( cmd, "r_unlockblue" ) || !Q_stricmp( cmd, "lockblue" ) || !Q_stricmp( cmd, "unlockblue" ) ) {
		OSP_Ref_Lock( ent, TEAM_BLUE );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_speclockred" ) || !Q_stricmp( cmd, "r_specunlockred" ) || !Q_stricmp( cmd, "speclockred" ) || !Q_stricmp( cmd, "specunlockred" ) ) {
		OSP_Ref_SpecLock( ent, TEAM_RED );
		return 1;
	}
	// OSP typo kept: "r_specklockblue"
	if ( !Q_stricmp( cmd, "r_specklockblue" ) || !Q_stricmp( cmd, "r_specunlockblue" ) || !Q_stricmp( cmd, "speclockblue" ) || !Q_stricmp( cmd, "specunlockblue" ) ) {
		OSP_Ref_SpecLock( ent, TEAM_BLUE );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_mutespecs" ) || !Q_stricmp( cmd, "mutespecs" ) ) {
		OSP_Ref_MuteSpecs( ent );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_promote" ) || !Q_stricmp( cmd, "promote" ) ) {
		OSP_Ref_Promote( ent, argOffset );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_putred" ) || !Q_stricmp( cmd, "putred" ) ) {
		OSP_Ref_Put( ent, TEAM_RED, argOffset );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_putblue" ) || !Q_stricmp( cmd, "putblue" ) ) {
		OSP_Ref_Put( ent, TEAM_BLUE, argOffset );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_remove" ) || !Q_stricmp( cmd, "remove" ) ) {
		OSP_Ref_Remove( ent, argOffset );
		return 1;
	}
	if ( !Q_stricmp( cmd, "r_restart" ) || !Q_stricmp( cmd, "restart" ) ) {
		OSP_Ref_Restart( ent );
		return 1;
	}
	if ( strstr( cmd, "r_" ) == cmd ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"Unknown referee command: ^2%s^7\n\"", cmd ) );
		trap_SendServerCommand( ent - g_entities, "print \"Type ^3r_help^7 for a list of valid referee commands.\n\"" );
		return 1;
	}

	return 0;
}

/*
==================
OSP_Cmd_Ref_f

"ref <password>": becomes a referee; for a referee "ref <cmd>" runs an OSP, referee or
vote command (fValue: ClientCommand passes 0)
==================
*/
void OSP_Cmd_Ref_f( gentity_t *ent, qboolean fValue ) {
	char	arg[MAX_TOKEN_CHARS];

	if ( ent->client->sess.referee ) {
		if ( fValue ) {
			return;
		}
		trap_Argv( 1, arg, sizeof( arg ) );
		if ( OSP_ClientCommand( ent, arg, qtrue ) ) {
			return;
		}
		if ( OSP_RefCommand( ent, arg, 1 ) ) {
			return;
		}
		if ( Cmd_CallVote_f( ent, qtrue ) ) {
			return;
		}
		OSP_Ref_Help( ent );
		return;
	}

	if ( !Q_stricmp( ref_password.string, "none" ) || !ref_password.string[0] ) {
		trap_SendServerCommand( ent - g_entities, "print \"Sorry, referee status disabled on this server.\n\"" );
		return;
	}
	if ( trap_Argc() < 2 ) {
		trap_SendServerCommand( ent - g_entities, "print \"Usage: ref [password]\n\"" );
		return;
	}
	trap_Argv( 1, arg, sizeof( arg ) );
	if ( Q_stricmp( arg, ref_password.string ) ) {
		trap_SendServerCommand( ent - g_entities, "print \"Invalid referee password!\n\"" );
		OSP_AdminLogPrintf( va( "Referee_Attempt_Fail\t%s\t(%s)", ent->client->pers.netname, arg ) );
		return;
	}

	ent->client->sess.referee = 1;
	ent->client->sess.specInvite = TEAM_RED | TEAM_BLUE;
	trap_SendServerCommand( ent - g_entities, "print \"Type: ^3ref ?^7 for a list of referee commands.\n\"" );
	trap_SendServerCommand( -1, va( "cp \"%s\n^3has become a referee\n\"", ent->client->pers.netname ) );
	OSP_AdminLogPrintf( va( "Referee_Attempt_Success\t%s\t(%s)", ent->client->pers.netname, arg ) );
}

/*
==================
OSP_Ref_Help
==================
*/
void OSP_Ref_Help( gentity_t *ent ) {
	if ( !ent->client->sess.referee ) {
		trap_SendServerCommand( ent - g_entities, "print \"Sorry, you are not a referee!\n" );
		return;
	}

	trap_SendServerCommand( ent - g_entities, "print \"\n^3Referee commands:^7\n\"" );
	trap_SendServerCommand( ent - g_entities, "print \"-----------------\n\"" );
	OSP_Vote_Help( ent, 0 );
	trap_SendServerCommand( ent - g_entities, "print \"\n^5abort            ^5promote^7 <p_id>   ^5restart\n\"" );
	trap_SendServerCommand( ent - g_entities, "print \"^5allready         ^5putblue^7 <p_id>   ^5speclock\n\"" );
	trap_SendServerCommand( ent - g_entities, "print \"^5ban ^7<p_id>       ^5putred^7 <p_id>    ^5specunlock\n\"" );
	trap_SendServerCommand( ent - g_entities, "print \"^5lock             ^5remove^7 <p_id>    ^5unlock\n\"" );
	trap_SendServerCommand( ent - g_entities, "print \"^5pause\n\"" );
	trap_SendServerCommand( ent - g_entities, "print \"\nUsage: ^3\\ref <cmd> [params]\n\n\"" );
}

/*
==================
OSP_Ref_Pause

Pauses / unpauses the match (ent NULL: server console, "Admin"). osp_matchPaused is the
pauser's client number + 10001 (1000 for the console). An unpause starts the 5 s
"Resuming" countdown, except for a pause made during the warmup, which ends at once.
==================
*/
void OSP_Ref_Pause( gentity_t *ent ) {
	static int	pausedInWarmup = 0;
	static int	pauseTime = 0;			// level.time of a warmup pause
	char	*name;
	int		pauser;

	if ( ent ) {
		int		pid;

		pid = osp_matchPaused;
		if ( pid ) {
			if ( pid > 10000 && pid != 11001 ) {
				pid -= 10001;
			}
			if ( g_entities[pid].client->pers.cmdDelayTime - level.time > 0 ) {
				trap_SendServerCommand( ent - g_entities, va( "print \"Wait another %.1fs to unpause match\n\"", 1.0f * ( g_entities[pid].client->pers.cmdDelayTime - level.time ) / 1000.0f ) );
				return;
			}
		}
		ent->client->pers.cmdDelayTime = level.time + 5000;
		name = "Referee";
		pauser = ent - g_entities;
	} else {
		name = "Admin";
		pauser = 1000;
	}

	if ( !osp_matchPaused ) {
		if ( osp_unpausing ) {
			return;
		}
		if ( level.warmupTime ) {
			pauseTime = level.time;
			pausedInWarmup = 1;
		} else {
			pausedInWarmup = 0;
		}
		osp_matchPaused = pauser + 10001;
		osp_timeoutEndTime = -1;
		OSP_GlobalSound( "sound/world/klaxon1.wav" );
		trap_SendServerCommand( -1, va( "cp \"^2%s has ^7PAUSED^2 the match!^7\n\"", name ) );
		trap_SendServerCommand( -1, va( "print \"^2%s has ^7PAUSED^2 the match! (%s^7)\n\"", name, ent ? ent->client->pers.netname : "Admin" ) );
		G_LogPrintf( "Pause: %s\n", name );
		trap_SetConfigstring( CS_LEVEL_START_TIME, va( "%i", level.time ) );
		OSP_UpdateScoreCvars( "PAUSED" );
	} else {
		if ( osp_unpausing ) {
			return;
		}
		G_LogPrintf( "Unpause: %s\n", name );
		if ( osp_matchPaused >= 10000 && pausedInWarmup ) {
			osp_timeoutEndTime = 0;
			osp_matchPaused = 0;
			osp_unpausing = 0;
			trap_SetConfigstring( CS_LEVEL_START_TIME, va( "%i", level.startTime + osp_pausedTime ) );
			if ( level.warmupTime > 0 ) {
				level.warmupTime += level.time - pauseTime;
				trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupTime ) );
			}
			OSP_GlobalSound( "sound/feedback/fight.wav" );
			trap_SendServerCommand( -1, va( "cp \"^5%s has ^7UNPAUSED^5 the match!\n\"", name ) );
		} else {
			osp_unpausing = 1;
			level.warmupTime = level.time + 5000;
			osp_timeoutEndTime = 0;
			trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupTime ) );
			trap_SendServerCommand( -1, va( "cp \"^5%s has ^7UNPAUSED^5 the match!\n^3Resuming...\n\"", name ) );
		}
	}
}

/*
==================
OSP_Ref_Ban

"ban <player>" (ent NULL: server console): adds an address ban to the filters and kicks
the player; without a free filter slot or a known address the player is only kicked
==================
*/
void OSP_Ref_Ban( gentity_t *ent, int argOffset ) {
	int		pid;
	char	arg[MAX_TOKEN_CHARS];

	trap_Argv( argOffset + 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		if ( ent ) {
			OSP_PrintInvalidPlayerID( ent );
		} else {
			G_Printf( "Use the \"status\" command to find a valid player ID.\n" );
		}
		return;
	}
	if ( ent && level.clients + pid == ent->client ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't ban yourself!\n\"" );
		return;
	}

	if ( level.numFilters < MAX_OSP_FILTERS && level.clients[pid].sess.ip[0] && !( g_entities[pid].r.svFlags & SVF_BOT ) ) {
		char	name[MAX_NETNAME];
		char	userinfo[MAX_INFO_STRING];
		char	cleanName[MAX_NETNAME];

		if ( level.numFilters < MAX_OSP_FILTERS ) {
			trap_GetUserinfo( pid, userinfo, sizeof( userinfo ) );
			Q_strncpyz( name, Info_ValueForKey( userinfo, "name" ), sizeof( name ) );
			OSP_NormalizeName( name, sizeof( name ), cleanName, qtrue );
			osp_filters[level.numFilters].type = 4;
			Q_strncpyz( osp_filters[level.numFilters].name, cleanName, sizeof( osp_filters[0].name ) );
			Q_strncpyz( osp_filters[level.numFilters].ip, level.clients[pid].sess.ip, sizeof( osp_filters[0].ip ) );
			Q_strncpyz( osp_filters[level.numFilters].password, "none", sizeof( osp_filters[0].password ) );
			level.numFilters++;
			trap_SendServerCommand( -1, va( "print \"%s has been banned by referee!\n\"", level.clients[pid].pers.netname ) );
			if ( ent ) {
				OSP_AdminLogPrintf( va( "Referee_Ban\t%s -> %s", ent->client->pers.netname, level.clients[pid].pers.netname ) );
			} else {
				OSP_AdminLogPrintf( va( "Referee_Ban\tconsole -> %s", level.clients[pid].pers.netname ) );
			}
		} else {
			// OSP bug kept: no "print" command
			trap_SendServerCommand( ent - g_entities, va( "\nSorry, no more slots left for banning %s\n\n", level.clients[pid].pers.netname ) );
		}
	} else {
		trap_SendServerCommand( -1, va( "print \"%s has been kicked by referee!\n\"", level.clients[pid].pers.netname ) );
		if ( ent ) {
			if ( level.numFilters >= MAX_OSP_FILTERS ) {
				trap_SendServerCommand( ent - g_entities, "print \"\n^3Sorry, server has no more available slots for banning.\n\n" );
			} else {
				trap_SendServerCommand( ent - g_entities, "print \"\n^3Sorry, client has no known IP address (?)\n\n" );
			}
			OSP_AdminLogPrintf( va( "Referee_Kick\t%s -> %s", ent->client->pers.netname, level.clients[pid].pers.netname ) );
		} else {
			OSP_AdminLogPrintf( va( "Referee_Kick\tconsole -> %s", level.clients[pid].pers.netname ) );
		}
	}
	trap_SendConsoleCommand( EXEC_APPEND, va( "clientkick %d\n", pid ) );
}

/*
==================
OSP_Ref_Kick
==================
*/
void OSP_Ref_Kick( gentity_t *ent, int argOffset ) {
	int		pid;
	char	arg[MAX_TOKEN_CHARS];

	trap_Argv( argOffset + 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}
	if ( level.clients + pid == ent->client ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't kick yourself!\n\"" );
		return;
	}

	trap_SendServerCommand( -1, va( "print \"%s has been kicked by referee!\n\"", level.clients[pid].pers.netname ) );
	OSP_AdminLogPrintf( va( "Referee_Kick\t%s -> %s", ent->client->pers.netname, level.clients[pid].pers.netname ) );
	trap_SendConsoleCommand( EXEC_APPEND, va( "clientkick %d\n", pid ) );
}

/*
==================
OSP_Ref_Map

"r_map <map>": with map_queue the map must be in the map list (name or number) and the
level ends into it; else a plain "map" command (keeping nextmap)
==================
*/
void OSP_Ref_Map( gentity_t *ent, int argOffset ) {
	char	map[MAX_TOKEN_CHARS];
	char	nextmap[MAX_STRING_CHARS];

	trap_Argv( argOffset + 1, map, sizeof( map ) );
	if ( map_queue.integer ) {
		if ( trap_Argc() == argOffset + 1 ) {
			OSP_Cmd_MapList_f( ent );
			if ( !argOffset ) {
				trap_SendServerCommand( ent - g_entities, "print \"Usage: ^3\\r_map <mapname>^7\n\"" );
			} else {
				trap_SendServerCommand( ent - g_entities, "print \"Usage: ^3\\ref map <mapname>^7\n\"" );
			}
			return;
		}
		if ( trap_Argc() > argOffset + 2 ) {
			if ( !argOffset ) {
				trap_SendServerCommand( ent - g_entities, "print \"Too many r_map parameters!\n\"" );
				trap_SendServerCommand( ent - g_entities, "print \"Usage: ^3\\r_map <mapname>^7\n\"" );
			} else {
				trap_SendServerCommand( ent - g_entities, "print \"Too many map parameters!\n\"" );
				trap_SendServerCommand( ent - g_entities, "print \"Usage: ^3\\ref map <mapname>^7\n\"" );
			}
			return;
		}
		if ( !OSP_ValidateMap( ent, map, qtrue ) ) {
			return;
		}
		LogExit( va( "Ref map: %s", map ) );
		OSP_NextMap( 2 );
		return;
	}

	// the name goes to the server console: accept only a map that exists on this server
	// (as the vote path does), which also keeps ';', quotes and control characters out
	if ( !map[0] || !OSP_FindMapFile( map ) ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"Map ^3%s^7 not found.\n\"", map ) );
		if ( !argOffset ) {
			trap_SendServerCommand( ent - g_entities, "print \"Usage: ^3\\r_map <mapname>^7\n\"" );
		} else {
			trap_SendServerCommand( ent - g_entities, "print \"Usage: ^3\\ref map <mapname>^7\n\"" );
		}
		return;
	}

	trap_SendServerCommand( -1, va( "print \"Referee changes map to: %s\n", map ) );
	trap_Cvar_VariableStringBuffer( "nextmap", nextmap, sizeof( nextmap ) );
	if ( *nextmap ) {
		trap_SendConsoleCommand( EXEC_APPEND, va( "map %s; set nextmap \"%s\"\n", map, nextmap ) );
	} else {
		trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", map ) );
	}
}

/*
==================
OSP_Ref_Lock

Toggles the lock of team (OSP_Cmd_Lock_f, OSP_RefCommand)
==================
*/
void OSP_Ref_Lock( gentity_t *ent, int team ) {
	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3r_[un]lock[red|blue]^7 only for team-based games!\n\"" );
		return;
	}
	if ( !TeamCount( -1, team ) ) {
		trap_SendServerCommand( ent - g_entities, "print \"Sorry, cannot lock empty teams!\n\"" );
		return;
	}

	osp_teamInfo[team].locked = 1 - osp_teamInfo[team].locked;
	if ( osp_teamInfo[team].locked ) {
		trap_SendServerCommand( -1, va( "cp \"Referee has LOCKED the %s team\n\"", osp_teamNames[team] ) );
		trap_SendServerCommand( ent - g_entities, va( "print \"You have ^3LOCKED^7 the %s team\n\"", osp_teamNames[team] ) );
	} else {
		trap_SendServerCommand( -1, va( "cp \"Referee has UNLOCKED the %s team\n\"", osp_teamNames[team] ) );
		trap_SendServerCommand( ent - g_entities, va( "print \"You have ^3UNLOCKED^7 the %s team\n\"", osp_teamNames[team] ) );
	}
}

/*
==================
OSP_Ref_SpecLock

Toggles the spectator lock of team (OSP_Cmd_SpecLock_f, OSP_RefCommand)
==================
*/
void OSP_Ref_SpecLock( gentity_t *ent, int team ) {
	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3r_[un]speclock[red|blue]^7 only for team-based games!\n\"" );
		return;
	}
	if ( !TeamCount( -1, team ) ) {
		trap_SendServerCommand( ent - g_entities, "print \"Sorry, cannot lock empty teams!\n\"" );
		return;
	}

	osp_teamInfo[team].specLocked = 1 - osp_teamInfo[team].specLocked;
	if ( osp_teamInfo[team].specLocked ) {
		trap_SendServerCommand( -1, va( "cp \"Referee has LOCKED the %s team\nfrom Spectators\n\"", osp_teamNames[team] ) );
		trap_SendServerCommand( ent - g_entities, va( "print \"You have ^3SPECTATOR LOCKED^7 the %s team\n\"", osp_teamNames[team] ) );
	} else {
		trap_SendServerCommand( -1, va( "cp \"Referee has UNLOCKED the %s team\nfrom Spectators\n\"", osp_teamNames[team] ) );
		trap_SendServerCommand( ent - g_entities, va( "print \"You have ^3SPECTATOR UNLOCKED^7 the %s team\n\"", osp_teamNames[team] ) );
	}
	OSP_MV_Validate();
}

/*
==================
OSP_Ref_Restart
==================
*/
void OSP_Ref_Restart( gentity_t *ent ) {
	trap_SendServerCommand( -1, "cp \"^1Referee restarting map!^7\n\"" );
	OSP_MapRestart();
}

/*
==================
OSP_Ref_Put

"r_putred / r_putblue <player>"
==================
*/
void OSP_Ref_Put( gentity_t *ent, int team, int argOffset ) {
	gentity_t	*targ;
	char		arg[MAX_TOKEN_CHARS];
	int			pid;

	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3r_put[red|blue]^7 only for team-based games!\n\"" );
		return;
	}

	trap_Argv( argOffset + 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}

	targ = g_entities + pid;
	if ( targ->client->sess.sessionTeam == team ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"%s is already on team %s!\n\"", targ->client->pers.netname, osp_teamNames[team] ) );
		return;
	}
	if ( team_maxplayers.integer && TeamCount( -1, team ) >= team_maxplayers.integer ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"Sorry, the %s team is already full!\n\"", osp_teamNames[team] ) );
		return;
	}

	targ->client->pers.ready = 0;
	targ->client->readyToExit = qfalse;
	targ->client->pers.invitedTeam = 1;
	if ( team == TEAM_RED ) {
		SetTeam( targ, "red" );
	} else {
		SetTeam( targ, "blue" );
	}
	OSP_ResetPlayerReady();
}

/*
==================
OSP_Ref_Remove

"r_remove <player>": moves a player to the spectators
==================
*/
void OSP_Ref_Remove( gentity_t *ent, int argOffset ) {
	gentity_t	*targ;
	int			pid;
	char		arg[MAX_TOKEN_CHARS];

	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3r_remove^7 only for team-based games!\n\"" );
		return;
	}

	trap_Argv( argOffset + 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}
	if ( level.clients + pid == ent->client ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't remove yourself!\n\"" );
		return;
	}

	targ = g_entities + pid;
	if ( targ->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can only remove people in the game!\n\"" );
		return;
	}

	if ( targ->client->sess.captain ) {
		OSP_SetCaptain( targ );
	}
	if ( targ->client->sess.sessionTeam == TEAM_RED ) {
		trap_SendServerCommand( -1, va( "cp \"%s\n^7removed from team ^1RED\n\"", targ->client->pers.netname ) );
		trap_SendServerCommand( targ - g_entities, "print \"^5You've been removed from the ^1RED^7 team\n\"" );
	} else {
		trap_SendServerCommand( -1, va( "cp \"%s\n^7removed from team ^4BLUE\n\"", targ->client->pers.netname ) );
		trap_SendServerCommand( targ - g_entities, "print \"^5You've been removed from the ^1BLUE^7 team\n\"" );
	}

	targ->client->pers.ready = 0;
	targ->client->readyToExit = qfalse;
	targ->client->pers.invitedTeam = 0;
	targ->client->pers.teamState.state = TEAM_BEGIN;
	targ->flags &= ~FL_GODMODE;
	targ->health = 0;
	targ->client->ps.stats[STAT_HEALTH] = 0;
	player_die( targ, targ, targ, 100000, MOD_SWITCHTEAM );
	targ->client->sess.sessionTeam = TEAM_SPECTATOR;
	targ->client->sess.spectatorTime = level.time;
	targ->client->pers.osp_unk_2bc = level.time;
	targ->client->sess.spectatorState = SPECTATOR_FREE;
	targ->client->sess.spectatorClient = 0;
	ClientUserinfoChanged( targ - g_entities );
	ClientBegin( targ - g_entities );
	OSP_ResetPlayerReady();
	OSP_StartCountdown();
}

/*
==================
OSP_Ref_Abort

"r_abort": stops the countdown / match and moves every player back to the spectators
==================
*/
void OSP_Ref_Abort( gentity_t *ent ) {
	gentity_t	*targ;
	int			i;

	if ( !g_doWarmup.integer ) {
		if ( ent ) {
			trap_SendServerCommand( ent - g_entities, "print \"^3r_abort^7 only for match-based games!\n\"" );
		}
		return;
	}

	for ( i = 0; i < level.maxclients; i++ ) {
		targ = g_entities + i;
		if ( targ->client->pers.connected != CON_CONNECTED || targ->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		targ->client->sess.captain = 0;
		targ->client->pers.ready = 0;
		targ->client->readyToExit = qfalse;
		targ->client->pers.invitedTeam = 0;
		targ->client->pers.teamState.state = TEAM_BEGIN;
		targ->flags &= ~FL_GODMODE;
		targ->health = 0;
		targ->client->ps.stats[STAT_HEALTH] = 0;
		player_die( targ, targ, targ, 100000, MOD_SWITCHTEAM );
		targ->client->sess.sessionTeam = TEAM_SPECTATOR;
		targ->client->sess.spectatorTime = level.time;
		targ->client->pers.osp_unk_2bc = level.time;
		targ->client->sess.spectatorState = SPECTATOR_FREE;
		targ->client->sess.spectatorClient = 0;
		ClientUserinfoChanged( i );
		ClientBegin( i );
	}
	OSP_ResetPlayerReady();
	OSP_ResetAllClientStats();
	OSP_AdminLogPrintf( va( "Referee_Abort\t%s", ent->client->pers.netname ) );
}

/*
==================
OSP_Ref_Promote

"r_promote <player>": makes a player the captain of his team
==================
*/
void OSP_Ref_Promote( gentity_t *ent, int argOffset ) {
	int			i;
	gentity_t	*targ;
	int			pid;
	char		arg[MAX_TOKEN_CHARS];

	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3r_promote^7 only for team-based games!\n\"" );
		return;
	}
	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Captains not enabled on this server.\n\"" );
		return;
	}

	trap_Argv( argOffset + 1, arg, sizeof( arg ) );
	if ( ( pid = ClientNumberFromString( ent, arg ) ) == -1 ) {
		OSP_PrintInvalidPlayerID( ent );
		return;
	}

	targ = g_entities + pid;
	if ( targ->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"You can't promote spectators!\n\"" );
		return;
	}

	// OSP bug kept: <= maxclients
	for ( i = 0; i <= level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == targ->client->sess.sessionTeam ) {
			level.clients[i].sess.captain = 0;
		}
	}
	targ->client->sess.captain = 1;
	trap_SendServerCommand( pid, "print \"** ^5You are now team captain!!^7\n\"" );
	OSP_TeamPrint( targ->client->sess.sessionTeam, va( "cp \"%s\n^7is now team captain\n\"", targ->client->pers.netname ) );
}

/*
==================
OSP_Ref_AllReady

"r_allready" (ent NULL: server console "allready"): readies every player and starts the
countdown
==================
*/
void OSP_Ref_AllReady( gentity_t *ent ) {
	int		i;

	if ( !level.warmupTime || ( OSP_IsRoundBased( g_gametype.integer ) && osp_roundsPlayed ) || osp_matchPaused ) {
		if ( ent ) {
			trap_SendServerCommand( ent - g_entities, "print \"Match already in progress!\n\"" );
		} else {
			G_Printf( "Match already in progress!\n" );
		}
		return;
	}
	if ( level.numPlayingClients < 2 ) {
		if ( ent ) {
			trap_SendServerCommand( ent - g_entities, "print \"Not enough players to start match!\n\"" );
		} else {
			G_Printf( "Not enough players to start match!\n" );
		}
		return;
	}

	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ) {
			level.clients[i].pers.ready = 1;
			level.clients[i].readyToExit = qtrue;
		}
	}
	if ( g_gametype.integer >= GT_TEAM ) {
		if ( TeamCount( -1, TEAM_RED ) ) {
			osp_teamInfo[TEAM_RED].locked = 1;
		}
		if ( TeamCount( -1, TEAM_BLUE ) ) {
			osp_teamInfo[TEAM_BLUE].locked = 1;
		}
	}
	OSP_ResetPlayerReady();
	level.forceReady = 1;
	OSP_StartCountdown();
}

/*
==================
OSP_Ref_MuteSpecs
==================
*/
void OSP_Ref_MuteSpecs( gentity_t *ent ) {
	if ( match_mutespecs.integer ) {
		trap_SendConsoleCommand( EXEC_APPEND, "set match_mutespecs 0\n" );
		trap_SendServerCommand( ent - g_entities, "print \"\nSpectator chat ^3DISABLED^7.\n\n\"" );
	} else {
		trap_SendConsoleCommand( EXEC_APPEND, "set match_mutespecs 1\n" );
		trap_SendServerCommand( ent - g_entities, "print \"\nSpectator chat ^3ENABLED^7.\n\n\"" );
	}
	trap_Cvar_Set( "default_change", "1" );
}

/*
==================
OSP_Ref_AllCaptain
==================
*/
void OSP_Ref_AllCaptain( gentity_t *ent ) {
	if ( g_gametype.integer < GT_TEAM ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3r_allcaptain^7 only for team-based games!\n\"" );
		return;
	}
	if ( team_nocontrols.integer ) {
		trap_SendServerCommand( ent - g_entities, "print \"Captains not enabled on this server.\n\"" );
		return;
	}

	if ( team_allcaptain.integer ) {
		trap_SendConsoleCommand( EXEC_APPEND, "set team_allcaptain 0\n" );
		trap_SendServerCommand( ent - g_entities, "print \"\nOnly one captain per team now enabled.\n\n\"" );
	} else {
		trap_SendConsoleCommand( EXEC_APPEND, "set team_allcaptain 1\n" );
		trap_SendServerCommand( ent - g_entities, "print \"\nAll players are now a team captain (invite and pause only).\n\n\"" );
	}
	trap_Cvar_Set( "default_change", "1" );
}
