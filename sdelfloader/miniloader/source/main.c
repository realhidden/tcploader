#include <gccore.h>
#include <stdio.h>

#include "dol.h"
#include "processor.h"
#include "asm.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

extern u8 loader_dol[];

extern void __exception_closeall();
extern s32 __IOS_ShutdownSubsystems();

int main(int argc, char **argv) {
	u32 level;

	VIDEO_Init();
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

	PAD_Init();

	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();

	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	printf("\n\nloading\n");
	VIDEO_WaitVSync();
	void (*ep)() = (void(*)())load_dol_image(loader_dol);
	printf("jumping to 0x%08X\n", (unsigned int)ep);

	__IOS_ShutdownSubsystems();
	_CPU_ISR_Disable (level);
	__exception_closeall ();
	printf("__exception_closeall() done. Jumping to ep now...\n");
	ep();
	_CPU_ISR_Restore (level);

	return 0; // fixes gcc warning
}
