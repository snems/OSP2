/*
 * g_osp_team.c: teams: joining, locks, captains, ready counts, per-mode team scores, round
 * start and the flag return.
 */

#include "g_local.h"

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t	team_noteamready;
vmCvar_t	team_nocontrols;
vmCvar_t	team_maxplayers;
vmCvar_t	team_armordamage;
vmCvar_t	team_allcaptain;
vmCvar_t	match_minplayers;
vmCvar_t	match_readypercent;
// OSP: the per-team table (see g_local.h)
ospTeamInfo_t	osp_teamInfo[3];

// OSP: initialised data
char	*osp_teamNames[] = { "NONE", "^1RED^7", "^4BLUE^7", "^3SPECTATORS^7" };	// indexed by team_t

extern teamgame_t	teamgame;	// g_team.c

/*
==================
OSP_ClientTeam

The team ent plays for: the coached team, the queued team of a waiting spectator in
round-based modes, else the session team.
==================
*/
int OSP_ClientTeam( gentity_t *ent ) {
	if ( ent->client->sess.coachTeam ) {
		return ent->client->sess.coachTeam;
	}

	if ( OSP_IsRoundBased( g_gametype.integer ) ) {
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && ent->client->sess.roundTeam != TEAM_SPECTATOR ) {
			return ent->client->sess.roundTeam;
		}
	}

	return ent->client->sess.sessionTeam;
}

/*
==================
OSP_CheckTeamJoin

0 when ent may not join team now (full, locked, round in progress); the reason goes to ent
when verbose.
==================
*/
int OSP_CheckTeamJoin( int team, gentity_t *ent, int verbose ) {
	int		count;

	count = OSP_IsRoundBased( g_gametype.integer ) ? OSP_TeamPlayerCount( team ) : TeamCount( -1, team );
	if ( count == 0 ) {
		osp_teamInfo[team].locked = 0;
		osp_teamInfo[team].specLocked = 0;
	}

	if ( team == TEAM_RED || team == TEAM_BLUE ) {
		if ( team_maxplayers.integer > 0 && team_maxplayers.integer <= count ) {
			if ( verbose && ent ) {
				trap_SendServerCommand( ent - g_entities, va( "print \"The %s team is full!\n\"", osp_teamNames[team] ) );
			}
			return 0;
		}

		if ( osp_teamInfo[team].locked != 0 && ( !ent || ( ent->client->pers.invitedTeam != team && level.intermissiontime == 0 ) ) ) {
			if ( verbose && ent ) {
				trap_SendServerCommand( ent - g_entities, va( "print \"The %s team is locked!\n\"", osp_teamNames[team] ) );
			}
			return 0;
		}

		if ( OSP_IsRoundBased( g_gametype.integer ) != 0 && osp_matchState == 2 && level.intermissiontime == 0 ) {
			if ( ent ) {
				if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
					trap_SendServerCommand( ent - g_entities, "print \"Sorry, can't switch teams until round is over!\n\"" );
				} else {
					trap_SendServerCommand( ent - g_entities, va( "print \"You will join the %s team next round.\n\"", osp_teamNames[team] ) );
					ent->client->sess.roundTeam = team;
				}
			}
			return 0;
		}

		return 1;
	}

	if ( team == TEAM_SPECTATOR || team == TEAM_FREE ) {
		return 0;
	}
	return 1;
}

/*
==================
OSP_ResetEmptyTeams

Unlocks the empty teams and resets their timeouts (no callers).
==================
*/
void OSP_ResetEmptyTeams( void ) {
	int		i;

	if ( g_gametype.integer < GT_TEAM ) {
		return;
	}

	for ( i = TEAM_RED; i <= TEAM_BLUE; i++ ) {
		if ( !TeamCount( -1, i ) ) {
			osp_teamInfo[i].locked = 0;
			osp_teamInfo[i].specLocked = 0;
			osp_teamInfo[i].osp_unk_28 = 0;
			osp_teamInfo[i].timeouts = match_timeoutcount.integer;
		}
	}
}

/*
==================
OSP_TeamPrint

Sends a server command to every connected player of team.
==================
*/
void OSP_TeamPrint( int team, char *cmd ) {
	int		i;

	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected != CON_CONNECTED || level.clients[i].sess.sessionTeam != team ) {
			continue;
		}
		trap_SendServerCommand( i, cmd );
	}
}

