/*
Copyright (C) 1996-1997 Id Software, Inc.

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
// sdl_vid.c -- Very simple SDL-based video system 
// 

#include "quakedef.h"
#include "d_local.h"

#include <SDL/SDL.h>
#include <kos.h>

viddef_t	vid;				// global video state

#define	BASEWIDTH	320
#define	BASEHEIGHT	240

//Ian micheal this might help
//#define    BASEWIDTH    (320*1)
//#define    BASEHEIGHT   (200*1)
///Ian  micheal changed surface cashe
short	zbuffer[BASEWIDTH*BASEHEIGHT];
byte	surfcache[BASEWIDTH*BASEHEIGHT*10];

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];

SDL_Surface *sdlscreen;



void	VID_SetPalette (unsigned char *palette)
{
	SDL_Color colors[256];
	int i;
	for(i = 0; i < 256; i++)
	{
		colors[i].r = palette[i*3+0];
		colors[i].g = palette[i*3+1];
		colors[i].b = palette[i*3+2];
	}
	SDL_SetPalette(sdlscreen, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette(palette);
}

void	VID_Init (unsigned char *palette)
{
	vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);

       // Load the SDL library
/*
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_CDROM) < 0)
        Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());
*/
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
        Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());
   
	// Initialise SDL Video
	sdlscreen = SDL_SetVideoMode(BASEWIDTH, BASEHEIGHT, 8, SDL_HWSURFACE);
    SDL_ShowCursor(SDL_DISABLE);

	VID_SetPalette(palette);

	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect =  1.0;
	vid.numpages = 2;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.buffer = vid.conbuffer = sdlscreen->pixels;
	vid.rowbytes = vid.conrowbytes = BASEWIDTH;
	
	d_pzbuffer = zbuffer;
	D_InitCaches (surfcache, sizeof(surfcache));
}

void	VID_Shutdown (void)
{
	SDL_FreeSurface(sdlscreen);
}

void	VID_Update (vrect_t *rects)
{
	SDL_Flip(sdlscreen);
	

}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
}


