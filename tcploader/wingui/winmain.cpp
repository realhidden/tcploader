#include "stdafx.h"

#include "SendElf.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;

WSADATA wsaData;


int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE,
	LPTSTR,
	int
) {

	hInst = hInstance;
	
	int success = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(success!=0) {
		MessageBox(NULL, TEXT("Winsock could not be initalized"), TEXT("Aw crap"), 0);
		return 1;
	}

	//MessageBox(NULL,commandline,0,MB_ICONINFORMATION);

	//if(*commandline==0) {
	DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), 0, MainDialog);
	//} else {
	//	do_cli(commandline);
	//}

	WSACleanup();

	return 0;
	//return (int) msg.wParam;
}

