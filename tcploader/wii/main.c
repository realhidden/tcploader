/*-------------------------------------------------------------

tcploader -- a very small tcp elf loader for the wii

Copyright (C) 2008 Sven Peter

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ogcsys.h>
#include <gccore.h>
#include <ogc/ipc.h>
#include <network.h>

#define READ_SIZE	(1 << 10)

#include "asm.h"
#include "processor.h"
#include "elf.h"
#include "dol.h"


static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

u8 *data = (u8 *)0x92000000;

static inline u32 wait_for(u32 btn)
{
	u32 btns;
	while(1)
	{
		PAD_ScanPads();
		btns = PAD_ButtonsHeld(0);
		if(btns & btn)
			return btns;
		VIDEO_WaitVSync();
	}
}


extern s32 __STM_Init();
extern void __exception_closeall();
extern s32 __IOS_ShutdownSubsystems();

/* code to establish a socket; originally from bzs@bu-cs.bu.edu
 */

int establish(unsigned short portnum) {
  int    s;
  struct sockaddr_in sa;

  memset(&sa, 0, sizeof(struct sockaddr_in)); /* clear our address */
  sa.sin_family= AF_INET;              /* this is our host address */
  sa.sin_port= htons(portnum);                /* this is our port number */
  sa.sin_addr.s_addr = net_gethostip();
  sa.sin_len = 8;
  if ((s= net_socket(AF_INET, SOCK_STREAM, 0)) < 0) /* create socket */
    return(-1);
  if (net_bind(s,(struct sockaddr *)&sa,8) < 0) {
    net_close(s);
    return(-1);                               /* bind address to socket */
  }
  net_listen(s, 3);                               /* max # of queued connects */
  return(s);
}

// wait for a connection to occur on a socket created with establish()

int get_connection(int s, struct sockaddr_in *sa)
{ 
  int t;                  /* socket of connection */
//  struct sockaddr_in sa;
  sa->sin_len = 8;
  sa->sin_family = AF_INET;
  u32 buflen = 8;
  t = net_accept(s,(struct sockaddr *)sa, &buflen);
  printf(" Incoming connection from %d.%d.%d.%d\n",
	 (sa->sin_addr.s_addr >> 24) & 0xFF,
	 (sa->sin_addr.s_addr >> 16) & 0xFF,
	 (sa->sin_addr.s_addr >> 8) & 0xFF,
	 (sa->sin_addr.s_addr) & 0xFF);

  return t;
}

int read_data(int s,     /* connected socket */
              char *buf, /* pointer to the buffer */
              int n      /* number of characters (bytes) we want */
	      )
{ int bcount; /* counts bytes read */
  int br;     /* bytes read this pass */

  bcount= 0;
  br= 0;
  while (bcount < n) {             /* loop until full buffer */
    if ((br= net_read(s,buf,n-bcount)) > 0) {
      bcount += br;                /* increment byte counter */
      buf += br;                   /* move buffer ptr for next read */
    }
    else if (br < 0)               /* signal an error to the caller */
      return br;
  }
  //  debug_printf("read_data(%d, %p, %d): \n", s, buf, n);
  // printf("read_data(%d, %p, %d): %d\n", s, buf, n, bcount);
  return bcount;
}

int write_data(int s, char *buf, int n)
{
  int bcount;
  int br;
  bcount = 0;
  br = 0;
  while(bcount < n) {
    if ((br = net_write(s,buf,n-bcount)) > 0) {
      bcount += br;
      buf += br;
    }
    else if (br < 0)
      return br;
  }
  //  debug_printf("write_data(%d, %p, %d): \n", s, buf, n);
  //  hexdump(buf, bcount);
  return (bcount);
}

