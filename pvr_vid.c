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
// pvr_vid.c -- converted to 2d PVR rendering by speud
// TRILINEAR and debug and Optmize Ian micheal 2019

#include "quakedef.h"
#include "d_local.h"

#include <SDL/SDL.h>
#include <kos.h>
#include <dc/pvr.h>
#include <png/png.h>

#include "flashrom.c"

#define	BASEWIDTH	320
#define	BASEHEIGHT	200
#define SCREENHEIGHT	240

#define	U1	0.3 * (1.0f / 512.0f)
#define	V1	0.3 * (1.0f / 256.0f)
#define	U2	(BASEWIDTH - 0.3) * (1.0f / 512.0f)
#define	V2	(BASEHEIGHT - 0.3) * (1.0f / 256.0f)


viddef_t	vid;				// global video state

pvr_poly_hdr_t	hdr[2];
pvr_ptr_t	vram_tex[2];
uint16		*vram_buf[2];
int		txr_cur;

pvr_ptr_t	str_tex; 
unsigned short	*str_buf;

pvr_poly_hdr_t	hdr_rect;

pvr_ptr_t	bg_tex; 

byte	vid_buffer[BASEWIDTH*BASEHEIGHT];
short	zbuffer[BASEWIDTH*BASEHEIGHT];

byte	surfcache[SURFCACHE_SIZE_AT_320X200];

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];


#define PACK_RGB565(r,g,b) (((r>>3)<<11)|((g>>2)<<5)|(b>>3))
unsigned short paletteData[256];


cvar_t	vid_screen_x = {"vid_screen_x", "0", true};
cvar_t	vid_screen_y = {"vid_screen_y", "0", true};
cvar_t	vid_screen_width = {"vid_screen_width", "640", true};
cvar_t	vid_screen_height = {"vid_screen_height", "480", true};
cvar_t	vid_disp_mode = {"vid_disp_mode", "0", true};


int dm;
unsigned short hline[BASEWIDTH];
void dc_txr_dma_cpy(void *dest, void *src, int n)
{
	long i = n/32;
	char	*ptr=dest;


	do 
	{
		asm("ocbp @%0" : : "r" (ptr));
		ptr += 32;
	} while (-- i);

		pvr_txr_load_dma(dest, src, n,1);

}

void VID_PvrInit (void) {
	pvr_init_defaults();
	pvr_dma_init();
}


void VID_DrawScreen (void) {
	pvr_vertex_t	vert;

	pvr_prim(&hdr[txr_cur], sizeof(*hdr));

	vert.flags = PVR_CMD_VERTEX; 
	vert.x = (float)vid_screen_x.value;
	vert.y = (float)vid_screen_y.value;
	vert.z = 1;
	vert.u = U1;
	vert.v = V1;
	vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);    
	vert.oargb = 0; 
	pvr_prim(&vert, sizeof(vert));

	vert.x = (float)vid_screen_x.value + (float)vid_screen_width.value;
	vert.u = U2;
	pvr_prim(&vert, sizeof(vert)); 

	vert.x = (float)vid_screen_x.value;
	vert.y = (float)vid_screen_y.value + (float)vid_screen_height.value;
	vert.u = U1;
	vert.v = V2;
	pvr_prim(&vert, sizeof(vert));

	vert.x = (float)vid_screen_x.value + (float)vid_screen_width.value;
	vert.u = U2;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));
	
     dcache_flush_range((int)vram_buf, 512*256*2);
	dc_txr_dma_cpy(vram_buf[txr_cur], vram_tex[txr_cur], 512*256*2);
}


void VID_DrawBackground (void) {
	pvr_poly_cxt_t cxt;
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert;

	pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, 256, 256, bg_tex, PVR_FILTER_TRILINEAR2);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
	vert.oargb = 0;
	vert.flags = PVR_CMD_VERTEX;

	vert.x = 1;
	vert.y = 1;
	vert.z = 1;
	vert.u = 0;
	vert.v = 0;
	pvr_prim(&vert, sizeof(vert));

	vert.x = 640;
	vert.u = 1.0;
	pvr_prim(&vert, sizeof(vert));

	vert.x = 1;
	vert.y = 480;
	vert.u = 0;
	vert.v = 1.0;
	pvr_prim(&vert, sizeof(vert));

	vert.x = 640;
	vert.u = 1.0;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));
}


