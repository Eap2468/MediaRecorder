#include "pch.h"
#include "SimpleMediaRecorder.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/timestamp.h>
}

SIMPLE_MEDIA_EXIT_CODE StartSimpleMediaStream(PSIMPLE_MEDIA_RECORDER Recorder) {
	SYSTEMTIME SystemTime = { 0 };
	char RecordingDirectory[MAX_PATH];

	if (Recorder == 0 || Recorder->OutputDirectory == 0)
		return SIMPLE_MEDIA_EXIT_INVALID_PARAMETER;

	if (Recorder->InputList.Flink == &Recorder->InputList)
		return SIMPLE_MEDIA_EXIT_NO_REGISTERED_INPUTS;

	GetSystemTime(&SystemTime);
	snprintf(RecordingDirectory, sizeof(RecordingDirectory), "%s\\%d-%d-%d_%d.%d", Recorder->OutputDirectory, SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear, SystemTime.wHour, SystemTime.wMinute);
	if (!CreateDirectoryA(RecordingDirectory, 0) && GetLastError() != ERROR_ALREADY_EXISTS) {
		if (GetLastError() == ERROR_ACCESS_DENIED)
			return SIMPLE_MEDIA_EXIT_ACCESS_DENIED;
		else
			return SIMPLE_MEDIA_EXIT_INVALID_OUTPUT_DIRECTORY;
	}

	for (PSIMPLE_MEDIA_INPUT i = (PSIMPLE_MEDIA_INPUT)Recorder->InputList.Flink; (PLIST_ENTRY)i != &Recorder->InputList; i = (PSIMPLE_MEDIA_INPUT)i->InputList.Flink) {
		StartSimpleMediaInputStream(i, _strdup(RecordingDirectory));
	}
	
	return SIMPLE_MEDIA_EXIT_SUCCESS;
}

SIMPLE_MEDIA_EXIT_CODE StopSimpleMediaStream(PSIMPLE_MEDIA_RECORDER Recorder) {
	if (Recorder == 0)
		return SIMPLE_MEDIA_EXIT_INVALID_PARAMETER;

	if (Recorder->InputList.Flink == &Recorder->InputList)
		return SIMPLE_MEDIA_EXIT_NO_REGISTERED_INPUTS;

	for (PSIMPLE_MEDIA_INPUT i = (PSIMPLE_MEDIA_INPUT)Recorder->InputList.Flink; (PLIST_ENTRY)i != &Recorder->InputList; i = (PSIMPLE_MEDIA_INPUT)i->InputList.Flink) {
		StopSimpleMediaInputStream(i);
	}

	return SIMPLE_MEDIA_EXIT_SUCCESS;
}

