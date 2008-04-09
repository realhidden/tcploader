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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <ogc/ios.h>

#define FILES_PER_PAGE	24

#include "sdio.h"
#include "tff.h"
#include "elf.h"
#include "dol.h"

#include "asm.h"
#include "processor.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

extern void __exception_closeall();
extern s32 __IOS_ShutdownSubsystems();

#define USB_ALIGN	__attribute__((aligned(32)))
static const char *usb_path USB_ALIGN = "/dev/usb/oh1/57e/305";
char bluetoothResetData1[] USB_ALIGN = {
	/* bmRequestType */
	0x20
};

char bluetoothResetData2[] USB_ALIGN = {
	/* bmRequest */
	0x00
};

char bluetoothResetData3[] USB_ALIGN = {
	/* wValue */
	0x00, 0x00
};

char bluetoothResetData4[] USB_ALIGN = {
	/* wIndex */
	0x00, 0x00
};

char bluetoothResetData5[] USB_ALIGN = {
	/* wLength */
	0x03, 0x00
};

char bluetoothResetData6[] USB_ALIGN = {
	/* unknown; set to zero. */
	0x00
};

char bluetoothResetData7[] USB_ALIGN = {
	/* Mesage payload. */
	0x03, 0x0c, 0x00
};

/** Vectors of data transfered. */
ioctlv bluetoothReset[] USB_ALIGN = {
	{
		bluetoothResetData1,
		sizeof(bluetoothResetData1)
	},
	{
		bluetoothResetData2,
		sizeof(bluetoothResetData2)
	},
	{
		bluetoothResetData3,
		sizeof(bluetoothResetData3)
	},
	{
		bluetoothResetData4,
		sizeof(bluetoothResetData4)
	},
	{
		bluetoothResetData5,
		sizeof(bluetoothResetData5)
	},
	{
		bluetoothResetData6,
		sizeof(bluetoothResetData6)
	},
	{
		bluetoothResetData7,
		sizeof(bluetoothResetData7)
	}
};



