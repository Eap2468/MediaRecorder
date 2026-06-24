#pragma once
#include <Windows.h>

typedef enum _SIMPLE_MEDIA_EXIT_CODE {
	/* Success */
	SIMPLE_MEDIA_EXIT_SUCCESS = 0,
	/* Generic failure */
	SIMPLE_MEDIA_EXIT_FAILURE = 1,
	/* Failed memory allocation */
	SIMPLE_MEDIA_EXIT_NO_MEMORY = 2,
	/* Invalid parameter passed to function */
	SIMPLE_MEDIA_EXIT_INVALID_PARAMETER = 3,
	/* Buffer too small */
	SIMPLE_MEDIA_EXIT_BUFFER_OVERFLOW = 4,
	/* Access denied by OS */
	SIMPLE_MEDIA_EXIT_ACCESS_DENIED = 5,
	/* Requested file does not exist */
	SIMPLE_MEDIA_EXIT_FILE_NOT_FOUND = 6,
	/* Directory leading to a specified path does not exist */
	SIMPLE_MEDIA_EXIT_PATH_NOT_FOUND = 7,
	/* Directory path leads to a file */
	SIMPLE_MEDIA_EXIT_FILE_NOT_DIRECTORY = 8,
	/* Input already created for specified name */
	SIMPLE_MEDIA_EXIT_NAME_ALREADY_TAKEN = 9,
	/* Operation requires recording to be stopped */
	SIMPLE_MEDIA_EXIT_RECORDING_IN_PROGRESS = 10,
	/* No inputs registered to recorder */
	SIMPLE_MEDIA_EXIT_NO_REGISTERED_INPUTS = 11,
	/* Output directory is invalid */
	SIMPLE_MEDIA_EXIT_INVALID_OUTPUT_DIRECTORY = 12,
	/* Stream URL is not in a valid format */
	SIMPLE_MEDIA_EXIT_INVALID_URL = 13,
	/* FFmpeg function errored, result is stored in LastFFmpegError for an input */
	SIMPLE_MEDIA_EXIT_FFMPEG_ERROR = 14,
	/* Error occured while creating the recording thread */
	SIMPLE_MEDIA_EXIT_CREATE_RECORDING_THREAD_ERROR = 15,
	/* Timeout was reached before action finished */
	SIMPLE_MEDIA_EXIT_TIMEOUT_REACHED = 16,
	/* Function that searches for a particular input couldn't find the input */
	SIMPLE_MEDIA_EXIT_INPUT_NOT_FOUND = 17
} SIMPLE_MEDIA_EXIT_CODE;

typedef struct _SIMPLE_MEDIA_INPUT {
	LIST_ENTRY InputList;
	LPSTR Name;
	LPSTR Ip;
	USHORT Port;
	LPSTR Username;
	LPSTR Password;
	BOOLEAN Recording;
	HANDLE hRecordingThread;
	SIMPLE_MEDIA_EXIT_CODE LastExitCode;
	int LastFFmpegError = 0;
} SIMPLE_MEDIA_INPUT, * PSIMPLE_MEDIA_INPUT;

typedef struct _SIMPLE_MEDIA_RECORDER {
	LIST_ENTRY InputList;
	LPSTR OutputDirectory;
} SIMPLE_MEDIA_RECORDER, * PSIMPLE_MEDIA_RECORDER;

SIMPLE_MEDIA_EXIT_CODE CreateSimpleMediaRecorder(PSIMPLE_MEDIA_RECORDER* Recorder);
SIMPLE_MEDIA_EXIT_CODE CleanupSimpleMediaRecorder(PSIMPLE_MEDIA_RECORDER Recorder);
SIMPLE_MEDIA_EXIT_CODE AddSimpleMediaInput(PSIMPLE_MEDIA_RECORDER Recorder, LPCSTR Name, LPCSTR Ip, USHORT Port, LPCSTR Username, LPCSTR Password, PSIMPLE_MEDIA_INPUT* SimpleMediaInput);
SIMPLE_MEDIA_EXIT_CODE RemoveSimpleMediaInput(PSIMPLE_MEDIA_INPUT Input);
SIMPLE_MEDIA_EXIT_CODE RemoveSimpleMediaInputByName(PSIMPLE_MEDIA_RECORDER Recorder, LPSTR Name);
SIMPLE_MEDIA_EXIT_CODE SetSimpleMediaOutputDirectory(PSIMPLE_MEDIA_RECORDER Recorder, LPCSTR OutputDirectory);
SIMPLE_MEDIA_EXIT_CODE GetSimpleMediaOutputDirectory(PSIMPLE_MEDIA_RECORDER Recorder, LPSTR OutputDirectory, SIZE_T OutputDirectorySize, SIZE_T* ReturnOutputDirectorySize);
SIMPLE_MEDIA_EXIT_CODE StartSimpleMediaStream(PSIMPLE_MEDIA_RECORDER Recorder);
SIMPLE_MEDIA_EXIT_CODE StopSimpleMediaStream(PSIMPLE_MEDIA_RECORDER Recorder);
SIMPLE_MEDIA_EXIT_CODE StartSimpleMediaInputStream(PSIMPLE_MEDIA_INPUT Input, LPSTR RecordingDirectoryPath);
SIMPLE_MEDIA_EXIT_CODE StopSimpleMediaInputStream(PSIMPLE_MEDIA_INPUT Input);
