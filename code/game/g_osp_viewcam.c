/*
 * g_osp_viewcam.c: the viewcam (osp-q3-Viewcam.txt): camera spots loaded from
 * <viewcam_directory>/viewcam-<mapname>.cfg ("id x y z pitch yaw [range]"), an action camera
 * chosen every frame for the viewcam spectators (OSP_ViewcamRunFrame), a camera that follows
 * one player (OSP_ViewcamThink), and the editing commands snapshot / snapshotdump /
 * vc_proximity / range.
 */

#include "g_local.h"

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t    viewcam_directory;
ospViewcam_t    osp_viewcams[MAX_OSP_VIEWCAMS];
// OSP: the rest of this file's bss; lcc emits it in reverse order of declaration.
int     osp_viewcamHoldTime;                            // the action camera is kept until then (OSP_ViewcamRunFrame)
vec3_t  osp_snapshotOrigins[MAX_OSP_SNAPSHOTS];         // positions recorded by "snapshot"
vec3_t  osp_snapshotAngles[MAX_OSP_SNAPSHOTS];          // view angles recorded by "snapshot"
int     osp_viewcamBestActivity;                        // activity of the camera chosen last (OSP_ViewcamRunFrame)

// OSP: initialised data
int     osp_numSnapshots = 0;   // snapshots taken on this map

/*
================
OSP_LoadViewcam

Loads the camera spots of the map (OSP_InitGame).
================
*/
void OSP_LoadViewcam(void) {
	char*            p;
	int             len;
	char            buf[MAX_OSP_FILE_CHARS];
	char            mapname[MAX_STRING_CHARS];
	char            filename[MAX_QPATH];
	fileHandle_t    f;
	char            serverinfo[MAX_INFO_STRING];

	osp_numViewcams = 0;
	filename[0] = 0;
	memset(osp_viewcams, 0, sizeof(osp_viewcams));

	if (g_gametype.integer == GT_SINGLE_PLAYER) {
		return;
	}

	trap_GetServerinfo(serverinfo, sizeof(serverinfo));
	Q_strncpyz(mapname, Info_ValueForKey(serverinfo, "mapname"), sizeof(mapname));

	if (viewcam_directory.string[0]) {
		strcpy(filename, va("%s/viewcam-%s.cfg", viewcam_directory.string, mapname));
	} else {
		strcpy(filename, va("viewcam-%s.cfg", mapname));
	}

	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if (len <= 0) {
		G_Printf("\n*** ERROR: Could not open viewcam file \"%s\"\n", filename);
		G_Printf("*** Viewcam spectator mode DISABLED for %s.\n\n", mapname);
		return;
	}
	if (len >= sizeof(buf)) {
		G_Printf("\n*** Viewcam file \"%s\" too big! (%d bytes, max = %d)\n", filename, len, sizeof(buf) - 1);
		G_Printf("*** Viewcam spectator mode DISABLED for %s.\n\n", mapname);
		trap_FS_FCloseFile(f);
		return;
	}

	if (f) {
		char    line[MAX_STRING_CHARS];

		G_Printf("\nLoading viewcam positions from \"%s\"\n", filename);
		trap_FS_Read(buf, len, f);
		p = buf;
		buf[len] = 0;

		do {
			line[0] = 0;
			p = OSP_ReadConfigLine(p, len - (p - buf), line, sizeof(line));
			if (line[0]) {
				osp_viewcams[osp_numViewcams].range = -1;
				sscanf(line, "%i %f %f %f %f %f %d",
				       &osp_viewcams[osp_numViewcams].id,
				       &osp_viewcams[osp_numViewcams].origin[0],
				       &osp_viewcams[osp_numViewcams].origin[1],
				       &osp_viewcams[osp_numViewcams].origin[2],
				       &osp_viewcams[osp_numViewcams].angles[0],
				       &osp_viewcams[osp_numViewcams].angles[1],
				       &osp_viewcams[osp_numViewcams].range);

				// an all-zero line is not a camera
				if (osp_viewcams[osp_numViewcams].id || osp_viewcams[osp_numViewcams].origin[0] != 0
				        || osp_viewcams[osp_numViewcams].origin[1] != 0 || osp_viewcams[osp_numViewcams].origin[2] != 0) {
					osp_viewcams[osp_numViewcams].target = -1;
					VectorCopy(osp_viewcams[osp_numViewcams].angles, osp_viewcams[osp_numViewcams].viewAngles);
					if (osp_viewcams[osp_numViewcams].range <= 50) {
						osp_viewcams[osp_numViewcams].range = 450;
					}
					osp_numViewcams++;
				}
			}
		} while (p && osp_numViewcams < MAX_OSP_VIEWCAMS);
	}

	if (osp_numViewcams) {
		osp_viewcamCurrent = -1;
		G_Printf("%d positions added to the viewcam for %s.\n\n", osp_numViewcams, mapname);
	} else {
		G_Printf("*** No positions found for viewcam in %s.\n", f);
		G_Printf("*** Viewcam spectator mode DISABLED for %s.\n\n", mapname);
	}

	trap_FS_FCloseFile(f);
}

