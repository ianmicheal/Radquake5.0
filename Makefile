#Rad_quake by Ian micheal
#update by speud


TARGET = RADquake.elf
BIN = RADquake.bin
FIRSTREAD = RAdquake_scr.bin
KOS_LOCAL_CFLAGS =  -DSDL -DLSB_FIRST -DALIGN_LONG -DINLINE -DDC -DUCHAR_MAX=0xff -DUSHRT_MAX=0xffff -DULONG_MAX=0xffffffff

# Version with Null sound system
OBJS =  dc_menu.o pvr_vid.o  vmu_config.o vmu_state.o vmu_misc.o  purupuru.o  snd_sdl.o snd_mix.o snd_dma.o snd_mem.o  chase.o cl_demo.o cl_input.o cl_main.o cl_parse.o cl_tent.o cmd.o common.o console.o crc.o cvar.o cd_sdl.o dc_input.o  dc_sys.o draw.o d_edge.o d_fill.o d_init.o d_modech.o d_part.o d_polyse.o d_scan.o d_sky.o d_sprite.o d_surf.o d_vars.o d_zpoint.o host.o host_cmd.o keys.o mathlib.o menu.o model.o net_loop.o net_main.o net_none.o net_vcr.o nonintel.o pr_cmds.o pr_edict.o pr_exec.o r_aclip.o r_alias.o r_bsp.o r_draw.o r_edge.o r_efrag.o r_light.o r_main.o r_misc.o r_part.o r_sky.o r_sprite.o r_surf.o r_vars.o sbar.o screen.o sv_main.o sv_move.o sv_phys.o sv_user.o view.o wad.o world.o zone.o syscalls.o

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

clean:
	-rm -f $(TARGET) $(BIN) $(FIRSTREAD) $(OBJS) romdisk.*

rm-elf:
	-rm -f $(TARGET) romdisk.*

$(TARGET): $(OBJS) romdisk.o
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $(TARGET) $(KOS_START) \
		$(OBJS) romdisk.o $(OBJEXTRA) -L$(KOS_BASE)/lib  -lSDL-1.2.9  -lSDL_mixer -lpng -lz -lm  -lkallisti -llwip4 -lc -ldcplib -lk++  $(KOS_LIBS)

romdisk.img:
	$(KOS_GENROMFS) -f romdisk.img -d romdisk -v

romdisk.o: romdisk.img
	$(KOS_BASE)/utils/bin2o/bin2o romdisk.img romdisk romdisk.o

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist:
	rm -f $(OBJS) romdisk.o romdisk.img
	$(KOS_STRIP) $(TARGET)
	$(KOS_CC_BASE)/bin/sh-elf-objcopy -O binary $(TARGET) $(BIN)
	$(KOS_CC_BASE)/bin/sh-elf-scramble $(BIN) $(FIRSTREAD)
	cp $(FIRSTREAD) 1st_read.bin