/*
==================
OSP_NumReady

Nonzero when enough players are ready to start the match (always without warmup).
==================
*/
int OSP_NumReady( void ) {
	gentity_t	*ent;
	int			i;
	int			ready, notReady, bots;

	ready = 0;
	notReady = 0;
	bots = 0;

	if ( !g_doWarmup.integer ) {
		return 1;
	}

	for ( i = 0; i < level.numConnectedClients; i++ ) {
		ent = g_entities + level.sortedClients[i];
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if ( ent->r.svFlags & SVF_BOT ) {
			bots++;
			continue;
		}
		if ( ent->client->pers.ready ) {
			ready++;
		} else {
			notReady++;
		}
	}

	if ( ready + notReady == 0 && level.forceReady == 0 ) {
		return 0;
	}

	if ( !level.forceReady && ready + notReady + bots < ( ( match_minplayers.integer < 2 ) ? 2 : match_minplayers.integer ) ) {
		return 0;
	}

	i = match_readypercent.integer;
	if ( i < 1 ) {
		i = 1;
	}

	return ( level.forceReady || ( 100 * ready ) / ( ready + notReady ) >= i );
}

/*
==================
OSP_SetCaptain

Passes ent's captaincy to the first other connected player of the team.
==================
*/
void OSP_SetCaptain( gentity_t *ent ) {
	int		i;

	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected != CON_CONNECTED || i == ent - g_entities
			|| level.clients[i].sess.sessionTeam != ent->client->sess.sessionTeam ) {
			continue;
		}

		level.clients[i].sess.captain = 1;
		ent->client->sess.captain = 0;
		trap_SendServerCommand( i, "print \"** ^5You are now team captain!!^7\n\"" );
		OSP_TeamPrint( ent->client->sess.sessionTeam, va( "cp \"%s\n^7is now team captain\n\"", level.clients[i].pers.netname ) );
		return;
	}
}

/*
==================
OSP_ClearCaptains
==================
*/
void OSP_ClearCaptains( void ) {
	int		i;

	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		level.clients[i].sess.captain = 0;
	}
}

/*
==================
OSP_PrintTimeRemaining
==================
*/
void OSP_PrintTimeRemaining( gentity_t *ent ) {
	if ( level.intermissionQueued == 0 && level.intermissiontime == 0 ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"\nTime Remaining: ^3%s\n\"", Score_Time.string ) );
	} else {
		char	serverinfo[MAX_INFO_STRING];
		char	mapname[MAX_INFO_STRING];

		trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );
		Q_strncpyz( mapname, Info_ValueForKey( serverinfo, "mapname" ), sizeof( mapname ) );
		trap_SendServerCommand( ent - g_entities, va( "print \"\nMap: %s\nTime Remaining: ^3MATCH COMPLETE\n\"", mapname ) );
	}
}