/*
================
OSP_ViewcamCyclePosition

Puts ent on the viewcam (SPECTATOR_VIEWCAM) at the spot dir steps from his current one
(SpectatorThink: JUMP / CROUCH, FIRE).
================
*/
void OSP_ViewcamCyclePosition(gentity_t* ent, int dir) {
	int     cam;

	cam = ent->client->pers.viewcam;

	if (!osp_numViewcams) {
		trap_SendServerCommand(ent - g_entities, "print \"Viewcam disabled.\n");
		return;
	}

	if (g_gametype.integer == GT_TOURNAMENT && level.warmupTime == 0 && ent->client->sess.sessionTeam == TEAM_FREE) {
		ent->client->sess.losses++;
	}

	if (ent->client->sess.spectatorState != SPECTATOR_VIEWCAM) {
	}

	cam += dir;
	if (cam >= osp_numViewcams) {
		cam = 0;
	}
	if (cam < 0) {
		cam = osp_numViewcams - 1;
	}

	ent->r.svFlags &= ~SVF_BOT;
	ent->client->ps.clientNum = ent - g_entities;
	ent->client->ps.persistant[PERS_TEAM] = TEAM_SPECTATOR;
	ent->client->sess.sessionTeam = TEAM_SPECTATOR;
	ent->client->sess.spectatorState = SPECTATOR_VIEWCAM;
	ent->client->pers.viewcam = cam;
	VectorCopy(osp_viewcams[cam].origin, ent->s.origin);
	VectorCopy(osp_viewcams[cam].origin, ent->client->ps.origin);
	VectorCopy(osp_viewcams[cam].angles, ent->client->ps.viewangles);
	ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
}

/*
================
OSP_ViewcamRunFrame

Chooses the action camera shown to the viewcam spectators (G_RunFrame, while any).
================
*/
void OSP_ViewcamRunFrame(void) {
	int     i;
	int     best;
	int     bestDist;

	if (!level.numPlayingClients || !osp_numViewcams) {
		return;
	}

	// keep the current camera for a while if it still sees enough
	if (osp_viewcamCurrent >= 0 && osp_viewcamHoldTime > level.time && OSP_ViewcamUpdate(osp_viewcamCurrent, qtrue)) {
		if (osp_viewcams[osp_viewcamCurrent].activity > 600 || osp_viewcamBestActivity < 600) {
			if (osp_viewcams[osp_viewcamCurrent].activity > 1) {
				osp_viewcamHoldTime = level.time + 1600;
			}
			if (osp_viewcams[osp_viewcamCurrent].activity > 600) {
				osp_viewcamHoldTime += 500;
			}
			return;
		}
	}

	best = osp_viewcams[osp_viewcamCurrent].activity;
	for (i = 0; i < osp_numViewcams; i++) {
		if (i == osp_viewcamCurrent && level.numPlayingClients > 1) {
			continue;
		}
		if (OSP_ViewcamUpdate(i, qfalse) > best) {
			best = osp_viewcams[i].activity;
		}
		if (best > 600) {
			break;
		}
	}

	if (!best) {
		return;
	}

	if (best < 600 && osp_viewcamBestActivity > 600 && osp_viewcamHoldTime > level.time) {
		osp_viewcamCurrent = -1;
		return;
	}

	osp_viewcamBestActivity = best;
	osp_viewcamLast = osp_viewcamCurrent;

	// the busiest camera with the players closest on average
	bestDist = 999999999;
	for (i = 0; i < osp_numViewcams; i++) {
		if (osp_viewcams[i].activity == best) {
			if (osp_viewcams[i].distSum / osp_viewcams[i].activity < bestDist) {
				bestDist = osp_viewcams[i].distSum / osp_viewcams[i].activity;
				osp_viewcamCurrent = i;
			}
		}
	}

	if (osp_viewcams[osp_viewcamCurrent].activity) {
		osp_viewcamHoldTime = level.time + 1600;
	}
}

