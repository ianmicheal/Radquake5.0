/*
Copyright (C) 2002-2003 BlackAura (obsidianglow@HotPOP.com)
Copyright (C) 1996-2003 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"

// ===========================================
// Quake Dreamcast Modmenu
// By BlackAura
// ===========================================

#include <kos.h>

int		modmenu_argc = 0;
char	**modmenu_argv = NULL;

typedef struct modentry_s
{
	char	fullpath[256];
	char	game_name[256];
	char	game_dir[256];
	char	game_args[256];
	char	game_text[1024];
} modentry_t;

#define MAX_MODLIST		100
#define MODLIST_SIZE	(sizeof(modentry_t) * MAX_MODLIST)
static modentry_t		*modlist = NULL;
static int				modlist_c = 0;

static void ModList_Clear()
{
	memset(modlist, 0, MODLIST_SIZE);
	modlist_c = 0;
}

static void ModList_Free()
{
	ModList_Clear();
	free(modlist);
}

static void ModList_Alloc()
{
	modlist = (modentry_t *)malloc(MODLIST_SIZE);
	ModList_Clear();
}

static void ModList_ReScan(const char *base)
{
	// Vars for directory scanning
	int			dir;
	dirent_t	*ent;

	// Flush the modlist
	if(modlist)
		ModList_Clear();
	else
		ModList_Alloc();

	// Add the id1 directory
	strcpy(modlist[modlist_c].game_dir, "id1");
	strncpy(modlist[modlist_c].fullpath, base, 256);
	strcat(modlist[modlist_c].fullpath, "/");
	strcat(modlist[modlist_c].fullpath, "id1");
	strncpy(modlist[modlist_c].game_name, "Normal Quake", 256);
	strncpy(modlist[modlist_c].game_text, "Unmodified Quake by id Software", 256);
	modlist_c++;

	// TODO - Add the rouge and hipnotic directories

	// Open directory for listing
	dir = fs_open(base, O_RDONLY | O_DIR);
	if(dir == 0)
		return;

	// Scan for directories
	while((ent = fs_readdir(dir)) != NULL)
	{
		// Skip files, parent directories
		if( (!(ent->size < 0)) || (ent->name[0] == '.') )
			continue;

		// Skip id1 directory
		if(!stricmp(ent->name, "id1"))
			continue;

		// Copy directory into modlist
		strncpy(modlist[modlist_c].game_dir, ent->name, 256);

		// Work out full pathname
		strncpy(modlist[modlist_c].fullpath, base, 256);
		strcat(modlist[modlist_c].fullpath, "/");
		strcat(modlist[modlist_c].fullpath, ent->name);

		// Fill out game name and description
		strncpy(modlist[modlist_c].game_name, ent->name, 256);
		strncpy(modlist[modlist_c].game_text, ent->name, 256);

		// Fill out argument list
		strncpy(modlist[modlist_c].game_args, "-game ", 256);
		strcat(modlist[modlist_c].game_args, ent->name);

		// Add to count
		modlist_c++;
	}
	fs_close(dir);
}

// Set up arguments from a mod
// Adapted from nxDoom
static void ModMenu_Select(int i)
{
	char commandLine[1024];
	int		n, c = 0, slcl;

	// Rebuild the full command line
	if(strlen(modlist[i].game_args) == 0)
		sprintf(commandLine, "quake");
	else
		sprintf(commandLine, "quake %s", modlist[i].game_args);

	// Count the arguments
	modmenu_argc = 1;
	for(n = 0; n < strlen(commandLine); n++)
	{
		if (commandLine[n] == ' ')
			modmenu_argc++;
	}

	// Set up modmenu_argv
	modmenu_argv = (char **)malloc(sizeof(char **) * modmenu_argc);
	modmenu_argv[0] = commandLine;
	c = 1;
	slcl = strlen(commandLine);
	for(n = 0; n < slcl; n++)
	{
		if(commandLine[n] == ' ')
		{ 
			commandLine[n] = 0;
			modmenu_argv[c++] = commandLine + n + 1;
		}
	}
}

static int		md_top = 0;
static int		md_bottom = 16;
static int		md_select = 0;
static int		md_last = -1;
static short	*bbuffer;

static void ModMenu_DrawText(int x, int y, char *string)
{
	bfont_draw_str(bbuffer + ((y + 1) * 24 * 640) + (x * 12), 640, 0, string);
}

static void MMD_CLS()
{
	int x, y, ofs;
	for(y = 0; y < 480; y++)
	{
		ofs = (640 * y);
		for(x = 0; x < 640; x++)
			bbuffer[ofs + x] = 0;
	}
}

static void ModMenu_Draw()
{
	int i;

	if(md_last == md_select)
		return;

	// Clear the buffer
	MMD_CLS();

	// Draw the list using the BIOS font
	ModMenu_DrawText(0, 0, "--------------RADQUAKE5.0---------------");
	for(i = md_top; i < md_bottom; i++)
	{
		if(i < modlist_c)
		{
			char tstring[256];
			sprintf(tstring, "%i. %s", i, modlist[i].game_name);
			ModMenu_DrawText(2, (i - md_top) + 1, tstring);
		}
	}
	ModMenu_DrawText(0, (md_select - md_top) + 1, ">");

	// Blit the buffer
	memcpy(vram_s, bbuffer, 640 * 480 * 2);

	// Store last selected, avoid continuous redraws
	md_last = md_select;
}

static void ModMenu_ScrollDown()
{
	if(!(md_bottom < modlist_c))
		return;
	md_top++;
	md_bottom++;
	md_last = -1;
}

static void ModMenu_ScrollUp()
{
	if(!(md_top > 0))
		return;
	md_top--;
	md_bottom--;
	md_last = -1;
}

void ModMenu(const char *basedir)
{
	int	finished = 0;

	// Controller garbage
	maple_device_t	*dev;
	cont_state_t	*state;

	// Scan the modlist
	ModList_ReScan(basedir);

	// Exit if only id1
	if(modlist_c <= 1)
	{
		ModMenu_Select(0);
		ModList_Free();
		return;
	}

	// Init KOS stuff
	vid_set_mode(DM_640x480, PM_RGB565);
	bbuffer = (unsigned short *)malloc(640 * 480 * 2);
	
	while(!finished)
	{
		// Handle the controller
		dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
		if(dev)
		{
			state = (cont_state_t *)maple_dev_status(dev);
			if(state)
			{
				if(state->buttons & CONT_DPAD_DOWN)
				{
					if(md_select < (modlist_c - 1))
						md_select++;
					while(md_select < md_top)
						ModMenu_ScrollUp();
					while(state->buttons & CONT_DPAD_DOWN)
						state = (cont_state_t *)maple_dev_status(dev);
				}
				else if(state->buttons & CONT_DPAD_UP)
				{
					if(md_select > 0)
						md_select--;
					while(md_select > md_bottom)
						ModMenu_ScrollDown();
					while(state->buttons & CONT_DPAD_UP)
						state = (cont_state_t *)maple_dev_status(dev);
				}
				else if(state->buttons & CONT_A)
				{
					ModMenu_Select(md_select);
					finished = 1;
				}
			}
		}

		// Redraw the modlist
		ModMenu_Draw();
	}

	ModList_Free();
	free(bbuffer);
}
