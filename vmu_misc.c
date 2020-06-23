#include <kos.h>
#include <png/png.h>
#include "vmu_misc.h"

#include "vmu_extrabg.h"

#include "quakedef.h"

/************************************************\
 * speud (27-06-2004)                           *
 *                                              *
 * 	Misc. VMU functions                     *
 *                                              *
\************************************************/


char main_icon[32*32/8] = {
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x08, 0x20, 0x00,
	0x00, 0x06, 0xc0, 0x00,
	0x00, 0x03, 0x80, 0x01,
	0x80, 0x01, 0x00, 0x03,
	0xc0, 0x00, 0x00, 0x06,
	0xc0, 0x00, 0x00, 0x06,
	0xe0, 0x00, 0x00, 0x0e,
	0x60, 0x00, 0x00, 0x0c,
	0x60, 0x00, 0x00, 0x0c,
	0x60, 0x00, 0x00, 0x0c,
	0x60, 0x00, 0x00, 0x0c,
	0x60, 0x00, 0x00, 0x0c,
	0xe0, 0x00, 0x00, 0x0e,
	0xe0, 0x00, 0x00, 0x0e,
	0xe0, 0xc0, 0x07, 0x0f,
	0xe0, 0xc1, 0x07, 0x0f,
	0xc0, 0x83, 0x83, 0x07,
	0xc0, 0x8f, 0xe3, 0x07,
	0x80, 0xff, 0xff, 0x03,
	0x00, 0xff, 0xff, 0x01,
	0x00, 0xfe, 0xff, 0x00,
	0x00, 0xfc, 0x7f, 0x00,
	0x00, 0xf0, 0x1f, 0x00,
	0x00, 0x80, 0x03, 0x00,
	0x00, 0x80, 0x03, 0x00,
	0x00, 0x80, 0x03, 0x00,
	0x00, 0x80, 0x03, 0x00,
	0x00, 0x80, 0x03, 0x00,
	0x00, 0x80, 0x03, 0x00,
	0x00, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x01, 0x00
};


cvar_t	vmu_port = {"vmu_port", "0", true};
cvar_t	vmu_unit = {"vmu_unit", "1", true};
cvar_t	vmu_autosave = {"vmu_autosave", "0", true};
cvar_t	vmu_disp = {"vmu_disp", "1", true};



void VMU_init() {
	Cvar_RegisterVariable (&vmu_port);
	Cvar_RegisterVariable (&vmu_unit);
	Cvar_RegisterVariable (&vmu_autosave);
	Cvar_RegisterVariable (&vmu_disp);

	vmu_dev = maple_enum_dev(0, 1);
	vmu_func = maple_device_func(0, 1);

	VMU_initLCDicons();
}


#define VMU_BAD_ROOT -1
#define VMU_BAD_FAT -2
#define VMU_BAD_DEV -3

int VMU_GetFreeblocks() {
	int	i;
	uint8	buf[512];

	freeblocks = 0;
	totalblocks = 0;

	if (vm_dev == NULL)
		return VMU_BAD_DEV;

	if (vmu_block_read(vm_dev, 255, buf))
		return VMU_BAD_ROOT;

	totalblocks = buf[0x50];

	if (vmu_block_read(vm_dev, 254, buf))
		return VMU_BAD_FAT;

	for (i=totalblocks-1; i>=0; i--)
		if (buf[i*2]==0xfc && buf[i*2+1]==0xff)
			freeblocks ++;

	return freeblocks;
}


/* calcCRC() by Marcus Comstedt */
uint16 	VMU_calcCRC(char *file_buf, int data_len) {
	uint16	crc=0;
	int	i, j;

	for (i=0; i<data_len; i++) {
		crc ^= (file_buf[i] << 8);

		for (j=0; j<8; j++) {
			if (crc & 0x8000)
				crc = (crc << 1) ^ 4129;
			else
				crc = (crc << 1);
		}
	}

	return crc;
}


