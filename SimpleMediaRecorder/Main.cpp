#include "pch.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/timestamp.h"
}

BOOL WINAPI DllMain(HINSTANCE hInstanceDLL, DWORD dwReason, PVOID Reserved) {
		
	if (dwReason == DLL_PROCESS_ATTACH)
		avformat_network_init();
	else if (dwReason == DLL_PROCESS_DETACH)
		avformat_network_deinit();

	return TRUE;
}