#include "pch.h"
#include "SimpleMediaRecorder.h"

SIMPLE_MEDIA_EXIT_CODE AddSimpleMediaInput(PSIMPLE_MEDIA_RECORDER Recorder, LPCSTR Name, LPCSTR Ip, USHORT Port, LPCSTR Username, LPCSTR Password, PSIMPLE_MEDIA_INPUT* SimpleMediaInput) {
	PSIMPLE_MEDIA_INPUT LocalSimpleMediaInput = 0;

	if (Name == 0 || Ip == 0 || (Username != 0 && Password == 0) || (Username == 0 && Password != 0) || strlen(Name) == 0 || strlen(Ip) == 0)
		return SIMPLE_MEDIA_EXIT_INVALID_PARAMETER;

	for (PSIMPLE_MEDIA_INPUT i = (PSIMPLE_MEDIA_INPUT)Recorder->InputList.Flink; (PLIST_ENTRY)i != &Recorder->InputList; i = (PSIMPLE_MEDIA_INPUT)i->InputList.Flink) {
		if (_stricmp(i->Name, Name) == 0) {
			return SIMPLE_MEDIA_EXIT_NAME_ALREADY_TAKEN;
		}
	}

	LocalSimpleMediaInput = (PSIMPLE_MEDIA_INPUT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SIMPLE_MEDIA_INPUT));
	if (LocalSimpleMediaInput == 0) {
		return SIMPLE_MEDIA_EXIT_NO_MEMORY;
	}

	LocalSimpleMediaInput->Name = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, strlen(Name) + 1);
	if (LocalSimpleMediaInput->Name == 0) {
		HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput);
		return SIMPLE_MEDIA_EXIT_NO_MEMORY;
	}
	memcpy(LocalSimpleMediaInput->Name, Name, strlen(Name) + 1);

	LocalSimpleMediaInput->Ip = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, strlen(Ip) + 1);
	if (LocalSimpleMediaInput->Ip == 0) {
		HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput->Name);
		HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput);
		return SIMPLE_MEDIA_EXIT_NO_MEMORY;
	}
	memcpy(LocalSimpleMediaInput->Ip, Ip, strlen(Ip) + 1);

	if (Username != 0 && Password != 0) {
		LocalSimpleMediaInput->Username = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, strlen(Username) + 1);
		if (LocalSimpleMediaInput->Username == 0) {
			HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput->Ip);
			HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput->Name);
			HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput);
			return SIMPLE_MEDIA_EXIT_NO_MEMORY;
		}
		memcpy(LocalSimpleMediaInput->Username, Username, strlen(Username) + 1);

		LocalSimpleMediaInput->Password = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, strlen(Username) + 1);
		if (LocalSimpleMediaInput->Password == 0) {
			HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput->Username);
			HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput->Ip);
			HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput->Name);
			HeapFree(GetProcessHeap(), 0, LocalSimpleMediaInput);
			return SIMPLE_MEDIA_EXIT_NO_MEMORY;
		}
		memcpy(LocalSimpleMediaInput->Password, Password, strlen(Password) + 1);
	}

	LocalSimpleMediaInput->InputList.Flink = &Recorder->InputList;
	LocalSimpleMediaInput->InputList.Blink = Recorder->InputList.Blink;

	Recorder->InputList.Blink->Flink = &LocalSimpleMediaInput->InputList;
	Recorder->InputList.Blink = &LocalSimpleMediaInput->InputList;

	LocalSimpleMediaInput->LastExitCode = SIMPLE_MEDIA_EXIT_SUCCESS;

	if (SimpleMediaInput != 0)
		*SimpleMediaInput = LocalSimpleMediaInput;

	return SIMPLE_MEDIA_EXIT_SUCCESS;
}

SIMPLE_MEDIA_EXIT_CODE RemoveSimpleMediaInput(PSIMPLE_MEDIA_INPUT Input) {
	if (Input == 0)
		return SIMPLE_MEDIA_EXIT_INVALID_PARAMETER;

	if (Input->Recording) {
		return SIMPLE_MEDIA_EXIT_RECORDING_IN_PROGRESS;
	}

	Input->InputList.Blink->Flink = Input->InputList.Flink;
	Input->InputList.Flink->Blink = Input->InputList.Blink;

	if (Input->Name)
		HeapFree(GetProcessHeap(), 0, Input->Name);
	if (Input->Ip)
		HeapFree(GetProcessHeap(), 0, Input->Ip);
	if (Input->Username)
		HeapFree(GetProcessHeap(), 0, Input->Username);
	if (Input->Password)
		HeapFree(GetProcessHeap(), 0, Input->Password);

	HeapFree(GetProcessHeap(), 0, Input);

	return SIMPLE_MEDIA_EXIT_SUCCESS;
}

SIMPLE_MEDIA_EXIT_CODE RemoveSimpleMediaInputByName(PSIMPLE_MEDIA_RECORDER Recorder, LPSTR Name) {
	if (Recorder == 0)
		return SIMPLE_MEDIA_EXIT_INVALID_PARAMETER;

	for (PSIMPLE_MEDIA_INPUT i = (PSIMPLE_MEDIA_INPUT)Recorder->InputList.Flink; (PLIST_ENTRY)i != &Recorder->InputList; i = (PSIMPLE_MEDIA_INPUT)i->InputList.Flink) {
		if (_stricmp(i->Name, Name) == 0) {
			return RemoveSimpleMediaInput(i);
		}
	}

	return SIMPLE_MEDIA_EXIT_NO_REGISTERED_INPUTS;
}