void VMU_initLCDicons() {
	kos_img_t *img;
	int	h, w, y, x;
	uint16	buf[48*3 * 32*5];
	file_t	f;


	f = fs_open("/cd/lcdicons.png", O_RDONLY);

	if (!f) {
		Con_Printf ("VMU_initLCDicons: can't find %s\n", "/cd/lcdicons.png");
		return;
	}

	img = (kos_img_t *)malloc(sizeof(kos_img_t));

	if(png_to_img("/cd/lcdicons.png", PNG_NO_ALPHA, img))
		Con_Printf ("VMU_initLCDicons: can't convert png image %s to a kos image.\n", "/cd/lcdicons.png");

	memcpy(buf, img->data, (48*3 * 32*5) * 2);

	for (h=0; h<5; h++)
		for (w=0; w<3; w++)
			for (y=0; y<32; y++)
				for (x=0; x<48; x++)
					lcd_icons[w][h][y*48+x] = (buf[(h*32+y)*48*3+w*48+x] != 0xffff);

	return;
}


void VMU_loadLCDicon(uint8 *src_icon, uint8 *dst_icon) {
	int x, y, i;

	for (y=0; y<32; y++)
		for (x=0; x<6; x++)
			for (i=0; i<8; i++)
				dst_icon[(31-y)*6+(5-x)] |= (src_icon[y*48 + x*8 + i] << i);

}


void VMU_displayLCDicon(uint8 *lcd_icon) {
	vm_dev = maple_enum_dev((int)vmu_port.value, (int)vmu_unit.value);
	uint32 func = maple_device_func((int)vmu_port.value, (int)vmu_unit.value);

	if (vm_dev==NULL || !(func & (MAPLE_FUNC_MEMCARD | MAPLE_FUNC_LCD)))
		return;

	if (vmu_disp.value)
		vmu_draw_lcd(vm_dev, lcd_icon);
}


void VMU_saveExtraIcon() {
	int	x, y, byte, revbyte, i, mask, bit;
	uint8	realmenu_hack[16] = {
		0xda, 0x69, 0xd0, 0xda, 0xc7, 0x4e, 0xf8, 0x36,
		0x18, 0x92, 0x79, 0x68, 0x2d, 0xb5, 0x30, 0x86
	};
	uint8	file_buf[2*512];
	char	file_path[32];
	file_t	f;


// Filling header
	memset(file_buf, 0, 2*512);
	memcpy(file_buf,	"RAD Quake icon  ",	16);	// description
	file_buf[16] = 0x20;					// monochrome icon's index
	file_buf[20] = 0xa0;					// color icon's index


// Filling monochrome icon
	for (y=0; y<32; y++) {
		for (x=0; x<4; x++) {
			byte = main_icon[y*4 + x];
			revbyte = 0;

			for (i=0; i<8; i++) {
				mask = 0x80 >> (7 - i);
				bit = (byte & mask) >> i;
				revbyte |= bit << (7 - i);
			}

			file_buf[0x20 + y*4 + x] = revbyte;
		}
	}


// Filling color icon
	memcpy(file_buf + 0xa0,		icon_palette,	32);
	memcpy(file_buf + 0xa0 + 32,	icon_bitmap,	512);


// "Real menu" hack
	memcpy(file_buf + 0x20 + 672,	realmenu_hack,	16);


// Saving file
	sprintf (file_path, "/vmu/%c%d/ICONDATA_VMS", (int)vmu_port.value+'a', (int)vmu_unit.value);

	Con_Printf ("Counting free blocks for VMU in %c-%d...\n", (int)vmu_port.value+'A', (int)vmu_unit.value);

	vm_dev = maple_enum_dev((int)vmu_port.value, (int)vmu_unit.value);
	freeblocks = VMU_GetFreeblocks();

	if (freeblocks == -1) {
		Con_Printf ("VMU_saveExtraIcon: couldn't read root block.\n");
		return;
	} else if (freeblocks == -2) {
		Con_Printf ("VMU_saveExtraIcon: couldn't read FAT.\n");
		return;
	}

	f = fs_open(file_path, O_RDONLY);
	if (f) {
		freeblocks += fs_total(f) / 512;
		fs_close(f);
	}

	if (freeblocks < 2) {
		Con_Printf ("Not enough free blocks. Free:%d Needed:%d.\n", freeblocks, sizeof(file_buf)/512);
		return;
	}

	Con_Printf ("Saving extra icon to %s...\n", file_path);

	fs_unlink(file_path);

	f = fs_open(file_path, O_WRONLY);
	if (!f)
	{
		Con_Printf ("VMU_saveExtraIcon: couldn't open.\n");
		return;
	}

	fs_write(f, file_buf, 2*512);
	fs_close(f);

	Con_Printf ("done.\n");
}