/*
================
OSP_ViewcamUpdate

Rates camera cam: the players it sees (in range, in front of it, not blocked) with bonuses
for quad and flag carriers, and the angles to look at them (dynamic: from the camera's current
view angles). Returns the activity.
================
*/
int OSP_ViewcamUpdate(int cam, qboolean dynamic) {
	gclient_t*   cl;
	vec3_t      dir;
	int         i;
	int         bonus;
	float       dist;
	vec3_t      forward;
	qboolean    found;
	trace_t     tr;

	osp_viewcams[cam].closestDist = 999999999.0f;
	osp_viewcams[cam].distSum = 0;
	osp_viewcams[cam].activity = 0;
	found = qfalse;

	for (i = 0; i < level.maxclients; i++) {
		cl = &level.clients[i];
		bonus = 0;

		if (cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam == TEAM_SPECTATOR) {
			continue;
		}

		VectorSubtract(cl->ps.origin, osp_viewcams[cam].origin, dir);
		dist = VectorNormalize(dir);
		if (dist > osp_viewcams[cam].range) {
			continue;
		}

		if (dynamic)
			AngleVectors(osp_viewcams[cam].viewAngles, forward, NULL, NULL);
		else
			AngleVectors(osp_viewcams[cam].angles, forward, NULL, NULL);
		if (DotProduct(dir, forward) < 0.45f) {
			continue;
		}

		trap_Trace(&tr, cl->ps.origin, NULL, NULL, osp_viewcams[cam].origin, ENTITYNUM_NONE, CONTENTS_SOLID);
		if (tr.fraction != 1.0) {
			continue;
		}

		if (cl->ps.powerups[PW_QUAD]) {
			bonus += 700;
		}
		if (cl->ps.powerups[PW_BLUEFLAG]) {
			bonus += 1000;
		}
		if (cl->ps.powerups[PW_REDFLAG]) {
			bonus += 1000;
		}

		osp_viewcams[cam].activity += bonus + 1;
		osp_viewcams[cam].distSum += dist;

		// a carrier is always the one to look at, then the current target, else the closest
		if (bonus) {
			found = qtrue;
			vectoangles(dir, osp_viewcams[cam].viewAngles);
			osp_viewcams[cam].target = i;
			osp_viewcams[cam].closestClient = i;
			continue;
		}
		if (i == osp_viewcams[cam].target) {
			vectoangles(dir, osp_viewcams[cam].viewAngles);
			found = qtrue;
			continue;
		}
		if (!found && dist < osp_viewcams[cam].closestDist) {
			osp_viewcams[cam].closestDist = dist;
			osp_viewcams[cam].closestClient = i;
			vectoangles(dir, osp_viewcams[cam].closestAngles);
		}
	}

	if (osp_viewcams[cam].activity) {
		if (!found) {
			osp_viewcams[cam].target = osp_viewcams[cam].closestClient;
			VectorCopy(osp_viewcams[cam].closestAngles, osp_viewcams[cam].viewAngles);
		}
	} else {
		osp_viewcams[cam].target = -1;
	}

	return osp_viewcams[cam].activity;
}

