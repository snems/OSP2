/*
 * g_osp_decals.c: map decals (<decal_directory>/decal-<map>.txt, else the
 * built-in entries of OSP_DecalEntry).
 */

#include "g_local.h"

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t    decal_directory;

// OSP: configstrings of the decals
#define CS_OSP_DECALS       840
#define MAX_OSP_DECALS      32

typedef struct {
	char*    mapname;
	char*    entries[7];
} ospDecal_t;

/*
==================
OSP_DecalEntry

Built-in decals of mapname (OSP's maps and a few id maps); 0 when there are none
==================
*/
int OSP_DecalEntry(char* mapname) {
	int         j;
	int         i;
	ospDecal_t  decals[] = {
		{ "q3dm6", { "0 35 -90.00 -1647.87 60.24 864.74 1.00 0.00 0.00" } },
		{ "pro-q3dm6", { "0 35 -90.00 -1647.87 60.24 864.74 1.00 0.00 0.00" } },
		{ "q3dm7", { "0 71 444.00 516.60 -802.06 -147.61 -0.19 0.00 0.98" } },
		{ "q3dm12", { "0 26 180.00 1219.94 736.88 252.90 -0.70 -0.70 0.00" } },
		{ "q3dm14", { "0 43 266.50 -830.34 -992.12 -243.21 0.00 -1.00 0.00" } },
		{ "q3ctf1", { "0 39 90.00 768.02 -1219.87 375.14 0.00 1.00 0.00" } },
		{ "q3ctf2", { "0 30 232.13 1051.54 332.80 -303.87 0.00 0.00 1.00" } },
		{ "q3ctf3", { "0 142 93.56 31.01 578.20 0.12 0.00 0.00 1.00" } },
		{ "q3wctf1", { "0 80 292.60 -201.94 470.84 544.12 0.00 0.00 1.00" } },
		{ "q3wctf2", { "0 33 180.00 -539.91 -731.90 224.94 0.70 0.70 0.00" } },
		{ "q3wctf3", { "0 30 -70.00 -1859.87 -1025.31 258.76 1.00 0.00 0.00" } },
		{ "ospdm1", { "0 42 407.00 -328.12 -842.30 713.83 -1.00 0.00 0.00" } },
		{ "ospdm2", { "0 33 257.00 1664.12 68.96 -34.50 1.00 0.00 0.00" } },
		{ "ospdm3", { "0 35 75.17 -415.69 -517.34 319.87 0.00 0.00 -1.00" } },
		{ "ospdm4", { "0 50 -90.00 -879.87 1458.00 131.83 1.00 0.00 0.00", "0 66 -34.50 -606.23 -0.12 331.54 0.00 -1.00 0.00" } },
		{ "ospdm5", { "0 39 257.50 -88.90 -912.12 791.09 0.00 -1.00 0.00", "0 30 156.00 934.27 1243.88 587.70 0.00 0.70 -0.70" } },
		{ "ospdm6", { "0 39 430.50 -672.12 674.90 558.40 -1.00 0.00 0.00" } },
		{ "ospdm7", { "0 30 90.00 -392.12 -118.70 151.97 -1.00 0.00 0.00" } },
		{ "ospdm8", { "0 38 90.00 -1407.66 -37.02 240.95 -0.89 0.00 0.44" } },
		{ "ospdm9", { "0 30 90.00 1086.87 256.12 607.70 0.00 1.00 0.00" } },
		{ "ospdm10", { "0 42 407.00 -328.12 -842.30 713.83 -1.00 0.00 0.00" } },
		{ "ospdm11", { "0 27 436.00 64.92 -1455.28 1334.68 0.00 0.99 0.01", "0 175 346.05 -677.29 1655.18 448.12 0.00 0.00 1.00" } },
		{ "ospdm12", { "0 30 -90.00 1488.12 1178.77 546.96 1.00 0.00 0.00", "0 84 156.94 386.53 -78.33 256.12 0.00 0.00 1.00" } },
		{ "ospctf1", { "0 30 180.00 496.95 -239.21 603.08 0.70 -0.70 0.00", "0 30 180.00 -3409.27 3297.44 725.90 0.70 0.70 0.00" } },
		{ "ospctf2", { "0 18 525.50 -331.73 -892.08 237.37 0.70 0.70 0.00", "0 30 90.00 2632.80 1656.12 364.87 0.00 1.00 0.00" } }
	};
	int         count;

	count = 0;
	for (i = 0; i < sizeof(decals) / sizeof(decals[0]); i++) {
		if (!Q_stricmp(decals[i].mapname, mapname)) {
			for (j = 0; j < 7; j++) {
				if (decals[i].entries[j] && *decals[i].entries[j]) {
					trap_SetConfigstring(count + CS_OSP_DECALS, decals[i].entries[j]);
					G_Printf("INTERNAL Decal entry: %s\n", decals[i].entries[j]);
					count++;
				} else {
					break;
				}
			}
			return 1;
		}
	}
	return 0;
}

/*
==================
OSP_LoadDecals

Sends the lines of the map's decal file (or its built-in decals) as configstrings
CS_OSP_DECALS + n
==================
*/
void OSP_LoadDecals(void) {
	int             len;
	char*            p;
	int             count;
	char            filename[MAX_QPATH];
	char            mapname[MAX_STRING_CHARS];
	fileHandle_t    f;
	char            serverinfo[MAX_INFO_STRING];
	char*            disabled;
	char            buf[MAX_OSP_FILE_CHARS];

	count = 0;
	disabled = "*** Map decals disabled\n\n";

	for (len = 0; len < MAX_OSP_DECALS; len++) {
		buf[0] = 0;
		trap_GetConfigstring(len + CS_OSP_DECALS, buf, sizeof(buf));
		if (!buf[0]) {
			trap_SetConfigstring(len + CS_OSP_DECALS, "-1");
		}
	}

	buf[0] = 0;
	filename[0] = 0;
	trap_GetServerinfo(serverinfo, sizeof(serverinfo));
	Q_strncpyz(mapname, Info_ValueForKey(serverinfo, "mapname"), sizeof(mapname));
	strcpy(filename, va("%s/decal-%s.txt", decal_directory.string, mapname));

	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if (len <= 0) {
		G_Printf("\n>>> ERROR: Could not open decal file \"%s\"\n", filename);
		if (!OSP_DecalEntry(mapname)) {
			G_Printf("%s", disabled);
		}
		return;
	}
	if (len >= sizeof(buf)) {
		G_Printf("\n>>> ERROR: Decal file \"%s\" too big! (%d bytes, max = %d)\n", filename, len, sizeof(buf) - 1);
		if (!OSP_DecalEntry(mapname)) {
			G_Printf("%s", disabled);
		}
		trap_FS_FCloseFile(f);
		return;
	}

	if (f) {
		G_Printf("\nLoading decals from \"%s\"\n", filename);
		trap_FS_Read(buf, len, f);
		p = buf;
		buf[len] = 0;
		count = 0;

		do {
			char    line[MAX_STRING_CHARS];

			line[0] = 0;
			p = OSP_ReadConfigLine(p, len - (p - buf), line, sizeof(line));
			if (line[0]) {
				trap_SetConfigstring(count + CS_OSP_DECALS, line);
				G_Printf("Decal entry: %s\n", line);
				count++;
			}
		} while (p && count < MAX_OSP_DECALS);
	}

	G_Printf("%d decals defined.\n", count);
	trap_FS_FCloseFile(f);
}