/*
==================
OSP_Scores_TDM

"scores" in team deathmatch: per-team tables (kills, deaths, suicides, team kills, net,
efficiency, damage given / received, team damage, score) and team totals.
==================
*/
void OSP_Scores_TDM( gentity_t *ent, qboolean fDump ) {
	gclient_t	*cl;
	int			i;
	int			eff;
	char		*color;
	char		name[MAX_STRING_CHARS];
	int			team;
	int			deaths, kills, suicides, teamKills;
	int			dg, dr, td;
	int			count;

	OSP_PrintTimeRemaining( ent );
	count = 0;

	for ( team = TEAM_RED; team <= TEAM_BLUE; team++ ) {
		if ( !TeamCount( -1, team ) ) {
			continue;
		}

		kills = 0;
		deaths = 0;
		suicides = 0;
		teamKills = 0;
		dg = 0;
		dr = 0;
		td = 0;

		trap_SendServerCommand( ent - g_entities, "print \"\n^7TEAM Player          Kll Dth Sui TK ^3NET^7 Eff    ^2DG    ^1DR   ^6TD  ^3Score\n^7-------------------------------------------------------------------\n\"" );

		for ( i = 0; i < level.numPlayingClients; i++ ) {
			cl = &level.clients[level.sortedClients[i]];
			if ( cl->sess.sessionTeam != team ) {
				continue;
			}

			SanitizeString( cl->pers.netname, name, qfalse );
			name[15] = 0;

			kills += cl->sess.kills;
			deaths += cl->sess.deaths;
			suicides += cl->sess.suicides;
			teamKills += cl->sess.teamKills;
			dg += cl->sess.damageGiven;
			dr += cl->sess.damageReceived;
			td += cl->sess.teamDamage;

			if ( cl->sess.deaths + cl->ps.persistant[PERS_SCORE] == 0 ) {
				eff = 0;
			} else {
				eff = 100 * cl->ps.persistant[PERS_SCORE] / ( cl->sess.deaths + cl->ps.persistant[PERS_SCORE] );
			}
			if ( eff < 0 ) {
				eff = 0;
			}

			if ( ent->client == cl || ( ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
				ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
				ent->client->sess.spectatorClient == level.sortedClients[i] ) ) {
				color = "^3";
			} else {
				color = "^7";
			}

			count++;
			trap_SendServerCommand( ent - g_entities, va( "print \"%-4s %s%-15s%4d%4d%4d%3d^3%4d%s%4d^2%6d^1%6d^6%5d^3%7d\n\"",
				osp_teamNames[team], color, name, cl->sess.kills, cl->sess.deaths, cl->sess.suicides, cl->sess.teamKills,
				cl->sess.kills - cl->sess.deaths - cl->sess.suicides - cl->sess.teamKills, color, eff,
				cl->sess.damageGiven, cl->sess.damageReceived, cl->sess.teamDamage, cl->ps.persistant[PERS_SCORE] ) );
		}

		if ( deaths + level.teamScores[team] == 0 ) {
			eff = 0;
		} else {
			eff = 100 * level.teamScores[team] / ( level.teamScores[team] + deaths );
		}
		if ( eff < 0 ) {
			eff = 0;
		}

		trap_SendServerCommand( ent - g_entities, va( "print \"^7-------------------------------------------------------------------\n%-4s %-15s%4d%4d%4d%3d^3%4d^5%4d^2%6d^1%6d^6%5d^3%7d\n\"",
			osp_teamNames[team], "^5Totals", kills, deaths, suicides, teamKills, kills - deaths - suicides - teamKills, eff,
			dg, dr, td, level.teamScores[team] ) );
	}

	if ( !count ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3No scores to report.\n\n\n\"" );
	} else {
		trap_SendServerCommand( ent - g_entities, "print \"\n\n\n\"" );
	}

	if ( fDump ) {
		OSP_StatsDump( ent );
	}
}

/*
==================
OSP_Scores_FT

"scores" in freeze tag: like OSP_Scores_TDM plus thaws (sess.losses) and rounds won (sess.wins).
==================
*/
void OSP_Scores_FT( gentity_t *ent, qboolean fDump ) {
	gclient_t	*cl;
	int			i;
	int			eff;
	char		*color;
	char		name[MAX_STRING_CHARS];
	int			team;
	int			deaths, kills, suicides, teamKills;
	int			dg, dr, td;
	int			thaws, wins;
	int			count;

	OSP_PrintTimeRemaining( ent );
	count = 0;

	for ( team = TEAM_RED; team <= TEAM_BLUE; team++ ) {
		if ( !OSP_TeamPlayerCount( team ) ) {
			continue;
		}

		kills = 0;
		deaths = 0;
		thaws = 0;
		wins = 0;
		suicides = 0;
		teamKills = 0;
		dg = 0;
		dr = 0;
		td = 0;

		trap_SendServerCommand( ent - g_entities, "print \"\n^7TEAM Player          Kll Thw Dth Sui TK ^3WIN^7 Eff    ^2DG    ^1DR   ^6TD  ^3Score\n^7-----------------------------------------------------------------------\n\"" );

		for ( i = 0; i < level.numConnectedClients; i++ ) {
			cl = &level.clients[level.sortedClients[i]];
			if ( cl->sess.sessionTeam != team ) {
				continue;
			}

			SanitizeString( cl->pers.netname, name, qfalse );
			name[15] = 0;

			kills += cl->sess.kills;
			deaths += cl->sess.deaths;
			thaws += cl->sess.losses & 0xffff;
			wins += cl->sess.wins & 0xffff;
			suicides += cl->sess.suicides;
			teamKills += cl->sess.teamKills;
			dg += cl->sess.damageGiven;
			dr += cl->sess.damageReceived;
			td += cl->sess.teamDamage;

			if ( cl->sess.deaths + cl->ps.persistant[PERS_SCORE] == 0 ) {
				eff = 0;
			} else {
				eff = 100 * cl->ps.persistant[PERS_SCORE] / ( cl->sess.deaths + cl->ps.persistant[PERS_SCORE] );
			}
			if ( eff < 0 ) {
				eff = 0;
			}

			if ( ent->client == cl || ( ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
				ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
				ent->client->sess.spectatorClient == level.sortedClients[i] ) ) {
				color = "^3";
			} else {
				color = "^7";
			}

			count++;
			trap_SendServerCommand( ent - g_entities, va( "print \"%-4s %s%-15s%4d%4d%4d%4d%3d^3%4d%s%4d^2%6d^1%6d^6%5d^3%7d\n\"",
				osp_teamNames[team], color, name, cl->sess.kills, cl->sess.losses & 0xffff, cl->sess.deaths, cl->sess.suicides,
				cl->sess.teamKills, cl->sess.wins & 0xffff, color, eff,
				cl->sess.damageGiven, cl->sess.damageReceived, cl->sess.teamDamage, cl->ps.persistant[PERS_SCORE] ) );
		}

		if ( deaths + level.teamScores[team] == 0 ) {
			eff = 0;
		} else {
			eff = 100 * level.teamScores[team] / ( level.teamScores[team] + deaths );
		}
		if ( eff < 0 ) {
			eff = 0;
		}

		trap_SendServerCommand( ent - g_entities, va( "print \"^7-----------------------------------------------------------------------\n%-4s %-15s%4d%4d%4d%4d%3d^3%4d^5%4d^2%6d^1%6d^6%5d^3%7d\n\"",
			osp_teamNames[team], "^5Totals", kills, thaws, deaths, suicides, teamKills, wins, eff,
			dg, dr, td, level.teamScores[team] ) );
	}

	if ( !count ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3No scores to report.\n\n\n\"" );
	} else {
		trap_SendServerCommand( ent - g_entities, "print \"\n\n\n\"" );
	}

	if ( fDump ) {
		OSP_StatsDump( ent );
	}
}