void VID_DrawStr (void) {
	pvr_poly_hdr_t hdr;
	pvr_vertex_t vert;
	pvr_poly_cxt_t cxt;

	pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB4444, 256, 256, str_tex, PVR_FILTER_TRILINEAR2);
	pvr_poly_compile(&hdr, &cxt);

	pvr_prim(&hdr, sizeof(hdr));

	vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
	vert.oargb = 0;
	vert.flags = PVR_CMD_VERTEX;

	vert.x = 192;
	vert.y = 112;
	vert.z = 1;
	vert.u = 0;
	vert.v = 0;
	pvr_prim(&vert, sizeof(vert));

	vert.x = 192+256;
	vert.u = 1;
	pvr_prim(&vert, sizeof(vert));

	vert.x = 192;
	vert.y = 112+256;
	vert.u = 0;
	vert.v = 1;
	pvr_prim(&vert, sizeof(vert));

	vert.x = 192+256;
	vert.u = 1;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));
}


void VID_DrawRect(float x1, float y1, float x2, float y2, float z, float a, float r, float g, float b)
{
	pvr_vertex_t vert;
	pvr_prim(&hdr_rect, sizeof(hdr_rect));

	vert.argb = PVR_PACK_COLOR(a, r, g, b);
	vert.oargb = 0;
	vert.flags = PVR_CMD_VERTEX;

	vert.x = x1;
	vert.y = y1;
	vert.z = z;
	pvr_prim(&vert, sizeof(vert));

	vert.x = x2;
	pvr_prim(&vert, sizeof(vert));

	vert.x = x1;
	vert.y = y2;
	pvr_prim(&vert, sizeof(vert));

	vert.x = x2;
	vert.flags = PVR_CMD_VERTEX_EOL;
	pvr_prim(&vert, sizeof(vert));
}


void dmSelector (void) {
   //check for holding start button
cont_cond_t regionswitch1; 
cont_get_cond(maple_first_controller(), &regionswitch1);
if(!(regionswitch1.buttons & CONT_START)){
   /* Set System Region */ 
    cont_cond_t regionswitch; 
   int regionloopend=1; 
   
   
  // vid_clear(0,255,0);
   bfont_draw_str(vram_s+(10)*640+(10), 640,480, "Region Select"); 
   bfont_draw_str(vram_s+(64)*640+(10), 640,480, "Y for 60hz");   
   bfont_draw_str(vram_s+(88)*640+(10), 640,480, "B for 50hz");

   bfont_draw_str(vram_s+(130)*640+(10), 640,480, "Current Region:"); 
   bfont_draw_str(vram_s+(130)*640+(190), 640,480, "USA"); 
   bfont_draw_str(vram_s+(200)*640+(10),640,480,"Hit A to continue..."); 

   while(regionloopend){ 
     
       
  cont_get_cond(maple_first_controller(), &regionswitch); 
      if(!(regionswitch.buttons & CONT_X))   { 
	  }
	  if(!(regionswitch.buttons & CONT_Y))   { 
		  vid_set_mode(DM_640x480, PM_RGB565);
		  bfont_draw_str(vram_s+(130)*640+(190), 640,480, "Hit A to continue...    ");
		  bfont_draw_str(vram_s+(130)*640+(190), 640,480, "60hz.."); 
	  }
	  if(!(regionswitch.buttons & CONT_B))   { 
		  vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);
		  bfont_draw_str(vram_s+(130)*640+(190), 640,480, "Hit A to continue...     ");
		  bfont_draw_str(vram_s+(130)*640+(190), 640,480, "50hz.."); 
	  }
       
if(!(regionswitch.buttons & CONT_A))   { 
   regionloopend=0; 
} 
   } 
//vid_empty();  
}
 
	}


// freeing strings
//	pvr_mem_free(str_tex);
//	free(str_buf);

// display selector end

//	printf("dmSelector: done\n");



