/*
 * g_osp_multiview.c: OSP's multi-view: a spectator with an OSP client watches up
 * to 12 players at once. Each window is a "misc_portal_surface" entity (pers.mvPortals) at the
 * spectator's origin whose origin2 follows the viewed player (OSP_MV_UpdatePortal); the client
 * gets the window list with "viewlist" / "viewactive".
 */

#include "g_local.h"

#define OSP_MV_MAXVIEWS     12
#define OSP_MV_MAXPORTALS   64  // server-wide cap on multiview portal entities (one per view)

// "viewadd <player>"
void OSP_Cmd_ViewAdd_f(gentity_t* ent) {
	int     pid;
	char*    name;
	int     invite;
	int     team;
	char    arg[MAX_STRING_CHARS];
	int     coachTeam;

	invite = ent->client->sess.specInvite;
	name = ent->client->pers.netname;

	if (!ent->client->sess.ospClient && strstr(name, "QTV-") != name && strstr(name, "GTV-") != name) {
		trap_SendServerCommand(ent - g_entities, "print \"Multi-view for OSP clients only!\n\"");
		return;
	}

	if (ent->client->sess.nameTeam) {
		trap_SendServerCommand(ent - g_entities, "print \"Not allowed to switch from current team.\n\"");
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	if ((pid = ClientNumberFromString(ent, arg)) == -1) {
		OSP_PrintInvalidPlayerID(ent);
		return;
	}

	if (&level.clients[pid] == ent->client) {
		trap_SendServerCommand(ent - g_entities, "print \"You can't view yourself!\n\"");
		return;
	}

	team = level.clients[pid].sess.sessionTeam;
	if (level.clients[pid].sess.sessionTeam == TEAM_SPECTATOR) {
		trap_SendServerCommand(ent - g_entities, "print \"You can't view spectators!\n\"");
		return;
	}

	if (ent->client->sess.coachTeam && team != ent->client->sess.coachTeam) {
		return;
	}

	if (g_gametype.integer >= GT_TEAM) {
		if (!ent->client->sess.referee && osp_teamInfo[team].specLocked && (!invite || (invite != team && invite != 3))) {
			trap_SendServerCommand(ent - g_entities, va("print \"Sorry, the %s team is locked from spectators.\n\"", osp_teamNames[team]));
			return;
		}
	}

	if (!ent->client->pers.mvCount && (ent->client->sess.sessionTeam != TEAM_SPECTATOR
	                                   || (ent->client->sess.spectatorState != SPECTATOR_FREE && ent->client->sess.spectatorState != SPECTATOR_LOCKED))) {
		coachTeam = ent->client->sess.coachTeam;
		invite = ent->client->pers.coachInvite;
		ent->client->sess.coachTeam = 0;
		SetTeam(ent, "s");
		ent->client->sess.coachTeam = coachTeam;
		ent->client->pers.coachInvite = invite;
		ClientUserinfoChanged(ent - g_entities);

		if (ent->client->sess.sessionTeam != TEAM_SPECTATOR
		        || (ent->client->sess.spectatorState != SPECTATOR_FREE && ent->client->sess.spectatorState != SPECTATOR_LOCKED)) {
			trap_SendServerCommand(ent - g_entities, "print \"Can only multi-view as a spectator!\n\"");
			return;
		}
	}

	OSP_MV_AddView(ent, pid, qtrue);
}

// "viewremove [player]": without an argument the active window is removed
void OSP_Cmd_ViewRemove_f(gentity_t* ent) {
	int     pid;
	char    arg[MAX_STRING_CHARS];

	if (ent->client->pers.mvCount < 1) {
		return;
	}

	if (ent->client->pers.mvCount == 1) {
		OSP_MV_RemoveAll(ent);
		return;
	}

	if (trap_Argc() == 1) {
		pid = ent->client->pers.mvClients[ent->client->pers.mvActive];
	} else {
		trap_Argv(1, arg, sizeof(arg));
		if ((pid = ClientNumberFromString(ent, arg)) == -1) {
			OSP_PrintInvalidPlayerID(ent);
			return;
		}
	}

	OSP_MV_RemoveView(ent, pid, 2);
}

// "viewfollow": switches between the multiview windows and following the player of the
// active window (the windows are saved in mvSavedClients and restored the next time)
void OSP_Cmd_ViewFollow_f(gentity_t* ent) {
	int     i;
	int     count = 0;

	if (ent->client->pers.mvCount < 1) {
		if (ent->client->pers.mvSavedCount < 1) {
			return;
		}

		StopFollowing(ent);
		for (i = 0; i < ent->client->pers.mvSavedCount; i++) {
			OSP_MV_AddView(ent, ent->client->pers.mvSavedClients[i], qfalse);
		}
		OSP_MV_Validate();
		return;
	}

	count = ent->client->pers.mvCount;
	for (i = 0; i < ent->client->pers.mvCount; i++) {
		ent->client->pers.mvSavedClients[i] = ent->client->pers.mvClients[i];
	}
	i = ent->client->pers.mvClients[ent->client->pers.mvActive];

	OSP_MV_RemoveAll(ent);
	ent->client->pers.mvSavedCount = count;
	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

// "viewnext" / "viewprev" (and FOLLOW cycling in multiview, Cmd_FollowCycle_f)
void OSP_Cmd_ViewNext_f(gentity_t* ent, int dir) {
	int     active;

	if (!ent->client->pers.mvCount) {
		return;
	}

	active = ent->client->pers.mvActive + dir;
	if (active < 0) {
		active = ent->client->pers.mvCount - 1;
	} else if (active >= ent->client->pers.mvCount) {
		active = 0;
	}
	ent->client->pers.mvActive = active;

	trap_SendServerCommand(ent - g_entities, va("viewactive %d", ent->client->pers.mvActive));
}

// "viewframe <window>"
void OSP_Cmd_ViewFrame_f(gentity_t* ent) {
	int     frame;
	char    arg[MAX_STRING_CHARS];

	if (!ent->client->pers.mvCount) {
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	frame = atoi(arg);
	if (frame < 0 || frame >= ent->client->pers.mvCount) {
		return;
	}

	ent->client->pers.mvActive = frame;
	trap_SendServerCommand(ent - g_entities, va("viewactive %d", ent->client->pers.mvActive));
}

// "viewcyclenext" / "viewcycleprev": the active window shows the next player not shown yet
void OSP_Cmd_ViewCycle_f(gentity_t* ent, int dir) {
	int                 i;
	clientPersistant_t*  pers;
	int                 next;
	int                 pid;
	int                 start;

	pers = &ent->client->pers;

	if (dir != 1 && dir != -1) {
		return;
	}

	if (!pers->mvCount || pers->mvCount >= level.numPlayingClients) {
		return;
	}

	if (ent->client->sess.coachTeam && pers->mvCount >= TeamCount(-1, ent->client->sess.coachTeam)) {
		return;
	}

	start = pers->mvClients[pers->mvActive];
	pid = start;
	do {
		i = ent->client->sess.spectatorClient;
		ent->client->sess.spectatorClient = pid;
		next = OSP_FollowNext(ent, dir, level.clients[pid].sess.sessionTeam);
		ent->client->sess.spectatorClient = i;
		if (next == -1) {
			return;
		}

		pid = next;
		for (i = 0; i < pers->mvCount; i++) {
			if (next == pers->mvClients[i]) {
				break;
			}
		}
	} while (i < pers->mvCount && pid != start);

	if (pid == start) {
		return;
	}

	pers->mvClients[pers->mvActive] = pid;
	OSP_MV_SendViewList(ent);
}

// "viewnone": removes every window
void OSP_MV_RemoveAll(gentity_t* ent) {
	if (!ent->client->pers.mvCount) {
		return;
	}

	do {
		OSP_MV_RemoveView(ent, ent->client->pers.mvClients[0], 0);
	} while (ent->client->pers.mvCount > 0);

	ent->client->pers.mvCount = 0;
	ent->client->pers.mvActive = 0;
	ent->client->pers.mvSavedCount = 0;
	OSP_MV_SendViewList(ent);
}

// "viewlist <count> <client>..." and "viewactive <window>" to the client
void OSP_MV_SendViewList(gentity_t* ent) {
	int                 i;
	clientPersistant_t*  pers;
	char                buf[512];

	pers = &ent->client->pers;

	strcpy(buf, va("viewlist %d", pers->mvCount));
	for (i = 0; i < pers->mvCount; i++) {
		strcat(buf, va(" %d", pers->mvClients[i]));
	}
	trap_SendServerCommand(ent - g_entities, va("%s\n", buf));

	if (pers->mvCount) {
		trap_SendServerCommand(ent - g_entities, va("viewactive %d\n", pers->mvActive));
	}
}

// "viewall": every player (team games: both teams)
void OSP_Cmd_ViewAll_f(gentity_t* ent) {
	int         j;
	int         i;
	int         found;
	gentity_t*   other;
	gclient_t*   cl;
	char*        name;

	name = ent->client->pers.netname;

	if (!ent->client->sess.ospClient && strstr(name, "QTV-") != name && strstr(name, "GTV-") != name) {
		trap_SendServerCommand(ent - g_entities, "print \"Multi-view for OSP clients only!\n\"");
		return;
	}

	if (g_gametype.integer >= GT_TEAM) {
		OSP_Cmd_ViewTeam_f(ent, TEAM_BLUE, qfalse);
		OSP_Cmd_ViewTeam_f(ent, TEAM_RED, qfalse);
		ent->client->pers.followTeam = 0;
		return;
	}

	if (!ent->client->pers.mvCount && (ent->client->sess.sessionTeam != TEAM_SPECTATOR
	                                   || (ent->client->sess.spectatorState != SPECTATOR_FREE && ent->client->sess.spectatorState != SPECTATOR_LOCKED))) {
		SetTeam(ent, "s");
		ClientUserinfoChanged(ent - g_entities);

		if (ent->client->sess.sessionTeam != TEAM_SPECTATOR
		        || (ent->client->sess.spectatorState != SPECTATOR_FREE && ent->client->sess.spectatorState != SPECTATOR_LOCKED)) {
			trap_SendServerCommand(ent - g_entities, "print \"Can only multi-view as a spectator!\n\"");
			return;
		}
	}

	for (i = 0; i < level.maxclients; i++) {
		other = &g_entities[i];
		cl = &level.clients[i];
		if (!other->inuse || cl->pers.connected != CON_CONNECTED) {
			continue;
		}

		found = 0;
		for (j = 0; j < ent->client->pers.mvCount; j++) {
			if (i == ent->client->pers.mvClients[j]) {
				found = 1;
				break;
			}
		}
		if (!found) {
			OSP_MV_AddView(ent, i, qfalse);
		}
	}

	OSP_MV_Validate();
	ClientUserinfoChanged(ent - g_entities);
}

// "viewred" / "viewblue": every player of team
void OSP_Cmd_ViewTeam_f(gentity_t* ent, int team, qboolean fValidate) {
	int         j;
	int         i;
	int         found;
	gclient_t*   cl;
	gentity_t*   other;
	char*        name;

	name = ent->client->pers.netname;

	if (!ent->client->sess.ospClient && strstr(name, "QTV-") != name && strstr(name, "GTV-") != name) {
		trap_SendServerCommand(ent - g_entities, "print \"Multi-view for OSP clients only!\n\"");
		return;
	}

	if (g_gametype.integer < GT_TEAM) {
		trap_SendServerCommand(ent - g_entities, "print \"Team multiview is only for team games!\n\"");
		return;
	}

	if (ent->client->sess.nameTeam) {
		trap_SendServerCommand(ent - g_entities, "print \"Not allowed to switch from current team.\n\"");
		return;
	}

	if (ent->client->sess.coachTeam && team != ent->client->sess.coachTeam) {
		return;
	}

	if (!ent->client->sess.referee && osp_teamInfo[team].specLocked
	        && (!ent->client->sess.specInvite || (ent->client->sess.specInvite != team && ent->client->sess.specInvite != 3))) {
		trap_SendServerCommand(ent - g_entities, va("print \"Sorry, the %s team is locked from spectators!\n\"", osp_teamNames[team]));
		return;
	}

	if (!ent->client->pers.mvCount && (ent->client->sess.sessionTeam != TEAM_SPECTATOR
	                                   || (ent->client->sess.spectatorState != SPECTATOR_FREE && ent->client->sess.spectatorState != SPECTATOR_LOCKED))) {
		i = ent->client->sess.coachTeam;
		j = ent->client->pers.coachInvite;
		ent->client->sess.coachTeam = 0;
		SetTeam(ent, "s");
		ent->client->sess.coachTeam = i;
		ent->client->pers.coachInvite = j;
		ClientUserinfoChanged(ent - g_entities);

		if (ent->client->sess.sessionTeam != TEAM_SPECTATOR
		        || (ent->client->sess.spectatorState != SPECTATOR_FREE && ent->client->sess.spectatorState != SPECTATOR_LOCKED)) {
			trap_SendServerCommand(ent - g_entities, "print \"Can only multi-view as a spectator!\n\"");
			return;
		}
	}

	ent->client->pers.followTeam = team;

	for (i = 0; i < level.maxclients; i++) {
		other = &g_entities[i];
		cl = &level.clients[i];
		if (!other->inuse || cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam != team) {
			continue;
		}

		found = 0;
		for (j = 0; j < ent->client->pers.mvCount; j++) {
			if (i == ent->client->pers.mvClients[j]) {
				found = 1;
				break;
			}
		}
		if (!found) {
			OSP_MV_AddView(ent, i, qfalse);
		}
	}

	if (fValidate) {
		OSP_MV_Validate();
	}
	ClientUserinfoChanged(ent - g_entities);
}

// live multiview views (= portal entities) of all connected clients
static int OSP_MV_TotalViews(void) {
	int     i;
	int     total = 0;

	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected != CON_DISCONNECTED) {
			total += level.clients[i].pers.mvCount;
		}
	}
	return total;
}

// new window showing pid; the first one starts the multiview spectator mode
void OSP_MV_AddView(gentity_t* ent, int pid, qboolean fVerbose) {
	int         i;
	gentity_t*   other = NULL;

	if (ent->client->pers.mvCount >= OSP_MV_MAXVIEWS) {
		if (fVerbose) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3Already have %d views defined!\n\"", OSP_MV_MAXVIEWS));
		}
		return;
	}

	// every view costs one entity; with many spectators that exhausted g_entities and
	// G_Spawn() ended the map, so cap the total server-wide
	if (OSP_MV_TotalViews() >= OSP_MV_MAXPORTALS) {
		if (fVerbose) {
			trap_SendServerCommand(ent - g_entities, "print \"^3The server has no room for more multiview windows.\n\"");
		}
		return;
	}

	if (pid >= level.maxclients || pid < 0) {
		if (fVerbose) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3Illegal player ID: %d\n\"", pid));
		}
		return;
	}

	other = &g_entities[pid];
	if (!other->inuse || !other->client || other->client->pers.connected == CON_DISCONNECTED) {
		if (fVerbose) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3Player %d is not connected!\n\"", pid));
		}
		return;
	}

	for (i = 0; i < ent->client->pers.mvCount; i++) {
		if (ent->client->pers.mvClients[i] == pid) {
			if (fVerbose) {
				trap_SendServerCommand(ent - g_entities, va("print \"^3You are already viewing player %d!\n\"", pid));
			}
			return;
		}
	}

	if (!ent->client->pers.mvCount) {
		OSP_MV_Start(ent);
	}

	i = ent->client->pers.mvCount++;
	ent->client->pers.mvClients[i] = pid;
	if (i == 1) {
		ent->client->pers.mvActive = 0;
	}

	ent->client->pers.mvPortals[i] = G_Spawn();
	ent->client->pers.mvPortals[i]->classname = "misc_portal_surface";
	ent->client->pers.mvPortals[i]->r.svFlags = SVF_PORTAL;
	ent->client->pers.mvPortals[i]->s.eType = ET_PORTAL;
	VectorClear(ent->client->pers.mvPortals[i]->r.mins);
	VectorClear(ent->client->pers.mvPortals[i]->r.maxs);
	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
		VectorCopy(ent->r.currentOrigin, ent->client->pers.mvPortals[i]->s.origin);
	} else {
		VectorCopy(ent->s.origin, ent->client->pers.mvPortals[i]->s.origin);
	}
	ent->client->pers.mvPortals[i]->s.generic1 = 0;

	OSP_MV_SendViewList(ent);

	if (fVerbose) {
		trap_SendServerCommand(ent - g_entities, va("print \"^1*** ^3Player %d view activated!\n\"", pid));
	}
}

