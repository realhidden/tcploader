#include "stdafx.h"
#include "SendElf.h"

void cliMessage(LPTSTR);

HANDLE consoleBuffer;

void do_cli(LPTSTR commandline) {

	//AllocConsole();
	AttachConsole(ATTACH_PARENT_PROCESS);
	consoleBuffer=CreateFile(
		TEXT("CONOUT$"),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	//SetConsoleActiveScreenBuffer(consoleBuffer);

	const size_t length=_tcslen(commandline);
	enum {IP,PORT,FILE} mode=IP;
	unsigned int bufferindex=0;
	TCHAR filename [MAX_PATH+1];
	char ipString [35];
	char portString [20];

	memset(filename,0,sizeof(filename));
	memset(ipString,0,sizeof(ipString));
	memset(portString,0,sizeof(portString));

	for(unsigned int i=0;i<=length;i++) {
		const TCHAR L=commandline[i];
		if(L==' ' && mode!=FILE) {
			if(mode==IP) {
				mode=PORT;
			} else if(mode==PORT) {
				mode=FILE;
			}
			bufferindex=0;
		} else {
			switch(mode) {
				case IP:
					ipString[bufferindex++]=(char)L;
					if(bufferindex>=35) {
						goto clibuffererror;
					}
				break;

				case FILE:
					filename[bufferindex++]=L;
					if(bufferindex>=MAX_PATH+1) {
						goto clibuffererror;
					}
				break;

				case PORT:
					portString[bufferindex++]=(char)L;
					if(bufferindex>=20) {
						goto clibuffererror;
					}
				break;
			} //end of switch
		}//end of if
	}//end of for

	unsigned long ipInt=inet_addr(ipString);
	unsigned int t =atoi(portString);
	unsigned short portInt=(unsigned short)t;

	sendFile(ipInt,portInt,filename,cliMessage);

	goto endofcli;

clibuffererror:
	cliMessage(TEXT("Ops, a part of the command line is too long"));
endofcli:
	CloseHandle(consoleBuffer);
}

void cliMessage(LPTSTR message) {
	WriteConsole(
	  consoleBuffer,
	  message,
	  (DWORD)_tcslen(message),
	  NULL,
	  0
	);

}
