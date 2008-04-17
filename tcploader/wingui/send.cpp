#include "stdafx.h"
#include "SendElf.h"

void sendFile(unsigned long ipInt, unsigned short portInt, LPTSTR filename, messageCallback message) {

	int success;

	HANDLE fileHandle = CreateFile(
		filename,
		FILE_READ_DATA | FILE_READ_ATTRIBUTES,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);

	if(fileHandle == INVALID_HANDLE_VALUE) {
		message(TEXT("CreateFile failed, are you sure that the file exists and is readable?"));
		goto createfileError;
	}

	int fileSize = GetFileSize(fileHandle,NULL);
	if(INVALID_FILE_SIZE == fileSize) {
		message(TEXT("GetFileSize failed, are you sure that the chosen file exists and is readable?"));
		goto getfilesizeError;
	}

	SOCKET socketHandle = socket(
		AF_INET,//ip4
		SOCK_STREAM,//tcp
		IPPROTO_TCP//tcp igen
	);

	if(socketHandle == INVALID_SOCKET) {
		message(TEXT("socket() failed, something seriously wrong is going on"));
		goto socketError;
	}

	struct sockaddr_in socketAddr;
	memset(&socketAddr,0,sizeof(socketAddr));

	socketAddr.sin_family=AF_INET;
	socketAddr.sin_addr.S_un.S_addr=ipInt;
	socketAddr.sin_port=htons(portInt);

	success = connect( socketHandle, (const sockaddr *)&socketAddr, sizeof(socketAddr) );

	if (success==SOCKET_ERROR) {
		message(TEXT("connect() failed! You sure that's the right ip address?"));
		/*char err[20];
		sprintf_s(err,20,"%u",WSAGetLastError());
		message(err, "Btw, this is the error code",0);*/
		goto connectError;
	}

  //Do an endian flip
	int outSize = (
		((fileSize & 0xFF000000) >> 24) |
		((fileSize & 0x00FF0000) >>  8) |
		((fileSize & 0x0000FF00) <<  8) |
		((fileSize & 0x000000FF) << 24)
	);

	TRANSMIT_FILE_BUFFERS transferBuffers;

	transferBuffers.Head=&outSize;
	transferBuffers.HeadLength=4;
	transferBuffers.Tail=NULL;
	transferBuffers.TailLength=0;

	BOOL bSuccess = TransmitFile(
		socketHandle,
		fileHandle,
		fileSize,
		0,
		NULL,
		&transferBuffers,
		TF_DISCONNECT | TF_USE_DEFAULT_WORKER 
	);

	if(!bSuccess) {
		goto transferError;
	}

	goto sendend;

transferError:
				
connectError:
	closesocket(socketHandle);
socketError:

getfilesizeError:
	CloseHandle(fileHandle);

createfileError:

sendend:
	;
}