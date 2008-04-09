/*
 *  Copyright (C) 2008 svpe, #wiidev at efnet
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdio.h"
#include "diskio.h"


DSTATUS disk_initialize (BYTE drv)
{
	s32 r;
	if(drv != 0)
		return RES_PARERR;
	
	r = sd_init();

	if(r == 0)
		return RES_OK;
	else
		return RES_NOTRDY;
}

DSTATUS disk_status ( BYTE drv )
{
	return RES_OK;
}

DRESULT disk_read (
  BYTE drv,		/* Physical drive nmuber (0) */
  BYTE *buff,		/* Data buffer to store read data */
  DWORD sector,		/* Sector number (LBA) */
  BYTE count		/* Sector count (1..255) */
)
{
	s32 r = -1;
	u32 i;

	for(i = 0; i < count; i++)
	{
		r = sd_read(sector + i, buff + (0x200 * i));
		if(r < 0)
			return RES_NOTRDY;
	}

	return RES_OK;
}

DRESULT disk_ioctl (
  BYTE drv,		/* Physical drive nmuber */
  BYTE ctrl,		/* Control code */
  void *buff		/* Buffer to send/receive data block */
)
{
	return RES_OK;
}

u32 get_fattime( void )
{
	return 0;
}

