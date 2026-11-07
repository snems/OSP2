/*
 * g_osp_customgfx.c: server custom graphics (osp-q3-ServerGfx.txt).
 */

#include "g_local.h"

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t    server_gfxspecfile;

// OSP: configstrings of the graphics specifications
#define CS_OSP_CUSTOMGFX        824
#define MAX_OSP_CUSTOMGFX       16

/*
==================
OSP_LoadCustomGfx

Sends the lines of server_gfxspecfile as configstrings CS_OSP_CUSTOMGFX + n; unused ones
are "-1:-1"
==================
*/
void OSP_LoadCustomGfx(void) {
	int             len;
	char*            p;
	int             count;
	char            filename[MAX_QPATH];
	fileHandle_t    f;
	char*            disabled;
	char            buf[MAX_OSP_FILE_CHARS];

	count = 0;
	disabled = "*** Server custom graphics DISABLED! ***\n\n";

	for (len = 0; len < MAX_OSP_CUSTOMGFX; len++) {
		buf[0] = 0;
		trap_GetConfigstring(len + CS_OSP_CUSTOMGFX, buf, sizeof(buf));
		if (!buf[0]) {
			trap_SetConfigstring(len + CS_OSP_CUSTOMGFX, "-1:-1");
		}
	}

	buf[0] = 0;
	filename[0] = 0;
	if (server_gfxspecfile.string[0] && Q_stricmp(server_gfxspecfile.string, "none")) {
		strcpy(filename, server_gfxspecfile.string);
	} else {
		G_Printf("\n%s", disabled);
		return;
	}

	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if (len <= 0) {
		G_Printf("\n>>> ERROR: Could not open graphics specification file \"%s\"\n", filename);
		G_Printf("%s", disabled);
		return;
	}
	if (len >= sizeof(buf)) {
		G_Printf("\n>>> ERROR: Graphics specification file \"%s\" too big! (%d bytes, max = %d)\n", filename, len, sizeof(buf) - 1);
		G_Printf("%s", disabled);
		return;
	}

	if (f) {
		G_Printf("\nLoading graphics specification from \"%s\"\n", filename);
		trap_FS_Read(buf, len, f);
		p = buf;
		buf[len] = 0;
		count = 0;

		do {
			char    line[MAX_STRING_CHARS];

			line[0] = 0;
			p = OSP_ReadConfigLine(p, len - (p - buf), line, sizeof(line));
			if (line[0]) {
				trap_SetConfigstring(count + CS_OSP_CUSTOMGFX, line);
				G_Printf("GFX Spec entry: %s\n", line);
				count++;
			}
		} while (p && count < MAX_OSP_CUSTOMGFX);
	}

	G_Printf("%d graphics specifications defined.\n", count);
	trap_FS_FCloseFile(f);
}