void SimpleMediaInputRecordingThread(PSIMPLE_MEDIA_RECORDING_CONTEXT RecordingCtx) {
	PSIMPLE_MEDIA_INPUT Input = 0;
	AVFormatContext* RtspStreamCtx = 0;
	AVFormatContext* OutputFileCtx = 0;
	AVDictionary* RtspStreamOptions = 0;
	AVStream* InStream = 0;
	AVStream* OutStream = 0;
	AVCodecParameters* InParameters = 0;
	AVPacket* pkt = 0;
	SYSTEMTIME SystemTime = { 0 };
	SIZE_T ParsedUrlSize = 0;
	int* StreamMapping = 0;
	int OutIndex = 0;
	int FFmpegReturn = 0;
	char* ParsedUrl = 0;
	char OutputFilePath[MAX_PATH];

	Input = RecordingCtx->Input;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	GetSystemTime(&SystemTime);
	snprintf(OutputFilePath, sizeof(OutputFilePath), "%s\\%s-%d.%d.%d.mp4", RecordingCtx->RecordingDirectory, Input->Name, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);

	if (Input->Username != 0 && Input->Password != 0 && strlen(Input->Username) != 0 && strlen(Input->Password) != 0) {
		/* +6 for max port length and null byte */
		ParsedUrlSize = strlen("rtsp://@:") + strlen(Input->Ip) + strlen(Input->Username) + strlen(Input->Password) + 6;

		ParsedUrl = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ParsedUrlSize);
		if (ParsedUrl == 0) {
			Input->LastExitCode = SIMPLE_MEDIA_EXIT_NO_MEMORY;
			goto cleanup;
		};

		if (Input->Port == 0)
			snprintf(ParsedUrl, ParsedUrlSize, "rtsp://%s:%s@%s:%d", Input->Username, Input->Password, Input->Ip, 554);
		else
			snprintf(ParsedUrl, ParsedUrlSize, "rtsp://%s:%s@%s:%d", Input->Username, Input->Password, Input->Ip, Input->Port);
	}
	else {
		/* +6 for max port length and null byte */
		ParsedUrlSize = strlen("rtsp://:") + strlen(Input->Ip) + 6;
		ParsedUrl = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ParsedUrlSize);
		if (ParsedUrl == 0) {
			Input->LastExitCode = SIMPLE_MEDIA_EXIT_NO_MEMORY;
			goto cleanup;
		}

		if (Input->Port == 0)
			snprintf(ParsedUrl, ParsedUrlSize, "rtsp://%s:%d", Input->Ip, 554);
		else
			snprintf(ParsedUrl, ParsedUrlSize, "rtsp://%s:%d", Input->Ip, Input->Port);
	}

	av_dict_set(&RtspStreamOptions, "timeout", "1000000", 0);

	FFmpegReturn = avformat_open_input(&RtspStreamCtx, ParsedUrl, 0, &RtspStreamOptions);
	av_dict_free(&RtspStreamOptions);
	if (FFmpegReturn < 0) {
		Input->LastExitCode = SIMPLE_MEDIA_EXIT_FFMPEG_ERROR;
		Input->LastFFmpegError = FFmpegReturn;
		goto cleanup;
	}

	FFmpegReturn = avformat_find_stream_info(RtspStreamCtx, 0);
	if (FFmpegReturn < 0) {
		Input->LastExitCode = SIMPLE_MEDIA_EXIT_FFMPEG_ERROR;
		Input->LastFFmpegError = FFmpegReturn;
		goto cleanup;
	}

	FFmpegReturn = avformat_alloc_output_context2(&OutputFileCtx, 0, 0, OutputFilePath);
	if (FFmpegReturn < 0 || !OutputFileCtx) {
		Input->LastExitCode = SIMPLE_MEDIA_EXIT_FFMPEG_ERROR;
		Input->LastFFmpegError = FFmpegReturn;
		goto cleanup;
	}

	StreamMapping = (int*)av_calloc(RtspStreamCtx->nb_streams, sizeof(int));
	if (!StreamMapping) {
		Input->LastExitCode = SIMPLE_MEDIA_EXIT_FFMPEG_ERROR;
		Input->LastFFmpegError = AVERROR(ENOMEM);
		goto cleanup;
	}

	for (unsigned int i = 0; i < RtspStreamCtx->nb_streams; i++) {
		InStream = RtspStreamCtx->streams[i];
		InParameters = InStream->codecpar;

		if (InParameters->codec_type != AVMEDIA_TYPE_VIDEO) {
			StreamMapping[i] = -1;
			continue;
		}

		OutStream = avformat_new_stream(OutputFileCtx, 0);
		if (!OutStream) {
			Input->LastExitCode = SIMPLE_MEDIA_EXIT_FFMPEG_ERROR;
			Input->LastFFmpegError = AVERROR(ENOMEM);
			goto cleanup;
		}

		FFmpegReturn = avcodec_parameters_copy(OutStream->codecpar, InParameters);
		if (FFmpegReturn < 0) {
			Input->LastExitCode = SIMPLE_MEDIA_EXIT_FFMPEG_ERROR;
			Input->LastFFmpegError = FFmpegReturn;
			goto cleanup;
		}

		OutStream->codecpar->codec_tag = 0;
	}

	FFmpegReturn = avio_open(&OutputFileCtx->pb, OutputFilePath, AVIO_FLAG_WRITE);
	if (FFmpegReturn < 0) {
		Input->LastExitCode = SIMPLE_MEDIA_EXIT_FFMPEG_ERROR;
		Input->LastFFmpegError = FFmpegReturn;
		goto cleanup;
	}

	FFmpegReturn = avformat_write_header(OutputFileCtx, 0);
	if (FFmpegReturn < 0) {
		Input->LastExitCode = SIMPLE_MEDIA_EXIT_FFMPEG_ERROR;
		Input->LastFFmpegError = FFmpegReturn;
		goto cleanup;
	}

	pkt = av_packet_alloc();
	if (!pkt) {
		Input->LastExitCode = SIMPLE_MEDIA_EXIT_FFMPEG_ERROR;
		Input->LastFFmpegError = AVERROR(ENOMEM);
		goto cleanup;
	}

	/* Main record loop starts here */
	while (Input->Recording) {
		FFmpegReturn = av_read_frame(RtspStreamCtx, pkt);
		if (FFmpegReturn < 0)
			break;

		if (pkt->stream_index >= (int)RtspStreamCtx->nb_streams || StreamMapping[pkt->stream_index] < 0) {
			av_packet_unref(pkt);
			continue;
		}

		InStream = RtspStreamCtx->streams[pkt->stream_index];
		OutIndex = StreamMapping[pkt->stream_index];
		OutStream = OutputFileCtx->streams[OutIndex];

		pkt->stream_index = OutIndex;

		av_packet_rescale_ts(pkt, InStream->time_base, OutStream->time_base);

		pkt->pos = -1;

		FFmpegReturn = av_interleaved_write_frame(OutputFileCtx, pkt);
		if (FFmpegReturn < 0)
			break;
	}

	Input->LastExitCode = SIMPLE_MEDIA_EXIT_SUCCESS;

