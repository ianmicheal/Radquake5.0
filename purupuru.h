#include <kos.h>

#ifndef __DC_MAPLE_PURUPURU_H
#define __DC_MAPLE_PURUPURU_H
#endif

#ifndef MAPLE_FUNC_PURUPURU
#define MAPLE_FUNC_PURUPURU		0x00010000
#endif

/*
int	purupuru_port;
int	purupuru_unit;
int	purupuru_enabled;
float	purupuru_freq;
float	purupuru_pow;
*/

void	purupuru_rumble_callback(maple_frame_t * frame);
int	purupuru_rumble(maple_device_t * dev, uint8 freq, uint8 pow, uint8 len);

void 	purupuru_periodic(maple_driver_t *drv);
int 	purupuru_attach(maple_driver_t *drv, maple_device_t *dev);
void 	purupuru_detach(maple_driver_t *drv, maple_device_t *dev);
int	purupuru_init();
void	purupuru_shutdown();

