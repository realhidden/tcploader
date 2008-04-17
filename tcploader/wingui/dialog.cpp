#include "stdafx.h"

#include "SendElf.h"

HWND hMainDialog;
HKEY softwareKey;

HWND hFilename;
HWND hPort;
HWND hIpaddress;
HWND hSendbutton;
void updateSendButton(void);
void guiMessage(LPTSTR message);

// Message handler for diaog box.
INT_PTR CALLBACK MainDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
		case WM_INITDIALOG: {
			TCHAR portString[10];
			TCHAR ipString[16];
			TCHAR filename[260];

			//Load some defaults
			_tcscpy_s(portString,10,TEXT("8080"));
			ipString[0]=0;
			filename[0]=0;

			long lSuccess=RegCreateKeyEx(
				HKEY_CURRENT_USER,
				TEXT("Software\\TCP Loader"),
				0,
				NULL,
				REG_OPTION_NON_VOLATILE,
				KEY_WRITE|KEY_READ,
				NULL,
				&softwareKey,
				NULL
			);

			if(lSuccess!=ERROR_SUCCESS) {
				goto regkeyopenError;
			}

			TCHAR keydata[260];
			DWORD keydataLength;
			TCHAR keyname[5];
			DWORD keynameLength;
			for(unsigned int keyindex=0;;) {
				keydataLength=259;
				memset(keydata,0,sizeof(keydata));
				keynameLength=5;
				memset(keyname,0,sizeof(keyname));

				lSuccess=RegEnumValue(
					softwareKey,
					keyindex++,
					keyname,
					&keynameLength,
					0,
					NULL,
					(LPBYTE)&keydata,
					&keydataLength
				);

				if(lSuccess!=ERROR_SUCCESS) {
					break;
				}

				if(_tcscmp(keyname,TEXT("File"))==0) {
					_tcscpy_s(filename,259,keydata);
				} else if(_tcscmp(keyname,TEXT("Ip"))==0) {
					_tcscpy_s(ipString,16,keydata);
				} else if(_tcscmp(keyname,TEXT("Port"))==0) {
					_tcscpy_s(portString,10,keydata);
				}

			}

			goto endofregread;

regkeyopenError:
			//Errors are ignored, they key most likly just didn't exist yet, since we where first
			//MessageBox(hDlg,"Could not open the registery key","Aw, crap",0);
endofregread:

			hPort=GetDlgItem(hDlg, IDC_PORT);
			hIpaddress=GetDlgItem(hDlg, IDC_IPADDRESS);
			hFilename=GetDlgItem(hDlg, IDC_FILENAME);
			hSendbutton=GetDlgItem(hDlg, IDC_SENDBUTTON);
			

			SetWindowText(hPort, portString);
			SetWindowText(hIpaddress, ipString);
			SetWindowText(hFilename, filename);
			
			DragAcceptFiles(hDlg,true);
			hMainDialog=hDlg;
			updateSendButton();

			SendMessage(
				hPort,
				EM_SETLIMITTEXT,
				6,
				0
			); 
		} break;

		case WM_DROPFILES: {
			HDROP hDrop = (HDROP)wParam;
			TCHAR filename[260];

			DragQueryFile(
				hDrop,
				0,
				filename,
				sizeof(filename)
			);

			SetWindowText(hFilename, filename);

			DragFinish(hDrop);
			//send button is updated since the controll will send a change message, so no need to do it manualy
		} break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_BROWSEBUTTON) {
				OPENFILENAME ofn;
				TCHAR filename[260];

				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFile = filename;

				GetWindowText(hFilename, filename, MAX_PATH);

				// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
				// use the contents of szFile to initialize itself.
				//ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(filename);
				ofn.lpstrFilter = TEXT("DOL's and ELF's\0*.DOL;*.ELF\0DOL's \0*.DOL\0ELF's\0*.ELF\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.hwndOwner=hDlg;

				// Display the Open dialog box. 
				if (GetOpenFileName(&ofn)==TRUE) {
					SetWindowText(hFilename, ofn.lpstrFile);
				}

				updateSendButton();

			} else if(LOWORD(wParam) == IDC_SENDBUTTON) {

				TCHAR filename [MAX_PATH+1];
        GetWindowText(hFilename, filename, MAX_PATH);
        
				char ipString [35];
				GetWindowTextA(hIpaddress, ipString, sizeof ipString);
				char portString [10];
				GetWindowTextA(hPort, portString, sizeof portString);

				unsigned long ipInt=inet_addr(ipString);
				unsigned int t =atoi(portString);
				unsigned short portInt=(unsigned short)t;

				sendFile(ipInt,portInt,filename,guiMessage);


			} else if(LOWORD(wParam)==IDC_FILENAME && HIWORD(wParam)==EN_CHANGE) {
				updateSendButton();
			} else if(LOWORD(wParam)==IDC_IPADDRESS	&& HIWORD(wParam)==EN_CHANGE) {
				updateSendButton();
			} else if(LOWORD(wParam)==IDC_PORT	&& HIWORD(wParam)==EN_CHANGE) {
				updateSendButton();
			}
		break;

		case WM_CLOSE: {
			EndDialog(hDlg, LOWORD(wParam));

			TCHAR filename [MAX_PATH+1];
      GetWindowText(hFilename, filename, MAX_PATH);
        
			TCHAR ipString [35];
			GetWindowText(hIpaddress, ipString, sizeof ipString);
			TCHAR portString [20];
			GetWindowText(hPort, portString, sizeof portString);

			long lSuccess=RegSetValueEx(
				softwareKey,
				TEXT("Port"),
				0,
				REG_SZ,
				(const BYTE*)portString,
				(DWORD)(_tcslen(portString)+1)*sizeof(TCHAR)
			);

			if(lSuccess!=ERROR_SUCCESS) {
				goto regkeywriteError;
			}

			lSuccess=RegSetValueEx(
				softwareKey,
				TEXT("Ip"),
				0,
				REG_SZ,
				(const BYTE*)ipString,
				(DWORD)(_tcslen(ipString)+1)*sizeof(TCHAR)
			);

			if(lSuccess!=ERROR_SUCCESS) {
				goto regkeywriteError;
			}

			lSuccess=RegSetValueEx(
				softwareKey,
				TEXT("File"),
				0,
				REG_SZ,
				(const BYTE*)filename,
				(DWORD)(_tcslen(filename)+1)*sizeof(TCHAR)
			);

			if(lSuccess!=ERROR_SUCCESS) {
				goto regkeywriteError;
			}

			goto endofregwrite;

			
regkeywriteError:
			MessageBox(hDlg,TEXT("Could not write to the registery key"),TEXT("Aw, crap"),0);
endofregwrite:
			RegCloseKey(softwareKey);

			hMainDialog=NULL;
			return (INT_PTR)TRUE;
		} break;
	}
	return (INT_PTR)FALSE;
}

void updateSendButton(void) {
	TCHAR filename [MAX_PATH+1];
  GetWindowText(hFilename, filename, MAX_PATH);

	long ipnumber;
	SendMessage(hIpaddress,IPM_GETADDRESS,0,(LPARAM)&ipnumber);

	TCHAR portString [10];
	GetWindowText(hPort, portString, sizeof portString);

	bool enable=(
		ipnumber!=0 &&
		_tcscmp(portString,TEXT(""))!=0 &&
		PathFileExists( filename)
	);

	EnableWindow(hSendbutton,enable);
}

void guiMessage(LPTSTR message) {
	MessageBox(hMainDialog,message,TEXT("Aw, crap"),0);
}