/*
==================
OSP_Scores_FFA

"scores" in free for all and 1v1 (kills, deaths, suicides, minutes played, frags per hour,
efficiency, damage given / received, score).
==================
*/
void OSP_Scores_FFA( gentity_t *ent, qboolean fDump ) {
	gclient_t	*cl;
	int			i;
	int			eff;
	int			fph;
	char		name[MAX_STRING_CHARS];
	char		*color;
	int			count;

	count = 0;
	OSP_PrintTimeRemaining( ent );
	trap_SendServerCommand( ent - g_entities, "print \"\nPlayer          Kll Dth Sui Time FPH Eff    ^2DG    ^1DR^3  Score\n^7-----------------------------------------------------------\n\"" );

	for ( i = 0; i < level.numPlayingClients; i++ ) {
		cl = &level.clients[level.sortedClients[i]];
		// OSP tests sess.spectatorTime (sess + 4), not sessionTeam
		if ( cl->sess.spectatorTime == TEAM_SPECTATOR ) {
			continue;
		}

		SanitizeString( cl->pers.netname, name, qfalse );
		name[15] = 0;

		if ( cl->sess.deaths + cl->ps.persistant[PERS_SCORE] == 0 ) {
			eff = 0;
		} else {
			eff = 100 * cl->ps.persistant[PERS_SCORE] / ( cl->sess.deaths + cl->ps.persistant[PERS_SCORE] );
		}
		if ( eff < 0 ) {
			eff = 0;
		}

		if ( cl->ps.persistant[PERS_SCORE] < 1 ) {
			fph = 0;
		} else if ( osp_matchTime - cl->pers.enterTime < 1 ) {
			fph = cl->ps.persistant[PERS_SCORE] * 1000 * 3600;
		} else {
			fph = cl->ps.persistant[PERS_SCORE] * 1000 * 3600 / ( osp_matchTime - cl->pers.enterTime );
		}
		if ( fph > 9999 ) {
			fph = 9999;
		}

		if ( ent->client == cl || ( ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
			ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
			ent->client->sess.spectatorClient == level.sortedClients[i] ) ) {
			color = "^3";
		} else {
			color = "^7";
		}

		count++;
		trap_SendServerCommand( ent - g_entities, va( "print \"%s%-15s^3%4d%4d%4d%5d%4d%4d^2%6d^1%6d^3%7d\n\"",
			color, name, cl->sess.kills, cl->sess.deaths, cl->sess.suicides,
			( osp_matchTime - cl->pers.enterTime ) / 60000, fph, eff,
			cl->sess.damageGiven, cl->sess.damageReceived, cl->ps.persistant[PERS_SCORE] ) );
	}

	if ( !count ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3No scores to report.\n\n\n\n\"" );
	} else {
		trap_SendServerCommand( ent - g_entities, "print \"\n\n\n\"" );
	}

	if ( fDump ) {
		OSP_StatsDump( ent );
	}
}

