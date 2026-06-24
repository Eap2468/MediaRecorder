#include "pch.h"
#include "StartStopPage.h"

using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml;

extern PSIMPLE_MEDIA_RECORDER Recorder;

void StartStopButtonCallback(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs);
void StartRecording(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs);
void StopRecording(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs);

/*
* TODO in morning, I'm pretty sure the DispatchTimer constructor is what's causing the call from wrong thread error 
* to occur when running. A dirty solution to get around the issue is to just define a structure that contains a DispatchTimer,
* define a pointer to that structure type as a global variable, then instantiate the structure and corresponding class in
* InitStartStopPage and use the global variable as a lasting reference to the timer (and potentially other classes needed
* for things like the TextBlock). Also if the allocations get unexpecidly freed smart pointer could be used to force keep a 
* reference so the allocation doesn't get freed while keeping the global variable reference.
*/

winrt::Windows::Foundation::TimeSpan RecordingTimeSpan;
DispatcherTimer RecordingTimeUpdateTimer = { 0 };
TextBlock TimerRecordingTimeBlock = { 0 };

/* Swap out for a check with the recording library when doing intigration */
BOOLEAN IsRecording = FALSE;

Page InitStartStopPage() {
	Page page;
	Grid grid;
	StackPanel MainPanel;
	TextBlock RecordingTimeBlock;
	Button StartStopButton;
	Border RecordingTimeBlockBorder;
	SolidColorBrush StartStopButtonInitialColor;
	SolidColorBrush RecordingTimeBlockBorderColor;
	Thickness RecordingTimeBlockThickness;

	StartStopButton.Style(Application::Current().Resources().Lookup(winrt::box_value(L"AccentButtonStyle")).as<Style>());
	StartStopButton.Content(winrt::box_value(L"Start"));
	StartStopButton.Click(StartStopButtonCallback);
	
	StartStopButtonInitialColor.Color(winrt::Microsoft::UI::Colors::LightGreen());
	StartStopButton.Background(StartStopButtonInitialColor);

	RecordingTimeBlock.Name(L"RecordingTimeBlock");
	RecordingTimeBlock.Text(L"Recording time: 00:00:00");

	RecordingTimeBlockBorderColor.Color(winrt::Microsoft::UI::Colors::Black());
	RecordingTimeBlockBorder.BorderBrush(RecordingTimeBlockBorderColor);

	RecordingTimeBlockThickness.Top = 2;
	RecordingTimeBlockThickness.Bottom = 2;
	RecordingTimeBlockThickness.Left = 2;
	RecordingTimeBlockThickness.Right = 2;
	RecordingTimeBlockBorder.BorderThickness(RecordingTimeBlockThickness);

	RecordingTimeBlockBorder.Child(RecordingTimeBlock);

	MainPanel.Orientation(Orientation::Vertical);
	MainPanel.Spacing(5);
	MainPanel.Children().Append(StartStopButton);
	MainPanel.Children().Append(RecordingTimeBlockBorder);

	grid.Children().Append(MainPanel);
	grid.HorizontalAlignment(HorizontalAlignment::Left);

	page.Content(grid);
	return page;
}

void RecordingTimeUpdateTick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e) {
	std::wstring RecordingTime = L"Recording time: ";
	RecordingTimeSpan += std::chrono::seconds(1);

	RecordingTime += std::format(L"{:%T}", std::chrono::duration_cast<std::chrono::seconds>(RecordingTimeSpan));

	TimerRecordingTimeBlock.Text(RecordingTime);
}

void StartStopButtonCallback(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs) {
	if (RecordingTimeUpdateTimer == 0)
		RecordingTimeUpdateTimer = DispatcherTimer();

	if (IsRecording)
		StopRecording(sender, EventArgs);
	else
		StartRecording(sender, EventArgs);
}