// the multiview spectator looks upside down from viewcam 0 or the intermission point
void OSP_MV_Start(gentity_t* ent) {
	if (osp_numViewcams) {
		VectorCopy(osp_viewcams[0].origin, ent->s.origin);
		VectorCopy(osp_viewcams[0].origin, ent->client->ps.origin);
		VectorCopy(osp_viewcams[0].angles, ent->client->ps.viewangles);
	} else {
		VectorCopy(level.intermission_origin, ent->s.origin);
		VectorCopy(level.intermission_origin, ent->client->ps.origin);
		VectorCopy(level.intermission_angle, ent->client->ps.viewangles);
	}

	ent->client->ps.viewangles[PITCH] += 180;
	ent->client->ps.viewangles[ROLL] += 180;
	ent->client->sess.spectatorState = SPECTATOR_MULTIVIEW;
}

// removes the window of pid; the last one makes ent a plain spectator again
void OSP_MV_RemoveView(gentity_t* ent, int pid, int verbose) {
	int     j;
	int     i;

	if (!ent->client->pers.mvCount) {
		if (verbose > 1) {
			trap_SendServerCommand(ent - g_entities, "print \"^3You aren't viewing any players!\n\"");
		}
		return;
	}

	if (pid > level.maxclients || pid < 0) {
		if (verbose > 1) {
			trap_SendServerCommand(ent - g_entities, va("print \"^3Illegal player ID: %d\n\"", pid));
		}
		return;
	}

	for (i = 0; i < ent->client->pers.mvCount; i++) {
		if (ent->client->pers.mvClients[i] == pid) {
			ent->client->pers.mvClients[i] = 0;
			G_FreeEntity(ent->client->pers.mvPortals[i]);
			ent->client->pers.mvPortals[i] = NULL;

			for (j = i + 1; j < ent->client->pers.mvCount; j++) {
				ent->client->pers.mvClients[j - 1] = ent->client->pers.mvClients[j];
				ent->client->pers.mvPortals[j - 1] = ent->client->pers.mvPortals[j];
				if (j == ent->client->pers.mvActive) {
					ent->client->pers.mvActive--;
				}
			}

			ent->client->pers.mvCount--;
			if (ent->client->pers.mvCount <= 0) {
				ent->client->pers.mvCount = 0;
				SetTeam(ent, "s");
				OSP_MV_SendViewList(ent);
			}

			if (ent->client->pers.mvCount) {
				if (ent->client->pers.mvActive >= ent->client->pers.mvCount) {
					ent->client->pers.mvActive = ent->client->pers.mvCount - 1;
				}
				if (ent->client->pers.mvActive < 0) {
					ent->client->pers.mvActive = 0;
				}
				if (verbose) {
					if (verbose > 1) {
						trap_SendServerCommand(ent - g_entities, va("print \"^1*** ^3Player %d view removed!\n\"", pid));
					}
					OSP_MV_SendViewList(ent);
				}
			}
			return;
		}
	}

	if (verbose > 1) {
		trap_SendServerCommand(ent - g_entities, va("print \"^3Player %d view not active!\n\"", pid));
	}
}

