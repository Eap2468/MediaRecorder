#include "pch.h"
#include "Config.h"

BOOLEAN InitSession() {
	HRESULT result = S_OK;
	LSTATUS status = ERROR_SUCCESS;
	HKEY hMediaRecorderKey = 0;
	HKEY hInputsKey = 0;
	LPSTR CurrentOutputDirectory = 0;
	LPWSTR CurrentDesktop = 0;
	LONG OutputDirectoryLength = 0;
	wchar_t CurrentDirectory[MAX_PATH];

	status = RegCreateKeyA(HKEY_CURRENT_USER, "SOFTWARE\\MediaRecorder\\MediaRecorder", &hMediaRecorderKey);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d creating HKCU\\SOFTWARE\\MediaRecorder\\MediaRecorder registry key\n", status);
		return FALSE;
	}

	status = RegCreateKeyA(hMediaRecorderKey, "Inputs", &hInputsKey);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d creating Inputs registry key\n", status);

		RegCloseKey(hMediaRecorderKey);
		return FALSE;
	}
	
	RegCloseKey(hInputsKey);

	if (RegQueryValueA(hMediaRecorderKey, "OutputDirectory", CurrentOutputDirectory, &OutputDirectoryLength) != ERROR_MORE_DATA && OutputDirectoryLength != 0) {
		result = SHGetKnownFolderPath(FOLDERID_Desktop, 0, 0, &CurrentDesktop);
		if (result != S_OK) {
			if (GetCurrentDirectoryW(sizeof(CurrentDirectory), CurrentDirectory) == 0) {
				log("[-] All methods to get initial directory failed\n");
				return FALSE;
			}
			else {
				status = RegSetValueExW(hMediaRecorderKey, L"OutputDirectory", 0, REG_SZ, (BYTE*)CurrentDirectory, sizeof(CurrentDirectory));
				if (status != ERROR_SUCCESS) {
					log("[-] Error %d setting initial output directory path\n", status);
					return FALSE;
				}
			}
		}
		else {
			status = RegSetValueExW(hMediaRecorderKey, L"OutputDirectory", 0, REG_SZ, (BYTE*)CurrentDesktop, (wcslen(CurrentDesktop) * 2) + 2);
			if (status != ERROR_SUCCESS) {
				log("[-] Error %d setting initial output directory path\n", status);
				return FALSE;
			}
		}
	}
	
	RegCloseKey(hMediaRecorderKey);
	return TRUE;
}

void SaveOutputDirectory(LPSTR OutputDirectory) {
	LSTATUS status = ERROR_SUCCESS;
	HKEY hMediaRecorder = 0;

	status = RegOpenKeyA(HKEY_CURRENT_USER, "SOFTWARE\\MediaRecorder\\MediaRecorder", &hMediaRecorder);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d opening handle to media recorder key to save output directory value\n", status);
		return;
	}

	status = RegSetValueExA(hMediaRecorder, "OutputDirectory", 0, REG_SZ, (BYTE*)OutputDirectory, strlen(OutputDirectory) + 1);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d saving output directory to registry value\n", status);
	}

	RegCloseKey(hMediaRecorder);
}

LSTATUS GetOutputDirectoryA(LPSTR OutputDirectory, ULONG OutputDirectorySize, PULONG ReturnedBytes) {
	LSTATUS status = ERROR_SUCCESS;

	status = RegGetValueA(HKEY_CURRENT_USER, "SOFTWARE\\MediaRecorder\\MediaRecorder", "OutputDirectory", RRF_RT_REG_SZ, 0, OutputDirectory, &OutputDirectorySize);
	
	if (ReturnedBytes) {
		*ReturnedBytes = OutputDirectorySize;
	}

	return status;
}

LSTATUS GetOutputDirectoryW(LPWSTR OutputDirectory, ULONG OutputDirectorySize, PULONG ReturnedBytes) {
	LSTATUS status = ERROR_SUCCESS;

	status = RegGetValueW(HKEY_CURRENT_USER, L"SOFTWARE\\MediaRecorder\\MediaRecorder", L"OutputDirectory", RRF_RT_REG_SZ, 0, OutputDirectory, &OutputDirectorySize);
	
	if (ReturnedBytes) {
		*ReturnedBytes = OutputDirectorySize;
	}

	return status;
}