/*
==================
OSP_Scores_CTF

"scores" in CTF (kills, deaths, flag captures, flag carry time, assists, defense, recoveries).
==================
*/
void OSP_Scores_CTF( gentity_t *ent, qboolean fDump ) {
	gclient_t	*cl;
	int			i;
	int			min;
	char		*color;
	char		name[MAX_NETNAME];
	int			team;
	float		secs;
	char		*capColor;
	int			flagTime;
	int			kills, deaths, captures, assists, defends, recoveries;
	int			count;

	OSP_PrintTimeRemaining( ent );
	count = 0;

	for ( team = TEAM_RED; team <= TEAM_BLUE; team++ ) {
		if ( !TeamCount( -1, team ) ) {
			continue;
		}

		kills = 0;
		deaths = 0;
		captures = 0;
		assists = 0;
		defends = 0;
		recoveries = 0;
		flagTime = 0;

		trap_SendServerCommand( ent - g_entities, "print \"\n^7TEAM Player          Kll Dth ^2FC^7   FTime FA TD FR  Score\n-------------------------------------------------------\n\"" );

		for ( i = 0; i < level.numPlayingClients; i++ ) {
			cl = &level.clients[level.sortedClients[i]];
			if ( cl->sess.sessionTeam != team ) {
				continue;
			}

			SanitizeString( cl->pers.netname, name, qfalse );
			name[15] = 0;

			kills += cl->sess.kills;
			deaths += cl->sess.deaths;
			captures += cl->pers.teamState.captures;
			assists += cl->pers.teamState.assists;
			defends += cl->pers.teamState.basedefense + cl->pers.teamState.carrierdefense;
			recoveries += cl->pers.teamState.flagrecovery;

			if ( ent->client == cl || ( ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
				ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
				ent->client->sess.spectatorClient == level.sortedClients[i] ) ) {
				color = "^3";
				capColor = "^3";
			} else {
				color = "^7";
				capColor = "^3";
			}

			min = cl->pers.teamState.flagtime / 60000;
			secs = ( cl->pers.teamState.flagtime - 60000.0f * min ) / 1000.0f;
			flagTime += cl->pers.teamState.flagtime;

			count++;
			trap_SendServerCommand( ent - g_entities, va( "print \"%-4s %s%-15s^3%4d%4d%s%3d%s%3d:%04.1f%3d%3d%3d^3%7d\n\"",
				osp_teamNames[team], color, name, cl->sess.kills, cl->sess.deaths, capColor,
				cl->pers.teamState.captures, color, min, secs, cl->pers.teamState.assists,
				cl->pers.teamState.basedefense + cl->pers.teamState.carrierdefense,
				cl->pers.teamState.flagrecovery, cl->ps.persistant[PERS_SCORE] ) );
		}

		min = flagTime / 60000;
		secs = ( flagTime - 60000.0f * min ) / 1000.0f;

		trap_SendServerCommand( ent - g_entities, va( "print \"^7-------------------------------------------------------\n%-4s %-15s%4d%4d^2%3d^5%3d:%04.1f%3d%3d%3d^3%7d\n\"",
			osp_teamNames[team], "^5Totals", kills, deaths, captures, min, secs, assists, defends, recoveries,
			level.teamScores[team] ) );
	}

	if ( !count ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3No scores to report.\n\n\n\"" );
	} else {
		trap_SendServerCommand( ent - g_entities, "print \"\n^3FC:^7 Flag Captures  ^3TD:^7 Team Defense\n^3FA:^7 Flag Assists   ^3FR:^7 Flag Recovery\n\n\n\n\"" );
	}

	if ( fDump ) {
		OSP_StatsDump( ent );
	}
}

