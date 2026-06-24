#include "pch.h"
#include "Logging.h"

void InitializeLogging() {
	/* 
	* Potentially will add logging functionality in the future
	* if there's occational crash cases and stuff but for now
	* just gonna have logging for debug builds
	*/
#ifdef _DEBUG
	HWND ConsoleWindow = 0;
	HANDLE hStdStream = INVALID_HANDLE_VALUE;
	FILE* fp = 0;

	ConsoleWindow = GetConsoleWindow();

	if (ConsoleWindow == 0) {
		if (!AllocConsole()) {
			return;
		}
	}

	hStdStream = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (hStdStream != INVALID_HANDLE_VALUE) {
		SetStdHandle(STD_INPUT_HANDLE, hStdStream);
		freopen_s(&fp, "CONIN$", "r", stdin);
	}

	hStdStream = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (hStdStream != INVALID_HANDLE_VALUE) {
		SetStdHandle(STD_OUTPUT_HANDLE, hStdStream);
		SetStdHandle(STD_ERROR_HANDLE, hStdStream);
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
	}

	log("[+] Logging started\n");
#endif
}