int main(int argc, char **argv) {

	FATFS ffs;
	DIR elfdir;
	FILINFO finfo;

	char filename[4+13+1];

	char filelist[999][13]; // FIXME: use a dynamic filelist here
	u32 files;

	FIL fp;
	WORD bytes_read;
	u32 bytes_read_total;
	u8 *data = (u8 *)0x92000000;

	u32 offset;
	u32 selection;
	u32 i;
	u32 level;
	u32 buttons;

	s32 res;

	s32 usb_fd = -1;

	void (*ep)();

	
	VIDEO_Init();
	PAD_Init();
        AUDIO_Init (NULL);
        DSP_Init ();
	AUDIO_StopDMA();
	AUDIO_RegisterDMACallback(NULL);
	
	switch(VIDEO_GetCurrentTvMode()) {
		case VI_NTSC:
			rmode = &TVNtsc480IntDf;
			break;
		case VI_PAL:
			rmode = &TVPal528IntDf;
			break;
		case VI_MPAL:
			rmode = &TVMpal480IntDf;
			break;
		default:
			rmode = &TVNtsc480IntDf;
			break;
	}

	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();


	printf("\n\n");
	printf("Front SD ELF Loader\n");
	printf("(c) 2008 Sven Peter (svpe, #wiidev at blitzed/efnet)\n");
	printf("Internal SD stuff by bushing, marcan and maybe some more ppl i don't know about.\n");
	printf("TinyFAT FS by elm-chan.org\n");
	printf("Some ELF loader code by dhewg and tmbinc\n");

	redo_top:
	printf("Press A to continue\n");
	while(1)
	{
		VIDEO_WaitVSync();
		PAD_ScanPads();
		if(PAD_ButtonsDown(0) & PAD_BUTTON_A)
			break;
	}

	// shutdown wiimote
	printf("Shutting down wiimote\n");
//	printf("Open Bluetooth Dongle\n");
	usb_fd = IOS_Open(usb_path, 2 /* 2 = write, 1 = read */);
	printf("fd = %d\n", usb_fd);

//	printf("Closing connection to blutooth\n");
	res = IOS_Ioctlv(usb_fd, 0, 6, 1, bluetoothReset);
//	printf("ret = %d\n", res);

	IOS_Close(usb_fd);


	files = 0;
	if(f_mount(0, &ffs) != FR_OK)
	{
		printf("f_mount() failed. No FAT filesystem on the SD card?\n");
		goto redo_top;
	}

	res = f_opendir(&elfdir, "elf");
	if(res != FR_OK)
	{
		printf("f_opendir() failed with %d. Does elf/ exist?\n", res);
		goto redo_top;
	}

	printf("Loading filelist...\n");
	files = 0;
	memset(&finfo, 0, sizeof(finfo));
	f_readdir(&elfdir, &finfo);
	while(strlen(finfo.fname) != 0)
	{
		if(!(finfo.fattrib & AM_DIR))
		{
			printf("Adding %s\n", finfo.fname);
			memcpy(filelist[files], finfo.fname, 13);
			files++;
		}
		f_readdir(&elfdir, &finfo);
	}

	goto while_loop;

	redo:

	printf("Press A..\n");
	while(1)
	{
		VIDEO_WaitVSync();
		PAD_ScanPads();
		if(PAD_ButtonsDown(0) & PAD_BUTTON_A)
			break;
	}

	while_loop:
	offset = selection = 0;
	while(1)
	{
		printf("\x1b[2J\n");

		for(i = offset; i < (offset + FILES_PER_PAGE) && i < files; i++)
		{
			if(selection == i)
				printf(">>%s\n", filelist[i]);
			else
				printf("%s\n", filelist[i]);
		}
		printf("\n\n");
		
		while(1)
		{
			PAD_ScanPads();
			buttons = PAD_ButtonsDown(0);
			if(buttons & PAD_BUTTON_A)
			{
				snprintf(filename, 4+13, "elf/%s", filelist[selection]);
				if(f_stat(filename, &finfo) != FR_OK)
				{
					printf("error: f_stat() failed :/\n");
					goto redo;
				}
				if(f_open(&fp, filename, FA_READ) != FR_OK)
				{
					printf("error: f_open() failed\n");
					goto redo;
				}
				printf("Reading %u bytes\n", (unsigned int)finfo.fsize);
				bytes_read = bytes_read_total = 0;
				while(bytes_read_total < finfo.fsize)
				{
					if(f_read(&fp, data + bytes_read_total, 0x200, &bytes_read) != FR_OK)
					{
						printf("error: f_read failed.\n");
						goto redo;
					}
					if(bytes_read == 0)
						break;
					bytes_read_total += bytes_read;
//					printf("%d/%d bytes read (%d this time)\n", bytes_read_total, (unsigned int)finfo.fsize, bytes_read);
				}
				if(bytes_read_total < finfo.fsize)
				{
					printf("error: read %u of %u bytes.\n", bytes_read_total, (unsigned int)finfo.fsize);
					goto redo;
				}
				res = valid_elf_image(data);
				if(res == 1)
				{
					printf("ELF image found.\n");
					ep = (void(*)())load_elf_image(data);
				}
				else
				{
					printf("Assuming that this is a DOL file. Your Wii will crash if it isn't...\n");
					ep = (void(*)())load_dol_image(data);
				}
				f_close(&fp);
				sd_deinit();

				printf("entry point: 0x%X\n", (unsigned int)ep);
				VIDEO_WaitVSync();

				// code from geckoloader
	                	__IOS_ShutdownSubsystems ();
				printf("IOS_ShutdownSubsystems() done\n");
				_CPU_ISR_Disable (level);
				printf("_CPU_ISR_Disable() done\n");
				__exception_closeall ();
				printf("__exception_closeall() done. Jumping to ep now...\n");
				ep();
				_CPU_ISR_Restore (level);
	
				printf("binaries really shouldn't return here...\n");
				while(1)
					VIDEO_WaitVSync();
			}
			else if(buttons & PAD_BUTTON_DOWN)
			{
				if(selection+1 < files)
					selection++;
				if(selection >= offset + FILES_PER_PAGE)
					offset += FILES_PER_PAGE;
				break;
			}
			else if(buttons & PAD_BUTTON_UP)
			{
				if(selection > 0)
					selection--;
				if(selection < offset)
				{
					if(FILES_PER_PAGE > offset)
						offset = 0;
					else
						offset -= FILES_PER_PAGE;
				}
				break;
			}

			VIDEO_WaitVSync();
		}
	}
	printf("this should not happen\n");
	while(1)
		VIDEO_WaitVSync();
	return 0;
}
