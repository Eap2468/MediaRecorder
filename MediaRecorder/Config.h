#pragma once
#include "pch.h"
#include "Logging.h"

typedef struct _MEDIA_RECORDER_SESSION_INFO_ENTRY {
	LPSTR Name;
	LPSTR Ip;
	USHORT Port;
	LPSTR Username;
	LPSTR Password;
} MEDIA_RECORDER_SESSION_INFO_ENTRY, * PMEDIA_RECORDER_SESSION_INFO_ENTRY;

typedef struct _MEDIA_RECORDER_SESSION_INFO {
	ULONG NumberOfEntries;
	MEDIA_RECORDER_SESSION_INFO_ENTRY Entries[1];
} MEDIA_RECORDER_SESSION_INFO, *PMEDIA_RECORDER_SESSION_INFO;

BOOLEAN InitSession();
void SaveOutputDirectory(LPSTR OutputDirectory);
LSTATUS GetOutputDirectoryA(LPSTR OutputDirectory, ULONG OutputDirectorySize, PULONG ReturnedBytes);
LSTATUS GetOutputDirectoryW(LPWSTR OutputDirectory, ULONG OutputDirectorySize, PULONG ReturnedBytes);
LSTATUS SaveInputToConfig(LPSTR Name, LPSTR Ip, USHORT Port, LPSTR Username, LPSTR Password);
LSTATUS RemoveInputFromConfig(LPSTR Name);
LSTATUS GetInputListFromConfig(PMEDIA_RECORDER_SESSION_INFO* SavedInputList);
void CleanupInputList(PMEDIA_RECORDER_SESSION_INFO SavedSessionInfo);
