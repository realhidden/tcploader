#include <gccore.h>
#include <gctypes.h>
#include <string.h>
#include <stdio.h>
#include "dol.h"
#include "asm.h"
#include "processor.h"

extern const u8 loader_dol[];
extern const u32 loader_dol_size;

extern void __exception_closeall();
extern s32 __IOS_ShutdownSubsystems();

static u32 *xfb;
static GXRModeObj *rmode;

int main(int argc, char *argv[])
{
	void (*ep)();
	unsigned long level;

	// doesn't work for some odd reason when this tuff is not done :/
	VIDEO_Init();
	PAD_Init();

	switch(VIDEO_GetCurrentTvMode())
	{
		case VI_PAL:
			rmode = &TVPal528IntDf;
			break;
		case VI_MPAL:
			rmode = &TVMpal480IntDf;
		default:
		case VI_NTSC:
			rmode = &TVNtsc480IntDf;
			break;
	}

	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	VIDEO_Configure(rmode);
	VIDEO_SetBlack(FALSE);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	
	printf("loading, please wait...\n");
	memcpy((void *)0x90000020, loader_dol, loader_dol_size);
//	memcpy((void *)0x80001800, stub_bin, stub_bin_size);

	ep = (void(*)())load_dol_image((void *)loader_dol, 1);
	__IOS_ShutdownSubsystems();
	_CPU_ISR_Disable(level);
	__exception_closeall();
	ep();
	return 0;
}
