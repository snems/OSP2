#include "cg_local.h"
#include "../qcommon/qcommon.h"

const char* weaponNames[10] = { "None", "Gauntlet", "MachineGun", "Shotgun", "G.Launcher", "R.Launcher", "LightningGun", "Railgun", "Plasmagun", "BFG" };

// very unsafe
void CG_OSPNormalizeText(char* src, int size, char* dst)
{
	int i = 0;

	while (*src != 0)
	{
		if (*src == '^')
		{
			src += 2;
			i += 2;
		}
		if (*src == 0x0a)
		{
			++src;
			++i;
		}
		else
		{
			*(dst++) = *(src++);
			++i;
		}
		if (i >= size)
		{
			*(dst) = 0;
		}
	}
}

/*
 * returns string size without special symbols like ^x123456, ^N
 * BUG: possible string overflow
 */
#define CHAR_IS_COLOR_CODE(C)  ( ((C >= '0') && (C <='9')) || ((C >= 'a') && (C <='f')) || ((C >= 'A') && (C <='F')))
int CG_OSPGetNormalizedStringSize(const char* str)
{
	int i = 0;
	while (*str)
	{
		if (str && *str == '^' && *(str + 1) != 0 &&  *(str + 1) != '^')
		{
			if (*(str + 1) == 'x' || *(str + 1) == 'X')
			{
				/* Ok, we have found ^x , check that next 6 charactes is color code */

				if (CHAR_IS_COLOR_CODE(*(str + 2)) ||
				        CHAR_IS_COLOR_CODE(*(str + 3)) ||
				        CHAR_IS_COLOR_CODE(*(str + 4)) ||
				        CHAR_IS_COLOR_CODE(*(str + 5)) ||
				        CHAR_IS_COLOR_CODE(*(str + 6)) ||
				        CHAR_IS_COLOR_CODE(*(str + 7)))
				{
					/* found eight characters code like ^xAABBCC */
					str += 8;
					continue;
				}
			}
			/* found two characters code like ^B */
			str += 2;
		}
		else
		{
			/* no special code */
			++i;
			++str;
		}
	}
	return i;
}



void CG_OSPColorFromNumber(int number, float* vector)
{
	float red;
	float green;
	float blue;

	if (number >= 10 && number < 34)
	{
		float tmp = ((360.0 * (float)number) - 3600.0) / 24.0;
		const float sixty_degree_rad = 60.0 * M_PI / 180.0;
		if (tmp <= 120.0)
		{
			tmp = M_PI * tmp / 180.0;
			blue = 0.0;
			red = (cos(tmp) / (cos(sixty_degree_rad - tmp)) + 1.0) / 3.0;
			green = 1.0 - (red + blue);
		}
		else if (tmp <= 240.0)
		{
			tmp = M_PI * (tmp - 120.0) / 180.0;
			red = 0.0;
			green = (cos(tmp) / (cos(sixty_degree_rad - tmp)) + 1.0) / 3.0;
			blue = 1.0 - (red + green);
		}
		else
		{
			tmp = M_PI * (tmp - 240.0) / 180.0;
			green = 0.0;
			blue = ((cos(tmp)) / (cos(sixty_degree_rad - tmp))  + 1.0) / 3.0;
			red = 1.0 - (green + blue);
		}

		VectorSet(vector, red, green, blue);

		return;
	}

	switch (number)
	{
		case 0:
			VectorSet(vector, 1.0, 1.0, 1.0);
			break;
		case 1:
			VectorSet(vector, 1.0, 0.2, 0.2);
			break;
		case 2:
			VectorSet(vector, 0.2, 1.0, 0.2);
			break;
		case 3:
			VectorSet(vector, 1.0, 1.0, 0.1);
			break;
		case 4:
			VectorSet(vector, 0.2, 0.2, 1.0);
			break;
		case 5:
			VectorSet(vector, 1.0, 0.1, 1.0);
			break;
		case 6:
			VectorSet(vector, 0.1, 1.0, 1.0);
			break;
		case 7:
			VectorSet(vector, 1.0, 0.59, 0.12);
			break;
		case 8:
			VectorSet(vector, 0.65, 0.2, 0.65);
			break;
		case 9:
			VectorSet(vector, 0.6, 0.6, 1.0);
			break;
		default:
			VectorSet(vector, 1.0, 1.0, 1.0);
	}
}

void CG_OSPColorFromChar(char c, float* i)
{
	int number;

	if (c >= '0' && c <= '9')
	{
		number = c - '0';
	}
	else
	{
		number = tolower(c) - 'a' + 10;
	}

	CG_OSPColorFromNumber(number, i);

}

void CG_OSPColorsFromString(const char* s, float* a, float* b, float* c, float* d)
{
	int i = 0;
	char str[4] = { '2', '2', '2', '2'}; // default green

	while (s && *s && i < 4)
	{
		str[i++] = *s++;
	}

	if (a) CG_OSPColorFromChar(str[0], a);
	if (b) CG_OSPColorFromChar(str[1], b);
	if (c) CG_OSPColorFromChar(str[2], c);
	if (d) CG_OSPColorFromChar(str[3], d);
}

void CG_OSPNormalizeNameCopy(char* from, char* to, unsigned int size)
{
	int i = 0;
	while (*from != 0 && i < (size - 1))
	{
		if (from[0] == '^' &&
		        (from[1] == 'x' || from[1] == 'X') &&
		        from[2] && from[3] && from[4] && from[5] && from[6] && from[7])
		{
			from += 8;
		}
		else
		{
			*(to++) = *(from++);
			++i;
		}
	}
	*to = 0;
}

void CG_DynamicMemReport(void)
{
	zone_stats_t stats;
	memset(&stats, 0, sizeof(stats));
	Z_Stats(&stats);

	Com_Printf("zoneSegments   = %d \n", stats.zoneSegments);
	Com_Printf("zoneBlocks     = %d \n", stats.zoneBlocks);
	Com_Printf("zoneBytes      = %d \n", stats.zoneBytes);
	Com_Printf("freeBytes      = %d \n", stats.freeBytes);
	Com_Printf("freeBlocks     = %d \n", stats.freeBlocks);
	Com_Printf("freeSmallest   = %d \n", stats.freeSmallest);
	Com_Printf("freeLargest    = %d \n", stats.freeLargest);
}