/*
==================
OSP_Scores_CA

"scores" in clan arena (kills, deaths, rounds won, health / armor left, damage score,
efficiency); the team with players comes first when blue is empty.
==================
*/
void OSP_Scores_CA( gentity_t *ent, qboolean fDump ) {
	gclient_t	*cl;
	int			i;
	char		*color;
	int			eff;
	int			team;
	char		*dsColor;
	char		name[MAX_STRING_CHARS];
	char		health[8];
	char		armor[8];
	int			t;
	int			count;

	trap_SendServerCommand( ent - g_entities, va( "print \"\nMatch Info: ^3%s\n\"", Score_Time.string ) );
	count = 0;

	for ( t = TEAM_RED; t <= TEAM_BLUE; t++ ) {
		team = t;
		if ( !TeamCount( -1, TEAM_BLUE ) ) {
			team = TEAM_BLUE - t + TEAM_RED;
		}
		if ( !OSP_TeamPlayerCount( team ) ) {
			continue;
		}

		count++;
		if ( count == 1 ) {
			trap_SendServerCommand( ent - g_entities, "print \"\n^3Hlt:^7 Health Remaining  ^3Arm:^7 Armor Remaining\n^3DS:^7  Damage Scoring    ^3Eff:^7 Efficiency\n\"" );
		}
		trap_SendServerCommand( ent - g_entities, "print \"\nTEAM Player          Kll Dth ^2WINS^7 Hlt Arm ^3DS^7 Eff  Score\n-------------------------------------------------------\n\"" );

		for ( i = 0; i < level.numConnectedClients; i++ ) {
			cl = &level.clients[level.sortedClients[i]];
			if ( cl->sess.sessionTeam != team && cl->sess.roundTeam != team ) {
				continue;
			}

			SanitizeString( cl->pers.netname, name, qfalse );
			name[15] = 0;

			if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) {
				strcpy( health, "   0" );
				strcpy( armor, "   0" );
			} else {
				if ( level.intermissiontime || level.intermissionQueued ) {
					strcpy( health, va( "%4d", cl->ps.stats[STAT_HEALTH] ) );
					strcpy( armor, va( "%4d", cl->ps.stats[STAT_ARMOR] ) );
				} else {
					strcpy( health, " XXX" );
					strcpy( armor, " XXX" );
				}
			}

			if ( cl->sess.deaths + cl->sess.kills == 0 ) {
				eff = 0;
			} else {
				eff = 100 * cl->sess.kills / ( cl->sess.deaths + cl->sess.kills );
			}
			if ( eff < 0 ) {
				eff = 0;
			}

			if ( ent->client == cl || ( ent->client->sess.sessionTeam == TEAM_SPECTATOR &&
				ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
				ent->client->sess.spectatorClient == level.sortedClients[i] ) ) {
				color = "^3";
				dsColor = "^3";
			} else {
				color = "^7";
				dsColor = "^3";
			}

			trap_SendServerCommand( ent - g_entities, va( "print \"%-4s %s%-15s^3%4d%4d^2%5d%s%s%s%s%3d%s%4d%s%7d\n\"",
				osp_teamNames[team], color, name, cl->sess.kills, cl->sess.deaths, cl->sess.wins & 0xffff,
				color, health, armor, dsColor, cl->sess.damageGiven / 100, color, eff, dsColor,
				cl->sess.kills + cl->sess.damageGiven / 100 ) );
		}

		trap_SendServerCommand( ent - g_entities, "print \"\n\"" );
	}

	if ( !count ) {
		trap_SendServerCommand( ent - g_entities, "print \"^3No scores to report.\n\n\n\"" );
	}

	if ( level.intermissionQueued == 0 && level.intermissiontime == 0 ) {
		OSP_PrintMatchResult( ent );
	}

	if ( fDump ) {
		OSP_StatsDump( ent );
	}
}

/*
==================
OSP_WinningTeam

Nonzero when the match is over: 1 outside round-based modes, else the team that reached
the roundlimit (TEAM_RED / TEAM_BLUE), 0 while rounds remain.
==================
*/
int OSP_WinningTeam( void ) {
	if ( !OSP_IsRoundBased( g_gametype.integer ) ) {
		return 1;
	}
	if ( level.teamScores[TEAM_RED] >= roundlimit.integer ) {
		return TEAM_RED;
	}
	if ( level.teamScores[TEAM_BLUE] >= roundlimit.integer ) {
		return TEAM_BLUE;
	}
	return 0;
}