LSTATUS SaveInputToConfig(LPSTR Name, LPSTR Ip, USHORT Port, LPSTR Username, LPSTR Password) {
	LSTATUS status = ERROR_SUCCESS;
	HKEY hInput;
	char KeyPath[0x200];

	memset(KeyPath, 0, sizeof(KeyPath));
	snprintf(KeyPath, sizeof(KeyPath), "SOFTWARE\\MediaRecorder\\MediaRecorder\\Inputs\\%s", Name);

	status = RegCreateKeyA(HKEY_CURRENT_USER, KeyPath, &hInput);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d creating registry key at %s to save input\n", status, KeyPath);

		status = RegDeleteKeyA(hInput, 0);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d deleting input %s after error in save\n", status, Name);
		}
		RegCloseKey(hInput);

		return status;
	}

	status = RegSetValueExA(hInput, "IP", 0, REG_SZ, (BYTE*)Ip, strlen(Ip) + 1);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d saving IP value for input %s\n", status, Name);

		status = RegDeleteKeyA(hInput, 0);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d deleting input %s after error in save\n", status, Name);
		}
		RegCloseKey(hInput);

		return status;
	}

	status = RegSetValueExA(hInput, "Port", 0, REG_BINARY, (BYTE*)&Port, sizeof(Port));
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d saving port value for input %s\n", status, Name);

		status = RegDeleteKeyA(hInput, 0);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d deleting input %s after error in save\n", status, Name);
		}
		RegCloseKey(hInput);

		return status;
	}

	status = RegSetValueExA(hInput, "Username", 0, REG_SZ, (BYTE*)Username, strlen(Username) + 1);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d saving username value for input %s\n", status, Name);

		status = RegDeleteKeyA(hInput, 0);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d deleting input %s after error in save\n", status, Name);
		}
		RegCloseKey(hInput);

		return status;
	}

	status = RegSetValueExA(hInput, "Password", 0, REG_SZ, (BYTE*)Password, strlen(Password) + 1);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d saving password value for input %s\n", status, Name);

		status = RegDeleteKeyA(hInput, 0);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d deleting input %s after error in save\n", status, Name);
		}
		RegCloseKey(hInput);

		return status;
	}

	RegCloseKey(hInput);
	return status;
}

LSTATUS RemoveInputFromConfig(LPSTR Name) {
	char KeyPath[0x200];

	memset(KeyPath, 0, sizeof(KeyPath));
	snprintf(KeyPath, sizeof(KeyPath), "SOFTWARE\\MediaRecorder\\MediaRecorder\\Inputs\\%s", Name);

	return RegDeleteKeyA(HKEY_CURRENT_USER, KeyPath);
}

