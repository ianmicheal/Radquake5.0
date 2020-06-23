#include "purupuru.h"

#include "quakedef.h"

cvar_t	purupuru_port = {"purupuru_port", "0", true};
cvar_t	purupuru_unit = {"purupuru_unit", "2", true};
cvar_t	purupuru_enabled = {"purupuru_enabled", "1", true};
cvar_t	purupuru_freq = {"purupuru_freq", "0", true};
cvar_t	purupuru_pow = {"purupuru_pow", "0", true};


void purupuru_rumble_callback(maple_frame_t * frame) {
	/* Unlock the frame for the next usage */
	maple_frame_unlock(frame);
}

int purupuru_rumble(maple_device_t * dev, uint8 freq, uint8 pow, uint8 len) {
	uint32 *send_buf;

//	assert( dev != NULL );
        
	/* Lock the frame */
	if (maple_frame_lock(&dev->frame) < 0)
		return MAPLE_EAGAIN;

	maple_frame_init(&dev->frame);
	send_buf = (uint32 *)dev->frame.recv_buf;
	send_buf[0] = MAPLE_FUNC_PURUPURU;
	send_buf[1] = (0x11 << 24) | (freq << 16) | (pow << 8) | (len << 0);
	dev->frame.cmd = MAPLE_COMMAND_SETCOND;
	dev->frame.dst_port = dev->port;
	dev->frame.dst_unit = dev->unit;
	dev->frame.length = 2;
	dev->frame.callback = purupuru_rumble_callback;
	dev->frame.send_buf = send_buf;
	maple_queue_frame(&dev->frame);

	return MAPLE_EOK;
}

void purupuru_periodic(maple_driver_t *drv) {

}

int purupuru_attach(maple_driver_t *drv, maple_device_t *dev) {
	dev->status_valid = 1;
	return 0;
}

void purupuru_detach(maple_driver_t *drv, maple_device_t *dev) {
	dev->status_valid = 0;
}

/* Device Driver Struct */
static maple_driver_t purupuru_drv = {
	functions:	MAPLE_FUNC_PURUPURU,
	name:		"PuruPuru (Vibration) Pack",
	periodic:	NULL,
	attach:		purupuru_attach,
	detach:		purupuru_detach
};

/* Add the mouse to the driver chain */
int purupuru_init() {
	extern	cvar_t	purupuru_port;
	extern	cvar_t	purupuru_unit;
	extern	cvar_t	purupuru_enabled;
	extern	cvar_t	purupuru_freq;
	extern	cvar_t	purupuru_pow;

	Cvar_RegisterVariable (&purupuru_port);
	Cvar_RegisterVariable (&purupuru_unit);
	Cvar_RegisterVariable (&purupuru_enabled);
	Cvar_RegisterVariable (&purupuru_freq);
	Cvar_RegisterVariable (&purupuru_pow);

/*
	Cvar_SetValue ("purupuru_port", 0);
	Cvar_SetValue ("purupuru_unit", 2);
	Cvar_SetValue ("purupuru_enabled", 1);
	Cvar_SetValue ("purupuru_freq", 0);
	Cvar_SetValue ("purupuru_pow", 0);
*/

	return maple_driver_reg(&purupuru_drv);
}

void purupuru_shutdown() {
	maple_driver_unreg(&purupuru_drv);
}