/*
================
OSP_ViewcamCyclePlayer

Puts ent on the viewcam following a player (SPECTATOR_VIEWCAM_FOLLOW), the next playing
client dir steps from the one he follows. Returns 0 when there is none.
================
*/
int OSP_ViewcamCyclePlayer(gentity_t* ent, int dir) {
	int     clientnum;
	int     original;

	if (!osp_numViewcams) {
		trap_SendServerCommand(ent - g_entities, "print \"Viewcam disabled.\n\"");
		return 0;
	}

	if (ent->client->sess.nameTeam) {
		trap_SendServerCommand(ent - g_entities, "print \"Not allowed to switch from current team.\n\"");
		return 0;
	}

	if (g_gametype.integer == GT_TOURNAMENT && level.warmupTime == 0 && ent->client->sess.sessionTeam == TEAM_FREE) {
		ent->client->sess.losses++;
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
		SetTeam(ent, "viewcam");
	} else if (ent->client->sess.spectatorState == SPECTATOR_VIEWCAM) {
		osp_numViewcamClients--;
	}

	do {
		clientnum += dir;
		if (clientnum >= level.maxclients) {
			clientnum = 0;
		}
		if (clientnum < 0) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if (level.clients[ clientnum ].pers.connected != CON_CONNECTED) {
			continue;
		}

		// can't follow another spectator
		if (level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR) {
			continue;
		}

		ent->r.svFlags &= ~SVF_BOT;
		ent->client->ps.clientNum = ent - g_entities;
		ent->client->ps.persistant[PERS_TEAM] = TEAM_SPECTATOR;
		ent->client->sess.sessionTeam = TEAM_SPECTATOR;
		ent->client->sess.spectatorState = SPECTATOR_VIEWCAM_FOLLOW;
		ent->client->sess.spectatorClient = clientnum;
		ent->client->pers.viewcam = 0;
		VectorCopy(osp_viewcams[0].origin, ent->s.origin);
		VectorCopy(osp_viewcams[0].origin, ent->client->ps.origin);
		VectorCopy(osp_viewcams[0].angles, ent->client->ps.viewangles);
		VectorCopy(osp_viewcams[0].angles, ent->client->pers.viewcamAngles);
		ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
		OSP_ViewcamThink(ent, 400);
		return 1;
	} while (clientnum != original);

	trap_SendServerCommand(ent - g_entities, "print \"No clients to follow.\n");
	return 0;
}

/*
================
OSP_ViewcamThink

Viewcam following a player (SPECTATOR_VIEWCAM_FOLLOW): keeps the current camera while it sees
the player (range + rangeBonus), else moves to the closest camera that does.
================
*/
void OSP_ViewcamThink(gentity_t* ent, int rangeBonus) {
	int         i;
	vec3_t      dir;
	gclient_t*   cl;
	vec3_t      forward;
	float       dist;
	trace_t     tr;
	int         bestDist;
	int         best;
	vec3_t      angles;

	if (!level.numPlayingClients || !osp_numViewcams || ent->client->sess.spectatorClient < 0) {
		ent->client->sess.spectatorClient = -1;
		OSP_ViewcamCyclePosition(ent, 1);
		return;
	}

	if (level.clients[ent->client->sess.spectatorClient].pers.connected != CON_CONNECTED
	        || level.clients[ent->client->sess.spectatorClient].sess.sessionTeam == TEAM_SPECTATOR) {
		if (!OSP_ViewcamCyclePlayer(ent, 1)) {
			ent->client->sess.spectatorClient = -1;
			trap_SendServerCommand(ent - g_entities, "print \"^3Player disconnected, switching to next player.\n\"");
			OSP_ViewcamCyclePosition(ent, 1);
			return;
		}
	}

	best = -1;
	bestDist = 99999999;
	cl = level.clients + ent->client->sess.spectatorClient;

	// keep the current camera if it still sees the player
	VectorSubtract(cl->ps.origin, ent->client->ps.origin, dir);
	dist = VectorNormalize(dir);
	if (dist <= osp_viewcams[ent->client->pers.viewcam].range + rangeBonus) {
		AngleVectors(ent->client->pers.viewcamAngles, forward, NULL, NULL);
		if (DotProduct(dir, forward) >= 0.45f) {
			trap_Trace(&tr, cl->ps.origin, NULL, NULL, ent->client->ps.origin, ENTITYNUM_NONE, CONTENTS_SOLID);
			if (tr.fraction == 1.0) {
				if (!ent->client->pers.viewcamStatic) {
					vectoangles(dir, ent->client->pers.viewcamAngles);
				}
				VectorCopy(ent->client->pers.viewcamAngles, ent->client->ps.viewangles);
				return;
			}
		}
	}

	for (i = 0; i < osp_numViewcams; i++) {
		if (i == ent->client->pers.viewcam) {
			continue;
		}

		VectorSubtract(cl->ps.origin, osp_viewcams[i].origin, dir);
		dist = VectorNormalize(dir);
		if (dist > osp_viewcams[i].range + rangeBonus) {
			continue;
		}

		AngleVectors(osp_viewcams[i].angles, forward, NULL, NULL);
		if (DotProduct(dir, forward) < 0.45f) {
			continue;
		}

		trap_Trace(&tr, cl->ps.origin, NULL, NULL, osp_viewcams[i].origin, ENTITYNUM_NONE, CONTENTS_SOLID);
		if (tr.fraction != 1.0) {
			continue;
		}

		if (dist < bestDist) {
			best = i;
			bestDist = dist;
			if (!ent->client->pers.viewcamStatic) {
				vectoangles(dir, angles);
			} else {
				VectorCopy(osp_viewcams[i].angles, angles);
			}
		}
	}

	if (best < 0) {
		VectorCopy(ent->client->pers.viewcamAngles, ent->client->ps.viewangles);
		return;
	}

	ent->client->pers.viewcam = best;
	VectorCopy(osp_viewcams[best].origin, ent->s.origin);
	VectorCopy(osp_viewcams[best].origin, ent->client->ps.origin);
	VectorCopy(angles, ent->client->ps.viewangles);
	VectorCopy(angles, ent->client->pers.viewcamAngles);
	ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
	ent->client->pers.viewcamHoldTime = level.time + 1600;
}