cleanup:
	Input->Recording = FALSE;

	if (ParsedUrl)
		HeapFree(GetProcessHeap(), 0, ParsedUrl);

	if (pkt)
		av_packet_free(&pkt);

	if (OutputFileCtx)
		FFmpegReturn = av_write_trailer(OutputFileCtx);

	if (RtspStreamCtx)
		avformat_close_input(&RtspStreamCtx);

	if (OutputFileCtx && OutputFileCtx->pb)
		avio_closep(&OutputFileCtx->pb);

	if (OutputFileCtx)
		avformat_free_context(OutputFileCtx);

	if (StreamMapping)
		av_freep(&StreamMapping);

	if (RecordingCtx->RecordingDirectory)
		free(RecordingCtx->RecordingDirectory);

	if (RecordingCtx)
		HeapFree(GetProcessHeap(), 0, RecordingCtx);
}

SIMPLE_MEDIA_EXIT_CODE StartSimpleMediaInputStream(PSIMPLE_MEDIA_INPUT Input, char* RecordingDirectoryPath) {
	PSIMPLE_MEDIA_RECORDING_CONTEXT RecordingCtx = 0;

	RecordingCtx = (PSIMPLE_MEDIA_RECORDING_CONTEXT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SIMPLE_MEDIA_RECORDING_CONTEXT));
	if (RecordingCtx == 0)
		return SIMPLE_MEDIA_EXIT_NO_MEMORY;

	RecordingCtx->Input = Input;
	RecordingCtx->RecordingDirectory = RecordingDirectoryPath;

	Input->Recording = TRUE;
	Input->hRecordingThread = CreateThread(0, 0x10000, (LPTHREAD_START_ROUTINE)SimpleMediaInputRecordingThread, RecordingCtx, 0, 0);
	if (Input->hRecordingThread == 0) {
		HeapFree(GetProcessHeap(), 0, RecordingCtx);
		return SIMPLE_MEDIA_EXIT_CREATE_RECORDING_THREAD_ERROR;
	}

	return SIMPLE_MEDIA_EXIT_SUCCESS;
}

SIMPLE_MEDIA_EXIT_CODE StopSimpleMediaInputStream(PSIMPLE_MEDIA_INPUT Input) {
	HANDLE hRecordingThread = 0;

	if (!Input->Recording)
		return SIMPLE_MEDIA_EXIT_SUCCESS;

	hRecordingThread = Input->hRecordingThread;

	Input->Recording = FALSE;
	if (WaitForSingleObject(hRecordingThread, SIMPLE_MEDIA_RECORDING_THREAD_TIMEOUT) == WAIT_TIMEOUT) {
		return SIMPLE_MEDIA_EXIT_TIMEOUT_REACHED;
	}

	Input->hRecordingThread = 0;
	CloseHandle(hRecordingThread);

	return SIMPLE_MEDIA_EXIT_SUCCESS;
}