// every frame of a multiview spectator (SpectatorThink): the portals sit at the spectator and
// look at the viewed players
void OSP_MV_UpdatePortal(gentity_t* ent) {
	int         i;
	gentity_t*   other = NULL;

	if (!ent->client->pers.mvCount || (ent->r.svFlags & SVF_BOT)) {
		return;
	}

	for (i = 0; i < ent->client->pers.mvCount; i++) {
		other = &g_entities[ent->client->pers.mvClients[i]];
		if (other) {
			VectorCopy(ent->s.origin, ent->client->pers.mvPortals[i]->s.origin);
			G_SetOrigin(ent->client->pers.mvPortals[i], ent->client->pers.mvPortals[i]->s.origin);
			VectorCopy(other->r.currentOrigin, ent->client->pers.mvPortals[i]->s.origin2);
			trap_LinkEntity(ent->client->pers.mvPortals[i]);
		}
	}
}

// drops the windows of players who left, became spectators or whose team got locked, and
// adds the new players of a viewed team (pers.followTeam)
void OSP_MV_Validate(void) {
	gclient_t*   cl;
	int         j;
	gclient_t*   viewed;
	int         pid;
	gentity_t*   other;
	gentity_t*   ent;
	int         i;

	for (i = 0; i < level.maxclients; i++) {
		ent = &g_entities[i];
		cl = &level.clients[i];
		if (!g_entities[i].inuse || cl->pers.connected != CON_CONNECTED || cl->pers.mvCount < 1) {
			continue;
		}

		j = cl->pers.mvCount - 1;
		while (j >= 0) {
			pid = cl->pers.mvClients[j];
			other = &g_entities[pid];
			viewed = &level.clients[pid];
			if (!other->inuse || viewed->pers.connected != CON_CONNECTED || viewed->sess.sessionTeam == TEAM_SPECTATOR) {
				OSP_MV_RemoveView(ent, pid, 1);
				j--;
				continue;
			}

			if (!cl->sess.referee) {
				if (osp_teamInfo[TEAM_RED].specLocked && viewed->sess.sessionTeam == TEAM_RED
				        && (!cl->sess.specInvite || cl->sess.specInvite == TEAM_BLUE)) {
					OSP_MV_RemoveView(ent, pid, 1);
					j--;
					continue;
				}
				if (osp_teamInfo[TEAM_BLUE].specLocked && viewed->sess.sessionTeam == TEAM_BLUE
				        && cl->sess.specInvite < TEAM_BLUE) {
					OSP_MV_RemoveView(ent, pid, 1);
					j--;
					continue;
				}
			}

			if (cl->pers.followTeam && viewed->sess.sessionTeam != cl->pers.followTeam) {
				OSP_MV_RemoveView(ent, pid, 1);
				j--;
				continue;
			}
			j--;
		}

		pid = cl->pers.followTeam;
		if (cl->pers.mvCount > 0 && pid && (!osp_teamInfo[pid].specLocked || cl->sess.referee
		                                    || (cl->sess.specInvite && (cl->sess.specInvite == pid || cl->sess.specInvite == 3)))) {
			int     k;
			int     found;

			for (j = 0; j < level.maxclients; j++) {
				other = &g_entities[j];
				viewed = &level.clients[j];
				if (!other->inuse || viewed->pers.connected != CON_CONNECTED || viewed->sess.sessionTeam != cl->pers.followTeam) {
					continue;
				}

				found = 0;
				for (k = 0; k < cl->pers.mvCount; k++) {
					if (j == cl->pers.mvClients[k]) {
						found = 1;
						break;
					}
				}
				if (!found) {
					OSP_MV_AddView(ent, j, qfalse);
				}
			}
		}
	}

	// OSP bug: runs once, for the last client slot
	if (cl->pers.mvCount < 1) {
		cl->pers.mvActive = 0;
		cl->pers.mvSavedCount = 0;
		OSP_MV_SendViewList(ent);
	}
}

