#include "pch.h"
#include "SimpleMediaRecorder.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
}

SIMPLE_MEDIA_EXIT_CODE CreateSimpleMediaRecorder(PSIMPLE_MEDIA_RECORDER* Recorder) {
	PSIMPLE_MEDIA_RECORDER LocalRecorder = 0;

	if (Recorder == 0) {
		return SIMPLE_MEDIA_EXIT_INVALID_PARAMETER;
	}

	LocalRecorder = (PSIMPLE_MEDIA_RECORDER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SIMPLE_MEDIA_RECORDER));
	if (LocalRecorder == 0) {
		*Recorder = 0;
		return SIMPLE_MEDIA_EXIT_NO_MEMORY;
	}

	memset(LocalRecorder, 0, sizeof(SIMPLE_MEDIA_RECORDER));
	LocalRecorder->InputList.Flink = &LocalRecorder->InputList;
	LocalRecorder->InputList.Blink = &LocalRecorder->InputList;
	
	*Recorder = LocalRecorder;

	return SIMPLE_MEDIA_EXIT_SUCCESS;
}

SIMPLE_MEDIA_EXIT_CODE CleanupSimpleMediaRecorder(PSIMPLE_MEDIA_RECORDER Recorder) {
	PSIMPLE_MEDIA_INPUT FlinkInput = 0;

	if (Recorder == 0)
		return SIMPLE_MEDIA_EXIT_INVALID_PARAMETER;

	for (PSIMPLE_MEDIA_INPUT i = (PSIMPLE_MEDIA_INPUT)Recorder->InputList.Flink; (PLIST_ENTRY)i != &Recorder->InputList;) {
		if (i->Recording)
			return SIMPLE_MEDIA_EXIT_RECORDING_IN_PROGRESS;

		FlinkInput = (PSIMPLE_MEDIA_INPUT)i->InputList.Flink;
		RemoveSimpleMediaInput(i);
		i = FlinkInput;
	}

	if (Recorder->OutputDirectory)
		HeapFree(GetProcessHeap(), 0, Recorder->OutputDirectory);
	
	HeapFree(GetProcessHeap(), 0, Recorder);

	return SIMPLE_MEDIA_EXIT_SUCCESS;
}

SIMPLE_MEDIA_EXIT_CODE SetSimpleMediaOutputDirectory(PSIMPLE_MEDIA_RECORDER Recorder, LPCSTR OutputDirectory) {
	LPSTR LocalOutputDirectory = 0;
	SIZE_T OutputDirectorySize = 0;
	DWORD dwDirectoryAttributes = 0;

	if (OutputDirectory == 0)
		return SIMPLE_MEDIA_EXIT_INVALID_PARAMETER;

	OutputDirectorySize = strlen(OutputDirectory) + 1;

	dwDirectoryAttributes = GetFileAttributesA(OutputDirectory);
	if (dwDirectoryAttributes == INVALID_FILE_ATTRIBUTES) {
		if (GetLastError() == ERROR_ACCESS_DENIED) {
			return SIMPLE_MEDIA_EXIT_ACCESS_DENIED;
		}
		else if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			return SIMPLE_MEDIA_EXIT_FILE_NOT_FOUND;
		}
		else if (GetLastError() == ERROR_PATH_NOT_FOUND) {
			return SIMPLE_MEDIA_EXIT_PATH_NOT_FOUND;
		}

		return SIMPLE_MEDIA_EXIT_FAILURE;
	}

	if ((dwDirectoryAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		return SIMPLE_MEDIA_EXIT_FILE_NOT_DIRECTORY;

	LocalOutputDirectory = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, OutputDirectorySize);
	if (LocalOutputDirectory == 0)
		return SIMPLE_MEDIA_EXIT_NO_MEMORY;

	memcpy(LocalOutputDirectory, OutputDirectory, OutputDirectorySize);

	if (Recorder->OutputDirectory != 0)
		HeapFree(GetProcessHeap(), 0, Recorder->OutputDirectory);

	Recorder->OutputDirectory = LocalOutputDirectory;

	return SIMPLE_MEDIA_EXIT_SUCCESS;
}

SIMPLE_MEDIA_EXIT_CODE GetSimpleMediaOutputDirectory(PSIMPLE_MEDIA_RECORDER Recorder, LPSTR OutputDirectory, SIZE_T OutputDirectorySize, SIZE_T* ReturnOutputDirectorySize) {
	SIZE_T LocalOutputDirectorySize = 0;
	
	LocalOutputDirectorySize = strlen(OutputDirectory) + 1;
	
	if (OutputDirectorySize < OutputDirectorySize) {
		if (ReturnOutputDirectorySize != 0)
			*ReturnOutputDirectorySize = strlen(Recorder->OutputDirectory);

		return SIMPLE_MEDIA_EXIT_BUFFER_OVERFLOW;
	}

	memcpy(OutputDirectory, Recorder->OutputDirectory, OutputDirectorySize);
	if (ReturnOutputDirectorySize != 0)
		*ReturnOutputDirectorySize = strlen(Recorder->OutputDirectory);

	return SIMPLE_MEDIA_EXIT_SUCCESS;
}