/*
==================
OSP_RoundStart

Starts the next round of a round-based match (clan arena / freeze tag rounds).
==================
*/
void OSP_RoundStart( void ) {
	int		round;
	int		numRounds;

	round = osp_roundsPlayed + 1;
	numRounds = roundlimit.integer * 2 - 1;

	level.warmupTime = 0;
	trap_SetConfigstring( CS_WARMUP, "0" );
	level.osp_unk_1e00 = 1;
	OSP_UpdateScoreCvars( va( "Round %d/%d", round, numRounds ) );

	level.startTime = level.time;
	trap_SetConfigstring( CS_LEVEL_START_TIME, va( "%i", level.startTime ) );

	osp_teamInfo[TEAM_RED].locked = 1;
	osp_teamInfo[TEAM_BLUE].locked = 1;

	OSP_GlobalSound( "sound/feedback/fight.wav" );

	if ( !match_carespawn.integer && g_gametype.integer == 5 ) {
		trap_SendServerCommand( -1, va( "cp \"Round ^3%d^7 of ^3%d^7\n", round, numRounds ) );
	} else {
		strcpy( osp_matchResultPrint, va( "print \"Round ^3%d^7 of ^3%d^7\n", round, numRounds ) );
		strcpy( osp_matchResultCP, va( "cp \"Round ^3%d^7 of ^3%d^7\n", round, numRounds ) );
	}

	osp_matchState = 2;

	if ( !osp_roundsPlayed ) {
		gclient_t	*cl;
		int			i;

		level.osp_unk_3808 = 1;
		for ( i = 0; i < level.numConnectedClients; i++ ) {
			cl = &level.clients[level.sortedClients[i]];
			cl->readyToExit = 0;
			cl->ps.stats[STAT_CLIENTS_READY] = 0;
			cl->ps.weaponTime = 0;
			cl->ps.weaponstate = WEAPON_READY;
		}
		G_WriteSessionData();
	}

	OSP_RoundReset();
	level.osp_unk_3808 = 0;
}

/*
==================
OSP_LogTeamScore
==================
*/
void OSP_LogTeamScore( void ) {
	int		i;

	if ( !log_default.integer ) {
		for ( i = TEAM_RED; i <= TEAM_BLUE; i++ ) {
			G_LogPrintf( "TeamScore: %d %d %s\n", i, level.teamScores[i], osp_teamInfo[i].teamName );
		}
	}
}

/*
==================
OSP_FlagReturnThink

Think of a CTF flag at base: while the flag is away it rechecks every 5 seconds that a
player still carries it; a flag nobody carries is returned ("Lost ... flag").
==================
*/
void OSP_FlagReturnThink( gentity_t *ent ) {
	int		i;
	int		powerup;
	int		time;
	int		team;

	time = 0;
	if ( !ent ) {
		return;
	}

	if ( g_gametype.integer != GT_CTF || ( ent->flags & FL_DROPPED_ITEM ) || level.intermissiontime ) {
	} else if ( !Q_stricmp( ent->classname, "team_CTF_redflag" ) ) {
		team = TEAM_RED;
		powerup = PW_REDFLAG;
		if ( teamgame.redStatus == FLAG_ATBASE ) {
		} else if ( teamgame.redStatus != FLAG_TAKEN ) {
			time = level.time + 5000;
		} else {
			goto checkCarrier;
		}
	} else if ( !Q_stricmp( ent->classname, "team_CTF_blueflag" ) ) {
		team = TEAM_BLUE;
		powerup = PW_BLUEFLAG;
		if ( teamgame.blueStatus == FLAG_ATBASE ) {
		} else if ( teamgame.blueStatus != FLAG_TAKEN ) {
			time = level.time + 5000;
		} else {
			goto checkCarrier;
		}
	}
	goto setThink;

checkCarrier:
	// the flag is taken: keep checking while somebody carries it
	for ( i = 0; i < level.maxclients; i++ ) {
		if ( level.clients[i].pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}
		if ( level.clients[i].ps.powerups[powerup] ) {
			time = level.time + 5000;
			break;
		}
	}

	if ( !time ) {
		ent->nextthink = 0;
		ent->think = NULL;
		trap_SendServerCommand( -1, va( "print \"^3Lost %s ^3flag has been returned!\n\"", osp_teamNames[team] ) );
		Team_ResetFlag( team );
		return;
	}

setThink:
	ent->nextthink = time;
	if ( !time ) {
		ent->think = NULL;
	}
}