LSTATUS GetInputListFromConfig(PMEDIA_RECORDER_SESSION_INFO* SavedInputList) {
	LSTATUS status = ERROR_SUCCESS;
	HKEY hInputs = 0;
	PMEDIA_RECORDER_SESSION_INFO LocalSavedInputList = 0;
	LPSTR KeyName = 0;
	LPSTR Name = 0;
	LPSTR Ip = 0;
	LPSTR Username = 0;
	LPSTR Password = 0;
	ULONG ValueLength = 0;
	ULONG NumberOfInputs = 0;
	ULONG LongestKeyNameLength = 0;
	ULONG KeyNameSize = 0;
	USHORT Port = 0;

	if (SavedInputList == 0)
		return ERROR_INVALID_PARAMETER;

	*SavedInputList = 0;

	status = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\MediaRecorder\\MediaRecorder\\Inputs", 0, KEY_ALL_ACCESS, &hInputs);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d opening handle to saved inputs registry key\n", status);
		return status;
	}

	status = RegQueryInfoKeyA(hInputs, 0, 0, 0, &NumberOfInputs, &LongestKeyNameLength, 0, 0, 0, 0, 0, 0);
	if (status != ERROR_SUCCESS) {
		log("[-] Error %d querying subkey count and length for inputs registry key\n", status);
		return status;
	}
	LongestKeyNameLength += 1;

	KeyName = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, LongestKeyNameLength);
	if (KeyName == 0) {
		log("[-] Error %d allocating memory to store key names\n", GetLastError());

		status = GetLastError();
		goto error;
	}

	LocalSavedInputList = (PMEDIA_RECORDER_SESSION_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MEDIA_RECORDER_SESSION_INFO) +
		(sizeof(MEDIA_RECORDER_SESSION_INFO_ENTRY) * NumberOfInputs));
	if (LocalSavedInputList == 0) {
		log("[-] Error %d allocating memory for media recorder session info structure\n", GetLastError());

		status = GetLastError();
		goto error;;
	}

	LocalSavedInputList->NumberOfEntries = NumberOfInputs;

	for (ULONG i = 0; i < NumberOfInputs; i++) {
		KeyNameSize = LongestKeyNameLength;

		memset(KeyName, 0, LongestKeyNameLength);
		status = RegEnumKeyExA(hInputs, i, KeyName, &KeyNameSize, 0, 0, 0, 0);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d enumerating key index %d while getting saved inputs from registry\n", status, i);
			goto error;
		}

		Name = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, strlen(KeyName) + 1);
		if (Name == 0) {
			log("[-] Error %d allocating memory for name for input %s\n", GetLastError(), KeyName);
			status = GetLastError();
			goto error;
		}
		memcpy(Name, KeyName, strlen(KeyName));

		ValueLength = 0;
		status = RegGetValueA(hInputs, KeyName, "Ip", RRF_RT_REG_SZ, 0, Ip, &ValueLength);
		if (status != ERROR_SUCCESS && status != ERROR_MORE_DATA) {
			log("[-] Error %d getting IP value for input %s\n", status, KeyName);
			goto error;
		}

		Ip = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ValueLength);
		if (Ip == 0) {
			log("[-] Error %d allocating memory for IP value for input %s\n", GetLastError(), KeyName);
			goto error;
		}

		status = RegGetValueA(hInputs, KeyName, "Ip", RRF_RT_REG_SZ, 0, Ip, &ValueLength);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d getting IP value for input %s\n", status, KeyName);
			goto error;
		}

		ValueLength = sizeof(Port);
		status = RegGetValueA(hInputs, KeyName, "Port", RRF_RT_REG_BINARY, 0, &Port, &ValueLength);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d getting Port value for input %s\n", status, KeyName);
			goto error;
		}

		ValueLength = 0;
		status = RegGetValueA(hInputs, KeyName, "Username", RRF_RT_REG_SZ, 0, Username, &ValueLength);
		if (status != ERROR_SUCCESS && status != ERROR_MORE_DATA) {
			log("[-] Error %d getting IP value for input %s\n", status, KeyName);
			goto error;
		}

		Username = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ValueLength);
		if (Username == 0) {
			log("[-] Error %d allocating memory for Username value for input %s\n", GetLastError(), KeyName);
			goto error;
		}

		status = RegGetValueA(hInputs, KeyName, "Username", RRF_RT_REG_SZ, 0, Username, &ValueLength);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d getting Username value for input %s\n", status, KeyName);
			goto error;
		}

		ValueLength = 0;
		status = RegGetValueA(hInputs, KeyName, "Password", RRF_RT_REG_SZ, 0, Password, &ValueLength);
		if (status != ERROR_SUCCESS && status != ERROR_MORE_DATA) {
			log("[-] Error %d getting IP value for input %s\n", status, KeyName);
			goto error;
		}

		Password = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ValueLength);
		if (Password == 0) {
			log("[-] Error %d allocating memory for Password value for input %s\n", GetLastError(), KeyName);
			goto error;
		}

		status = RegGetValueA(hInputs, KeyName, "Password", RRF_RT_REG_SZ, 0, Password, &ValueLength);
		if (status != ERROR_SUCCESS) {
			log("[-] Error %d getting Password value for input %s\n", status, KeyName);
			goto error;
		}
		
		LocalSavedInputList->Entries[i].Name = Name;
		LocalSavedInputList->Entries[i].Ip = Ip;
		LocalSavedInputList->Entries[i].Port = Port;
		LocalSavedInputList->Entries[i].Username = Username;
		LocalSavedInputList->Entries[i].Password = Password;
	}

	*SavedInputList = LocalSavedInputList;
	goto cleanup;

error:
	for (ULONG i = 0; i < LocalSavedInputList->NumberOfEntries; i++) {
		if (LocalSavedInputList->Entries[i].Name)
			HeapFree(GetProcessHeap(), 0, LocalSavedInputList->Entries[i].Name);
		if (LocalSavedInputList->Entries[i].Ip)
			HeapFree(GetProcessHeap(), 0, LocalSavedInputList->Entries[i].Ip);
		if (LocalSavedInputList->Entries[i].Username)
			HeapFree(GetProcessHeap(), 0, LocalSavedInputList->Entries[i].Username);
		if (LocalSavedInputList->Entries[i].Password)
			HeapFree(GetProcessHeap(), 0, LocalSavedInputList->Entries[i].Password);
	}

	if (LocalSavedInputList)
		HeapFree(GetProcessHeap(), 0, LocalSavedInputList);

cleanup:
	if (hInputs)
		RegCloseKey(hInputs);
	if (KeyName)
		HeapFree(GetProcessHeap(), 0, KeyName);

	return status;
}

void CleanupInputList(PMEDIA_RECORDER_SESSION_INFO SavedSessionInfo) {

}