// "sess_mv" (number of multiview spectators) and "sess_mv<client>" ("<count> <client>...")
// keep the windows over a map change (G_WriteSessionData)
void OSP_MV_WriteSession(void) {
	int         j;
	gentity_t*   ent = NULL;
	char        buf[256];
	int         i;
	int         count;

	if (level.intermissiontime && (map_rotate.integer || !osp_mapChangeReason)) {
		trap_Cvar_Set("sess_mv", "0");
		return;
	}

	count = 0;
	for (i = 0; i < level.numConnectedClients; i++) {
		ent = &g_entities[level.sortedClients[i]];
		if (ent->client->sess.sessionTeam != TEAM_SPECTATOR || !ent->client->pers.mvCount) {
			trap_Cvar_Set(va("sess_mv%d", level.sortedClients[i]), "0 0");
			continue;
		}

		count++;
		buf[0] = 0;
		for (j = 0; j < ent->client->pers.mvCount; j++) {
			strcat(buf, va(" %d", ent->client->pers.mvClients[j]));
		}
		trap_Cvar_Set(va("sess_mv%d", level.sortedClients[i]), va("%d%s", ent->client->pers.mvCount, buf));
	}

	trap_Cvar_Set("sess_mv", va("%d", count));
}

// restores the windows of clientNum from "sess_mv<client>" (ClientEndFrame)
void OSP_MV_ReadSession(int clientNum) {
	char*        p;
	int         i;
	int         pid;
	int         count;
	int         first;
	char        buf[MAX_STRING_CHARS];
	gclient_t*   cl;

	cl = &level.clients[clientNum];

	trap_Cvar_VariableStringBuffer(va("sess_mv%i", cl - level.clients), buf, sizeof(buf));
	if (!buf[0]) {
		trap_Cvar_Set(va("sess_mv%i", cl - level.clients), "0 0");
		cl->pers.mvCount = 0;
		return;
	}

	sscanf(buf, "%i %i", &count, &first);
	p = strchr(buf, ' ');
	if (p && count) {
		p++;
	} else {
		cl->pers.mvCount = 0;
		return;
	}

	for (i = 0; i < count; i++) {
		sscanf(p, "%i %i", &pid, &first);
		OSP_MV_AddView(&g_entities[clientNum], pid, qfalse);
		p = strchr(p, ' ');
		if (!p) {
			return;
		}
		p++;
	}
}