/*
================
OSP_ViewcamRandomSpot

Intermission point from a random camera spot (FindIntermissionPoint; CheckIntermissionExit
every 8 seconds with moveClients set). Returns 0 without viewcam.
================
*/
int OSP_ViewcamRandomSpot(int moveClients) {
	if (!osp_numViewcams) {
		return 0;
	}

	osp_viewcamRandomSpot = (int)(random() * osp_numViewcams) % osp_numViewcams;
	VectorCopy(osp_viewcams[osp_viewcamRandomSpot].origin, level.intermission_origin);
	VectorCopy(osp_viewcams[osp_viewcamRandomSpot].angles, level.intermission_angle);

	if (moveClients) {
		int     i;

		for (i = 0; i < level.maxclients; i++) {
			if (!g_entities[i].inuse) {
				continue;
			}
			VectorCopy(level.intermission_origin, g_entities[i].s.origin);
			VectorCopy(level.intermission_origin, g_entities[i].client->ps.origin);
			VectorCopy(level.intermission_angle, g_entities[i].client->ps.viewangles);
			g_entities[i].client->ps.eFlags ^= EF_TELEPORT_BIT;
		}
	}

	osp_viewcamRandomTime = level.time + 8000;
	return 1;
}

/*
================
OSP_Cmd_Snapshot_f

"snapshot": prints the current position as a viewcam file line and records it.
================
*/
void OSP_Cmd_Snapshot_f(gentity_t* ent) {
	trap_SendServerCommand(ent - g_entities, va("print \"%d %.3f %.3f %.3f %.3f %.3f\n",
	                       ent->client->pers.snapshotCount++,
	                       ent->client->ps.origin[0], ent->client->ps.origin[1], ent->client->ps.origin[2],
	                       ent->client->ps.viewangles[0], ent->client->ps.viewangles[1]));

	if (osp_numSnapshots < MAX_OSP_SNAPSHOTS) {
		VectorCopy(ent->client->ps.origin, osp_snapshotOrigins[osp_numSnapshots]);
		VectorCopy(ent->client->ps.viewangles, osp_snapshotAngles[osp_numSnapshots]);
		osp_numSnapshots++;
		trap_SendServerCommand(ent - g_entities, va("cp \"Snapshot #%d recorded.\n\"", osp_numSnapshots));
	}

	// also logged on a listen server with one client
	if (!g_dedicated.integer && level.numConnectedClients == 1) {
		G_LogPrintf("%d %.3f %.3f %.3f %.3f %.3f\n", osp_numSnapshots - 1,
		            osp_snapshotOrigins[osp_numSnapshots - 1][0], osp_snapshotOrigins[osp_numSnapshots - 1][1],
		            osp_snapshotOrigins[osp_numSnapshots - 1][2], osp_snapshotAngles[osp_numSnapshots - 1][0],
		            osp_snapshotAngles[osp_numSnapshots - 1][1]);
	}
}