void StartRecording(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs) {
	SIMPLE_MEDIA_EXIT_CODE SimpleMediaCode = SIMPLE_MEDIA_EXIT_SUCCESS;
	Button SenderButton = sender.as<Button>();
	StackPanel ParentElement = SenderButton.Parent().as<StackPanel>();
	Border RecordingTimeBlockBorder = ParentElement.Children().GetAt(1).as<Border>();
	SolidColorBrush StopColor;
	wchar_t ErrorBuffer[1024];

	log("[+] Start recording button pressed\n");

	TimerRecordingTimeBlock = RecordingTimeBlockBorder.Child().as<TextBlock>();

	RecordingTimeSpan = std::chrono::seconds(0);

	if (RecordingTimeUpdateTimer.Interval() == std::chrono::seconds(0)) {
		RecordingTimeUpdateTimer.Interval(std::chrono::seconds(1));
		RecordingTimeUpdateTimer.Tick(RecordingTimeUpdateTick);
	}

	SimpleMediaCode = StartSimpleMediaStream(Recorder);
	if (SimpleMediaCode != SIMPLE_MEDIA_EXIT_SUCCESS) {
		log("[-] SimpleMediaRecorder error %d starting streams\n", SimpleMediaCode);

		memset(ErrorBuffer, 0, sizeof(ErrorBuffer));

		if (SimpleMediaCode == SIMPLE_MEDIA_EXIT_NO_REGISTERED_INPUTS)
			swprintf_s(ErrorBuffer, sizeof(ErrorBuffer) / sizeof(ErrorBuffer[0]), L"SimpleMediaRecorder no inputs registered", SimpleMediaCode);
		else
			swprintf_s(ErrorBuffer, sizeof(ErrorBuffer) / sizeof(ErrorBuffer[0]), L"SimpleMediaRecorder error %d starting streams", SimpleMediaCode);
		
		CreateErrorMessageBox(ParentElement.XamlRoot(), L"Start Stream Error", ErrorBuffer);
		return;
	}

	RecordingTimeUpdateTimer.Start();
	IsRecording = TRUE;

	Sleep(1500);

	for (PSIMPLE_MEDIA_INPUT i = (PSIMPLE_MEDIA_INPUT)Recorder->InputList.Flink; (PLIST_ENTRY)i != &Recorder->InputList; i = (PSIMPLE_MEDIA_INPUT)i->InputList.Flink) {
		if (i->LastExitCode != SIMPLE_MEDIA_EXIT_SUCCESS) {
			if (i->LastExitCode == SIMPLE_MEDIA_EXIT_FFMPEG_ERROR && i->LastFFmpegError == -138) {
				memset(ErrorBuffer, 0, sizeof(ErrorBuffer));

				swprintf_s(ErrorBuffer, sizeof(ErrorBuffer) / sizeof(ErrorBuffer[0]), L"Error connecting to RTSP for %S, potentially wrong ip/port, incorrect credentials, or RTSP might be disabled", i->Name);
				CreateErrorMessageBox(ParentElement.XamlRoot(), L"Stream Start Error", ErrorBuffer);
			}
			else {
				memset(ErrorBuffer, 0, sizeof(ErrorBuffer));

				if (i->LastExitCode == SIMPLE_MEDIA_EXIT_FFMPEG_ERROR)
					swprintf_s(ErrorBuffer, sizeof(ErrorBuffer) / sizeof(ErrorBuffer[0]), L"SimpleMediaRecorder ffmpeg error %d starting %S", i->LastFFmpegError, i->Name);
				else
					swprintf_s(ErrorBuffer, sizeof(ErrorBuffer) / sizeof(ErrorBuffer[0]), L"SimpleMediaRecorder error %d starting %S", i->LastExitCode, i->Name);
				CreateErrorMessageBox(ParentElement.XamlRoot(), L"Stream Start Error", ErrorBuffer);
			}
		}
	}

	StopColor.Color(winrt::Microsoft::UI::Colors::Red());

	SenderButton.Content(winrt::box_value(L"Stop"));
	SenderButton.Background(StopColor);
}

void StopRecording(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs) {
	SIMPLE_MEDIA_EXIT_CODE SimpleMediaCode = SIMPLE_MEDIA_EXIT_SUCCESS;
	Button SenderButton = sender.as<Button>();
	StackPanel ParentElement = SenderButton.Parent().as<StackPanel>();
	SolidColorBrush StartColor;

	log("[+] Stop recording button pressed\n");

	SimpleMediaCode = StopSimpleMediaStream(Recorder);
	if (SimpleMediaCode != SIMPLE_MEDIA_EXIT_SUCCESS) {
		log("[-] SimpleMediaRecorder error %d stopping streams\n", SimpleMediaCode);
		return;
	}

	RecordingTimeUpdateTimer.Stop();

	StartColor.Color(winrt::Microsoft::UI::Colors::LightGreen());

	SenderButton.Content(winrt::box_value(L"Start"));
	SenderButton.Background(StartColor);


	TimerRecordingTimeBlock.Text(L"Recording time: 00:00:00");
	TimerRecordingTimeBlock = { 0 };

	IsRecording = FALSE;
}