void waitForCD (void) {
	int	checked=0, delay=0;
	file_t	f;
	cont_cond_t cont;

	while (1) {
		pvr_wait_ready();
		pvr_scene_begin();

		pvr_list_begin(PVR_LIST_OP_POLY);
// drawing background
		VID_DrawBackground();
		pvr_list_finish();

		pvr_list_begin(PVR_LIST_TR_POLY);
// drawing frame
		VID_DrawRect(100, 190, 540, 270, 1, 0.7, 0.2, 0, 0);
		pvr_list_finish();

		pvr_scene_finish();

		if (checked) {
			bfont_draw_str(vram_s + 220*640 + 122, 640, 0, "Can't find /cd/QUAKE/ID1/PAK0.PAK");
			delay ++;
			if (delay >= 100) {
				delay = 0;
				checked = 0;
			}
		} else {
			bfont_draw_str(vram_s + 220*640 + 122, 640, 0, " Insert Quake CD and press START ");
		}

		cont_get_cond(maple_first_controller(), &cont);

		if(!(cont.buttons & CONT_START)) {
			iso_ioctl(0,NULL,0); 

			delay = 0;
			checked = 1;

			if ( (f = fs_open("/cd/QUAKE/ID1/PAK0.PAK", O_RDONLY)) )
				break;
		}
	}

	fs_close(f);
	bfont_draw_str(vram_s + 220*640 + 122, 640, 0, "            Loading...           ");

// freeing background
	pvr_mem_free(bg_tex);

	printf("waitForCD: done\n");
}

void timer_us_gettime64(void)
{
}




void VID_ClearVRam ()
{
	int y;

	// Clear VRAM
	 *(volatile uint32*)0xa05F6884 = 0x1;
	for(y=0; y<512; y++)
		sq_set16(vram_buf[txr_cur]+y*512, 0, 256*2);
		*(volatile uint32*)0xa05F6884 = 0x0;
		
		
}


void	VID_SetPalette (unsigned char *palette)
{
	int r, g, b, i, c=0;

	for(i=0; i<256; i++)
	{
		r = palette[c++];
		g = palette[c++];
		b = palette[c++];
		paletteData[i] = PACK_RGB565(r, g, b);
	}	
}


void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette (palette);
}

void fastmemset(void *dest,int value, int bytes)
{
	uint32 *d= (uint32*) dest;
	
    bytes = bytes / 4;
    while (bytes-- > 0)
    {   *d++ = value;}
}
void	VID_Init (unsigned char *palette)
{
   //  CDAudio_Init();
	pvr_poly_cxt_t cxt;
	int i;

/*
	extern	cvar_t	vid_screen_x;
	extern	cvar_t	vid_screen_y;
	extern	cvar_t	vid_screen_width;
	extern	cvar_t	vid_screen_height;
	extern	cvar_t	vid_disp_mode;
*/

	Cvar_RegisterVariable (&vid_screen_x);
	Cvar_RegisterVariable (&vid_screen_y);
	Cvar_RegisterVariable (&vid_screen_width);
	Cvar_RegisterVariable (&vid_screen_height);
	Cvar_RegisterVariable (&vid_disp_mode);

/* initing screen */
	for (i=0; i<2; i++) {
		vram_tex[i] = pvr_mem_malloc(512*256*2);
		fastmemset(vram_tex[i], 0, 512*256*2);
		vram_buf[i] = memalign(32, 512*256*2);
		if (vram_buf[i] == NULL) {
			printf("Not enough memory.\n");
			arch_exit();
		}

		pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565|PVR_TXRFMT_NONTWIDDLED, 512, 256, vram_tex[i], PVR_FILTER_TRILINEAR2);
		pvr_poly_compile(&hdr[i], &cxt); 
	}
	txr_cur = 0;


	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.buffer = vid.conbuffer = vid_buffer;
	vid.rowbytes = vid.conrowbytes = BASEWIDTH;
	
	d_pzbuffer = zbuffer;
	D_InitCaches (surfcache, sizeof(surfcache));

	VID_ClearVRam();

/* initing palette */
	VID_SetPalette(palette);

/* initing display mode */
	vid_disp_mode.value = dm;
	Cvar_SetValue ("vid_disp_mode", vid_disp_mode.value);
}


void	VID_Shutdown (void)
{
	int i;

	for (i=0; i<2; i++)
		pvr_mem_free(vram_tex[i]);
	pvr_shutdown();
}


void	VID_Update (vrect_t *rects)
{
	int x, y, s=0;

	for(y=0; y<BASEHEIGHT; y++) {
		for(x=0; x<BASEWIDTH; x++)
			hline[x] = paletteData[vid_buffer[s++]];

		sq_cpy(vram_buf[txr_cur]+y*512, hline, BASEWIDTH*2);
	}

	pvr_wait_ready();
	pvr_scene_begin();
	pvr_list_begin(PVR_LIST_OP_POLY);
	VID_DrawScreen();
	pvr_list_finish();
	pvr_scene_finish();

	vid_border_color(0, 0, 0);
	txr_cur ^= 1;
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