/*
================
OSP_Cmd_SnapshotDump_f

"snapshotdump"
================
*/
void OSP_Cmd_SnapshotDump_f(gentity_t* ent) {
	int     i;

	if (!osp_numSnapshots) {
		trap_SendServerCommand(ent - g_entities, "print \"No snapshots have been taken.\n\"");
		return;
	}

	for (i = 0; i < osp_numSnapshots; i++) {
		trap_SendServerCommand(ent - g_entities, va("print \"%d %.3f %.3f %.3f %.3f %.3f\n\"", i,
		                       osp_snapshotOrigins[i][0], osp_snapshotOrigins[i][1], osp_snapshotOrigins[i][2],
		                       osp_snapshotAngles[i][0], osp_snapshotAngles[i][1]));
		if (!g_dedicated.integer && level.numConnectedClients == 1) {
			G_LogPrintf("%d %.3f %.3f %.3f %.3f %.3f\n", i,
			            osp_snapshotOrigins[i][0], osp_snapshotOrigins[i][1], osp_snapshotOrigins[i][2],
			            osp_snapshotAngles[i][0], osp_snapshotAngles[i][1]);
		}
	}
}

/*
================
OSP_Cmd_VCProximity_f

"vc_proximity": distance to the closest camera spot and snapshot.
================
*/
void OSP_Cmd_VCProximity_f(gentity_t* ent) {
	int     i;
	vec3_t  dir;
	int     dist;
	int     camDist;
	int     snapDist;
	int     cam;
	int     snap;

	cam = 0;
	camDist = 99999999;
	for (i = 0; i < osp_numViewcams; i++) {
		VectorSubtract(ent->client->ps.origin, osp_viewcams[i].origin, dir);
		dist = VectorNormalize(dir);
		if (dist < camDist) {
			camDist = dist;
			cam = i;
		}
	}

	snap = 0;
	snapDist = 99999999;
	for (i = 0; i < osp_numSnapshots; i++) {
		VectorSubtract(ent->client->ps.origin, osp_snapshotOrigins[i], dir);
		dist = VectorNormalize(dir);
		if (dist < snapDist) {
			snapDist = dist;
			snap = i;
		}
	}

	if (osp_numViewcams && osp_numSnapshots) {
		trap_SendServerCommand(ent - g_entities, va("cp \"^3Near ViewCam:^5 %d, id=%d\n^3Near Snapshot:^5 %d, id=%d\n\"",
		                       camDist, osp_viewcams[cam].id, snapDist, snap));
	} else if (osp_numViewcams) {
		trap_SendServerCommand(ent - g_entities, va("cp \"^3Near ViewCam:^5 %d, id=%d\n\"", camDist, osp_viewcams[cam].id));
	} else if (osp_numSnapshots) {
		trap_SendServerCommand(ent - g_entities, va("cp \"^3Near Snapshot:^5 %d, id=%d\n\"", snapDist, snap));
	} else {
		trap_SendServerCommand(ent - g_entities, "cp \"No other viewcam positions available.\n\"");
	}
}

/*
================
OSP_Cmd_Range_f

"range": distance to the surface in the crosshair, yellow within the default camera range
(450), red beyond.
================
*/
void OSP_Cmd_Range_f(gentity_t* ent) {
	vec3_t  forward, right, up;
	vec3_t  end;
	vec3_t  dir;
	trace_t tr;
	int     dist;

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
		return;
	}
	if (ent->client->sess.coachTeam) {
		return;
	}

	AngleVectors(ent->client->ps.viewangles, forward, right, up);
	VectorMA(ent->client->ps.origin, 8192, forward, end);
	trap_Trace(&tr, ent->client->ps.origin, NULL, NULL, end, ent->s.number, MASK_SHOT);
	VectorSubtract(ent->client->ps.origin, tr.endpos, dir);
	dist = VectorNormalize(dir);

	if (dist <= 450) {
		trap_SendServerCommand(ent - g_entities, va("cp \"Current range: ^3%d units\n\"", dist));
	} else {
		trap_SendServerCommand(ent - g_entities, va("cp \"Current range: ^1%d units\n\"", dist));
	}
}

