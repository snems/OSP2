#include "cg_local.h"
#include "../qcommon/qcommon.h"

#define OSP_CUSTOM_LOCATIONS_MAX 80

typedef struct
{
	vec3_t pos;
	char name[MAX_QPATH];
} customLocation_t;

#define MAX_CTF_LOCATION_LENGTH 17
char ctfLocationsColors[MAX_LOCATIONS][MAX_CTF_LOCATION_LENGTH];


static customLocation_t customLocationEntry[OSP_CUSTOM_LOCATIONS_MAX];
static int numberOfCustomLocations;
qboolean customLocationsEnabled = qfalse;

void CG_CustomLocationsLoad(void)
{
	const char* ptr;
	const char* name;
	char path[MAX_QPATH];
	int rc;
	int matches;
	char* fileContent;
	fileHandle_t locFileHandle;
	char line[1024];
	int line_num = 0;

	numberOfCustomLocations = 0;
	customLocationsEnabled = qfalse;

	if (!cg_customLoc.integer)
	{
		return;
	}

	ptr = Info_ValueForKey(CG_ConfigString(0), "mapname");
	if (!ptr)
	{
		CG_Printf("^1Couldn't find mapname\n");
		return;
	}
	Com_sprintf(path, MAX_QPATH, "locs/%s.cfg", ptr);

	rc = trap_FS_FOpenFile(path, &locFileHandle, FS_READ);
	if (rc < 0 || !locFileHandle)
	{
		CG_Printf("^1locfile %s not found. Use addpos <text> to create locfile\n", path);
		numberOfCustomLocations = 0;
		return;
	}

	fileContent = Z_Malloc(rc + 1);
	OSP_MEMORY_CHECK(fileContent);

	trap_FS_Read(fileContent, rc, locFileHandle);
	trap_FS_FCloseFile(locFileHandle);
	fileContent[rc] = 0;


	ptr = fileContent;
	do
	{
		ptr = CG_LoadLine(ptr, &line[0], 1024);

		if (line[0])
		{
			matches = Q_sscanf(&line[0], "%f %f %f %s",
			                   &customLocationEntry[numberOfCustomLocations].pos[0],
			                   &customLocationEntry[numberOfCustomLocations].pos[1],
			                   &customLocationEntry[numberOfCustomLocations].pos[2],
			                   &customLocationEntry[numberOfCustomLocations].name[0]);
			name = strstr(&line[0], &customLocationEntry[numberOfCustomLocations].name[0]);
			if (name)
			{
				Q_strncpyz(&customLocationEntry[numberOfCustomLocations].name[0], name, MAX_QPATH);
			}
			++line_num;
			if (matches != 4)
			{
				CG_Printf("Ignored non-comment line %i\n", line_num);
				CG_Printf("Ignored non-comment line n %i\n", matches);
			}
			else
			{
				++numberOfCustomLocations;
				if (numberOfCustomLocations >= OSP_CUSTOM_LOCATIONS_MAX)
				{
					CG_Printf("^1Maximum number of locations read (%d)\n", OSP_CUSTOM_LOCATIONS_MAX);
					break;
				}
			}
		}
	}
	while (*ptr);
	CG_Printf("^3>>> (%s):^5 %i custom locations enabled.\n", path, numberOfCustomLocations);
	if (numberOfCustomLocations > 0)
	{
		customLocationsEnabled = qtrue;
	}

	Z_Free(fileContent);
}

const char* CG_CustomLocationsGetName(const float* pos)
{
	float x;
	float y;
	float z;
	customLocation_t* loc;
	int i;
	float curr_distance;
	float min_distance = 0;
	customLocation_t* loc2 = NULL;

	if (numberOfCustomLocations > 0)
	{
		for (i = 0; i < numberOfCustomLocations; ++i)
		{
			loc = &customLocationEntry[i];
			x = loc->pos[0] - pos[0];
			y = loc->pos[1] - pos[1];
			z = loc->pos[2] - pos[2];
			curr_distance = x * x + y * y + z * z;
			if (curr_distance < min_distance || !loc2)
			{
				min_distance = curr_distance;
				loc2 = loc;
			}
		}
	}
	if (loc2)
	{
		return loc2->name;
	}
	return "unknown";
}

void CG_CustomLocationsSetLocation(const char* info, vec3_t loc)
{
	int rc;
	int values[4];

	rc = Q_sscanf(info, "%i_%i_%i_%i", &values[0], &values[1], &values[2], &values[3]);
	if (rc == 4)
	{
		loc[0] = (float)values[1];
		loc[1] = (float)values[2];
		loc[2] = (float)values[3];
	}
	else
	{
		loc[0] = 0.0;
		loc[1] = 0.0;
		loc[2] = 0.0;
	}
}

