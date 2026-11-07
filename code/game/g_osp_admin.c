/*
 * g_osp_admin.c: OSP's admin log (admin_log).
 */

#include "g_local.h"

// OSP: vmCvar_ts registered in gameCvarTable (g_main.c); their order comes from the extern
// list in g_local.h.
vmCvar_t    admin_log;

// OSP: OSP_AdminLogPrintf
// sets it when admin_log is "none" or can't be opened, G_UpdateCvars clears it when admin_log changes
int     osp_adminLogDisabled = 0;

/*
=================
OSP_AdminLogPrintf

Appends a time-stamped line to the admin_log file (and echoes it on a dedicated server).
OSP: not variadic - every caller passes a finished va() string. The month table is
the function's initialised local.
=================
*/
void OSP_AdminLogPrintf(const char* text) {
	qtime_t     qt;
	char        buf[4096];
	fileHandle_t    f;
	char*        months[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	if (!admin_log.string[0] || !Q_stricmp(admin_log.string, "none") || osp_adminLogDisabled) {
		osp_adminLogDisabled = 1;
		return;
	}

	trap_FS_FOpenFile(admin_log.string, &f, FS_APPEND_SYNC);
	if (!f) {
		osp_adminLogDisabled = 1;
		G_Printf("*** ERROR: Couldn't open admin log \"%s\"\n\n", admin_log.string);
		return;
	}

	trap_RealTime(&qt);
	Q_strncpyz(buf, va("%02d %s %d (%02d:%02d:%02d)\t%s\n", qt.tm_mday, months[qt.tm_mon], qt.tm_year + 1900,
	                   qt.tm_hour, qt.tm_min, qt.tm_sec, text), sizeof(buf));
	trap_FS_Write(buf, strlen(buf), f);
	trap_FS_FCloseFile(f);

	if (g_dedicated.integer) {
		G_Printf("%s\n", text);
	}
}
