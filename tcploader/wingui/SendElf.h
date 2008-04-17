#pragma once

#include "resource.h"

typedef void (*messageCallback)(LPTSTR);

void sendFile(unsigned long ipInt, unsigned short portInt, LPTSTR filename, messageCallback message) ;
INT_PTR CALLBACK MainDialog(HWND, UINT, WPARAM, LPARAM) ;
void do_cli(LPTSTR commandline) ;