void VMU_saveExtraBg() {
	char	name[32], *ptr, *file_buf;
	uint8	buffer[8];
	uint16	crc;
	int	filesize, total;
	file_t	f;


	filesize = sizeof(extrabg);

	total = filesize + 640;
	while ((total % 512) != 0)
		total ++;

	file_buf = (char *)malloc(total+1);
	if (file_buf == NULL) {
		Con_Printf ("VMU_saveExtraBg: not enough memory.\n");
		return;
	}

	memset(file_buf, 0, total);

	memcpy(file_buf+640, extrabg, sizeof(extrabg));


// Filling VMS header
	ptr = file_buf;

	memcpy(ptr, "\x12RADQUAKE       ",16);	ptr += 16;	// VM desc
	memcpy(ptr, "\x12RADQUAKE       ",16);	ptr += 16;	// DC desc 1
	memcpy(ptr, "EXTRA BG        ",	16);	ptr += 16;	// DC desc 2
	memcpy(ptr, "RADQuake        ",	16);	ptr += 16;	// Application
	memcpy(ptr, "\x01\0",		2);	ptr += 2;	// Icons number
	memcpy(ptr, "\0\0",		2);	ptr += 2;	// Anim speed
	memset(ptr, 0,			2);	ptr += 2;	// Eyecatch type
	memset(ptr, 0,			2);	ptr += 2;	// CRC checksum

	sprintf(buffer, "%c%c%c%c",
		(filesize & 0x000000ff)	>> 0,
		(filesize & 0x0000ff00)	>> 8,
		(filesize & 0x00ff0000)	>> 16,
		(filesize & 0xff000000)	>> 24);

	memcpy(ptr, buffer,		4);	ptr += 4;	// Filesize
	memset(ptr, 0,			20);	ptr += 20;	// Reserved
	memcpy(ptr, icon_palette, 	32);	ptr += 32;	// Icons palette
	memcpy(ptr, icon_bitmap, 	512);	ptr += 512;	// Icons bitmap


// Calculating CRC checksum
	crc = VMU_calcCRC(file_buf, filesize+640);

	file_buf[0x46] = (crc & 0x00ff)	>> 0;
	file_buf[0x47] = (crc & 0xff00)	>> 8;


// Saving extra BG
	sprintf (name, "/vmu/%c%d/EXTRA.BG.PVR", (int)vmu_port.value+'a', (int)vmu_unit.value);

	Con_Printf ("Counting free blocks for VMU in %c-%d...\n", (int)vmu_port.value+'A', (int)vmu_unit.value);

	vm_dev = maple_enum_dev((int)vmu_port.value, (int)vmu_unit.value);
	freeblocks = VMU_GetFreeblocks();

	if (freeblocks == -1) {
		Con_Printf ("VMU_saveExtraBg: couldn't read root block.\n");
		free(file_buf);
		return;
	} else if (freeblocks == -2) {
		Con_Printf ("VMU_saveExtraBg: couldn't read FAT.\n");
		free(file_buf);
		return;
	}

	f = fs_open(name, O_RDONLY);
	if (f) {
		freeblocks += fs_total(f) / 512;
		fs_close(f);
	}

	if ((freeblocks*512) < total) {
		Con_Printf ("Not enough free blocks. Free:%d Needed:%d.\n", freeblocks, total/512);
		free(file_buf);
		return;
	}

	Con_Printf ("Saving extra BG to %s...\n", name);

	fs_unlink(name);

	f = fs_open (name, O_WRONLY);
	if (!f)
	{
		Con_Printf ("VMU_saveExtraBg: couldn't open.\n");
		free(file_buf);
		return;
	}

	fs_write(f, file_buf, total);
	fs_close(f);

	Con_Printf ("done.\n");
	free(file_buf);
}
