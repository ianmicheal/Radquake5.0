#include <kos.h>

int	freeblocks, totalblocks;
maple_device_t	*vm_dev;

maple_device_t	*vmu_dev;
uint32		vmu_func;

extern	uint8 icon_palette[32];
extern	uint8 icon_bitmap[512];

uint8 lcd_icons[3][5][48*32];

void	VMU_init();
int	VMU_GetFreeblocks();
uint16	VMU_calcCRC(char *file_buf, int data_len);
void	VMU_initLCDicons();
void	VMU_loadLCDicon(uint8 *src_icon, uint8 *dst_icon);
void	VMU_displayLCDicon(uint8 *lcd_icon);
void	VMU_saveExtraIcon();
void	VMU_saveExtraBg();