int main(int argc, char **argv)
{
	unsigned long level;
	u32 res, offset, read, size, ip, btn;
	s32 client, listen;
	u8 *oct = (u8 *)&ip;
	u8 *bfr[READ_SIZE];
	struct sockaddr_in addr;
	void (*ep)();

	VIDEO_Init();
	PAD_Init();

	switch(VIDEO_GetCurrentTvMode())
	{
		case VI_PAL:
			rmode = &TVPal528IntDf;
			break;
		case VI_MPAL:
			rmode = &TVMpal480IntDf;
			break;
		case VI_EURGB60:
			rmode = &TVNtsc480Prog;
			break;
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
 
 	__STM_Init();

	printf("\n\n TCP ELF Loader\n");
	printf(" (c) 2008 Sven Peter\n");
	printf(" Based on bushing's title_listener.c and his networking code.\n");
	printf(" (c) 2008 bushing\n\n");

	redo:
	printf(" Press A to continue.\n");
	wait_for(PAD_BUTTON_A);

	printf(" net_init(), please wait...\n");
	net_init();


	printf(" Opening socket on port 8080\n");
	listen = establish(8080);

	if(listen < 0)
	{
		printf(" establish() returned < 0.\n");
		goto redo;
	}

	ip = net_gethostip();
	printf(" fd = %d\n", listen);
	printf(" You Wii's ip address is %d.%d.%d.%d\n", oct[0], oct[1], oct[2], oct[3]);

	while(1)
	{
		printf(" Waiting for connection\n");
		client = get_connection(listen, &addr);

		if(client > 0)
		{
			printf(" client connected..\n");
			printf(" Please check the IP address and press A to continue and Z to abort\n");
			btn = wait_for(PAD_BUTTON_A | PAD_TRIGGER_Z);
			if(btn & PAD_TRIGGER_Z)
			{
				net_close(client);
				continue;
			}

			if(((addr.sin_addr.s_addr >> 24) & 0xFF) != oct[0] || ((addr.sin_addr.s_addr >> 16) & 0xFF) != oct[1])
			{
				printf(" WARNING: the client is not connecting from your local network.\n");
				printf(" Please check if you really want to run a file from this IP as it\n");
				printf(" may be something that bricks your wii from someone you don't even know!!\n");
				printf(" Press Z if you really want to continue!\n");
				printf(" Press A to deny the connection and wait for a new client\n");
				btn = wait_for(PAD_TRIGGER_Z | PAD_BUTTON_A);
				if(btn & PAD_BUTTON_A)
				{
					net_close(client);
					continue;
				}
			}

			if(read_data(client, (char *)&size, 4) < 0)
			{
				printf(" read_data() error while reading filesize\n");
				net_close(client);
				net_close(listen);
				goto redo;
			}

			printf(" size: %d\n", size);
			printf(" reading data, please wait...\n");

			offset = 0;
			while(offset < size && (read = read_data(client, (char *)bfr, (size - offset) > READ_SIZE ? READ_SIZE : (size - offset))) > 0)
			{
				memcpy(data + offset, bfr, READ_SIZE);
				offset += read;
			}
			net_close(client);
			net_close(listen);

			res = valid_elf_image(data);
			if(res != 1)
			{
				printf(" Invalid ELF image.\n");
				printf(" assuming DOL image. your wii will crash if this is just some random file...\n");
				ep = (void(*)())load_dol_image(data, 1);
			}
			else
			{
				printf(" Loading ELF image...\n");
				ep = (void(*)())load_elf_image(data);
			}
			printf(" entry point: 0x%08X\n", (unsigned int)ep);
			printf(" shutting down...\n");
			__IOS_ShutdownSubsystems();
			_CPU_ISR_Disable(level);
			__exception_closeall();
			printf(" jumping to entry point now...\n\n\n\n");
			ep();
			_CPU_ISR_Restore(level);
			printf(" this should not happen :/\n\n");
			goto redo;
		}
		else
		{
			printf("net_accept() returned %d\n", client);
		}
	}

	// never reached but fixes a gcc warning
	return 0;
}