/*
================
OSP_Cmd_Viewcam_f

"viewcam"
================
*/
void OSP_Cmd_Viewcam_f(gentity_t* ent) {
	if (!osp_numViewcams) {
		trap_SendServerCommand(ent - g_entities, "print \"\nSorry, ^3ViewCam^7 mode has been disabled for this map\n(No camera positions defined)\n\n\"");
		return;
	}

	if (ent->client->sess.nameTeam) {
		trap_SendServerCommand(ent - g_entities, "print \"Not allowed to switch from current team.\n\"");
		return;
	}

	if (ent->client->sess.coachTeam) {
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR && (ent->client->sess.spectatorState == SPECTATOR_VIEWCAM
	        || ent->client->sess.spectatorState == SPECTATOR_VIEWCAM_FOLLOW)) {
		return;
	}

	ent->client->pers.viewcam = 0;
	ent->client->pers.mvSavedCount = 0;

	if (ent->client->pers.followViewcam && level.numPlayingClients) {
		OSP_Cmd_VCFollow_f(ent);
	} else {
		SetTeam(ent, "viewcam");
	}

	if (level.numPlayingClients && !ent->client->pers.viewcamFree && !ent->client->pers.followViewcam) {
		trap_SendServerCommand(ent - g_entities, "print \"^5VIEWCAM:^3  Hit ^7FIRE^3 to go to FreeFloat.\n");
		return;
	}

	if (ent->client->pers.followViewcam && level.numPlayingClients) {
		return;
	}

	trap_SendServerCommand(ent - g_entities, "print \"^5VIEWCAM:^3  Hit ^7FIRE^3 to go to FreeFloat, ^7JUMP^3 to cycle positions.\n");
}

/*
================
OSP_Cmd_VCFollow_f

"vc_follow [player]": the viewcam follows one player.
================
*/
void OSP_Cmd_VCFollow_f(gentity_t* ent) {
	if (!osp_numViewcams) {
		trap_SendServerCommand(ent - g_entities, "print \"\nSorry, ^3ViewCam^7 mode has been disabled for this map\n(No camera positions defined)\n\n\"");
		return;
	}

	if (!level.numPlayingClients) {
		trap_SendServerCommand(ent - g_entities, "print \"\nSorry, no players to follow\n\n\"");
		return;
	}

	if (ent->client->sess.coachTeam) {
		return;
	}

	if (trap_Argc() == 1) {
		if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW) {
			if (--ent->client->sess.spectatorClient < 0) {
				ent->client->sess.spectatorClient = level.maxclients - 1;
			}
		} else {
			ent->client->sess.spectatorClient = level.maxclients - 1;
		}
	} else {
		int     pid;
		char    arg[MAX_TOKEN_CHARS];

		trap_Argv(1, arg, sizeof(arg));
		if ((pid = ClientNumberFromString(ent, arg)) == -1) {
			OSP_PrintInvalidPlayerID(ent);
			return;
		}
		if (level.clients[pid].sess.sessionTeam == TEAM_SPECTATOR) {
			trap_SendServerCommand(ent - g_entities, "print \"\nYou can't follow spectators!\n\"");
			return;
		}
		// OSP_ViewcamCyclePlayer moves one step on
		if (--pid < 0) {
			pid = level.maxclients - 1;
		}
		ent->client->sess.spectatorClient = pid;
	}

	if (ent->client->pers.mvCount) {
		OSP_MV_RemoveAll(ent);
	}
	OSP_ViewcamCyclePlayer(ent, 1);
	trap_SendServerCommand(ent - g_entities, "print \"^5VIEWCAM:^3  Hit ^7FIRE^3 to go to FreeFloat, ^7JUMP^3 to cycle through players.\n");
}