qboolean CG_CustomLocationsTeamChatCode(const char* str, vec3_t cloc, char** cloc_begin, char** cloc_end)
{
	int rc;
	int x, y, z;

	/* find and decode string: ($x$y$z) */
	*cloc_begin = strstr(str, "($");
	if (*cloc_begin == NULL)
	{
		return qfalse;
	}
	++(*cloc_begin); //points to first $

	*cloc_end = strstr(*cloc_begin, ")");//points to )
	if (*cloc_end == NULL)
	{
		return qfalse;
	}

	rc = Q_sscanf(*cloc_begin, "$%i$%i$%i", &x, &y, &z);
	if (rc == 3)
	{
		cloc[0] = (float)x;
		cloc[1] = (float)y;
		cloc[2] = (float)z;
		return qtrue;
	}

	return qfalse;
}

static void CG_OSPCustomLocationsWrite(void)
{
	const char* ptr;
	char path[MAX_QPATH];
	char line[1024];
	int rc;
	fileHandle_t locFileHandle;
	int i;

	ptr = Info_ValueForKey(CG_ConfigString(0), "mapname");
	if (!ptr)
	{
		CG_Printf("^1Couldn't find mapname\n");
		return;
	}
	Com_sprintf(path, MAX_QPATH, "locs/%s.cfg", ptr);

	rc = trap_FS_FOpenFile(path, &locFileHandle, FS_WRITE);
	if (rc < 0 || !locFileHandle)
	{
		CG_Printf("^1Couldn't create locfile %s.\n", path);
		return;
	}
	for (i = 0; i < numberOfCustomLocations; ++i)
	{
		Com_sprintf(line, 1024, "%i %i %i %s\n",
		            (int)customLocationEntry[i].pos[0],
		            (int)customLocationEntry[i].pos[1],
		            (int)customLocationEntry[i].pos[2],
		            customLocationEntry[i].name);

		trap_FS_Write(line, strlen(line), locFileHandle);
	}
	trap_FS_FCloseFile(locFileHandle);
	if (numberOfCustomLocations > 0)
	{
		CG_Printf("^3>>> Saved location \"%s^3\" to %s\n", customLocationEntry[i - 1].name, path);
	}
}

void CG_CustomLocationsAddEntry(vec3_t pos, const char* str)
{
	if (numberOfCustomLocations >= OSP_CUSTOM_LOCATIONS_MAX)
	{
		CG_Printf("^1Maximum of location limit reached\n");
	}
	else
	{
		customLocation_t* cl = &customLocationEntry[numberOfCustomLocations];
		VectorCopy(pos, cl->pos);
		Q_strncpyz(cl->name, str, MAX_QPATH);
		++numberOfCustomLocations;
		CG_OSPCustomLocationsWrite();
	}
}

void CG_InitCTFLocations(void)
{
	int i;
	const char* locStrCS;
	char* locStr;
	char tmp[5];

	for (i = 0; i < MAX_LOCATIONS; ++i)
	{
		locStrCS = CG_ConfigString(CS_LOCATIONS + i);
		if (cgs.gametype == GT_CTF)
		{
			if (Q_stricmpn("blue", locStrCS, 4) == 0)
			{
				Q_strncpyz(&ctfLocationsColors[i][0], S_COLOR_CYAN, MAX_CTF_LOCATION_LENGTH);
				Q_strncpyz(&ctfLocationsColors[i][2], locStrCS, MAX_CTF_LOCATION_LENGTH - 2);
			}
			else if (Q_stricmpn("red", locStrCS, 3) == 0)
			{
				Q_strncpyz(&ctfLocationsColors[i][0], S_COLOR_RED, MAX_CTF_LOCATION_LENGTH);
				Q_strncpyz(&ctfLocationsColors[i][2], locStrCS, MAX_CTF_LOCATION_LENGTH - 2);
			}
			else
			{
				Q_strncpyz(&ctfLocationsColors[i][0], locStrCS, MAX_CTF_LOCATION_LENGTH);
			}
		}
		else
		{
			Q_strncpyz(&ctfLocationsColors[i][0], locStrCS, MAX_CTF_LOCATION_LENGTH);
		}
	}
}

const char* CG_GetCTFLocation(int loc)
{
	return &ctfLocationsColors[loc % MAX_LOCATIONS][0];
}


