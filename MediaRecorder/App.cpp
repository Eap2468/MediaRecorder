#include "pch.h"
#include "App.h"

Window MainWindow = { 0 };
PSIMPLE_MEDIA_RECORDER Recorder = 0;

BOOL CloseWindow(HWND hWnd, LPARAM lParam) {
	DWORD WindowProcessId;

	if (!GetWindowThreadProcessId(hWnd, &WindowProcessId))
		log("[-] Error %d getting process id for window 0x%llx\n", GetLastError(), (ULONGLONG)hWnd);

	if (WindowProcessId == GetCurrentProcessId()) {
		if (!PostMessageW(hWnd, WM_CLOSE, 0, 0))
			log("[-] Error %d sending window close to window 0x%llx\n", GetLastError(), (ULONGLONG)hWnd);
	}

	return TRUE;
}

void App::OnLaunched(LaunchActivatedEventArgs const&) {
	Application::Current().Resources().MergedDictionaries().Append(XamlControlsResources());
	Grid grid;
	Grid StartStopInputListGrid;
	TitleBar Title;
	TextBlock TitleBlock;
	Page OutDirPage;
	Page StartStopPage;
	Page InputListPage;
	FontIconSource GlyphIcon;
	RowDefinition row1;
	RowDefinition row2;
	RowDefinition row3;
	RowDefinition row4;
	ColumnDefinition column1;
	ColumnDefinition column2;
	ColumnDefinition column3;
	ColumnDefinition column4;
	SIMPLE_MEDIA_EXIT_CODE SimpleMediaCode = SIMPLE_MEDIA_EXIT_SUCCESS;

	if (!InitSession()) {
		log("[-] Error occured during session initialization\n");
		return;
	}

	SimpleMediaCode = CreateSimpleMediaRecorder(&Recorder);
	if (SimpleMediaCode != SIMPLE_MEDIA_EXIT_SUCCESS) {
		log("[-] SimpleMediaRecorder error %d creating recorder structure\n", SimpleMediaCode);
		return;
	}

	MainWindow = Window();
	MainWindow.SystemBackdrop(Media::MicaBackdrop());

	GlyphIcon.Glyph(L"\uF4AA");
	GlyphIcon.FontFamily(Media::FontFamily(L"Segoe MDL2 Assets"));
	GlyphIcon.FontSize(16.0);

	Title.IconSource(GlyphIcon);
	Title.Title(L"Media Recorder");

	TitleBlock.Text(L"Media Recorder");
	TitleBlock.FontSize(32);
	TitleBlock.FontWeight(winrt::Microsoft::UI::Text::FontWeights::Bold());
	TitleBlock.HorizontalAlignment(HorizontalAlignment::Center);

	row1.Height(GridLengthHelper::Auto());
	row2.Height(GridLengthHelper::FromValueAndType(100, GridUnitType::Pixel));
	row3.Height(GridLengthHelper::FromValueAndType(100, GridUnitType::Pixel));
	row4.Height(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));

	column1.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));
	column2.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Auto));
	column3.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));

	grid.RowDefinitions().Append(row1);
	grid.RowDefinitions().Append(row2);
	grid.RowDefinitions().Append(row3);
	grid.RowDefinitions().Append(row4);

	grid.ColumnDefinitions().Append(column1);
	grid.ColumnDefinitions().Append(column2);
	grid.ColumnDefinitions().Append(column3);

	grid.Children().Append(Title);
	Controls::Grid::SetRow(Title, 0);
	Controls::Grid::SetColumnSpan(Title, grid.ColumnDefinitions().Size());

	grid.Children().Append(TitleBlock);
	Controls::Grid::SetRow(TitleBlock, 1);
	Controls::Grid::SetColumn(TitleBlock, 1);

	OutDirPage = InitOutDirPage();
	grid.Children().Append(OutDirPage);
	Controls::Grid::SetRow(OutDirPage, 2);
	Controls::Grid::SetColumn(OutDirPage, 1);

	StartStopPage = InitStartStopPage();

	InputListPage = InitInputListPage();

	StartStopInputListGrid.Children().Append(StartStopPage);
	StartStopInputListGrid.Children().Append(InputListPage);

	grid.Children().Append(StartStopInputListGrid);
	Controls::Grid::SetRow(StartStopInputListGrid, 3);
	Controls::Grid::SetColumn(StartStopInputListGrid, 1);

	MainWindow.ExtendsContentIntoTitleBar(true);
	MainWindow.SetTitleBar(Title);

	MainWindow.Content(grid);
	MainWindow.AppWindow().Closing({ this, &App::OnClose });
	MainWindow.Activate();
}

void App::OnClose(winrt::Microsoft::UI::Windowing::AppWindow const& sender, winrt::Microsoft::UI::Windowing::AppWindowClosingEventArgs const& args) {
	log("Main window closing, sending close messages to all process Windows\n");

	if (CleanupSimpleMediaRecorder(Recorder) == SIMPLE_MEDIA_EXIT_RECORDING_IN_PROGRESS) {
		StopSimpleMediaStream(Recorder);
		CleanupSimpleMediaRecorder(Recorder);
	}

	EnumWindows(CloseWindow, 0);
}

IXamlType App::GetXamlType(TypeName const& Type) {
	return provider.GetXamlType(Type);
}

IXamlType App::GetXamlType(winrt::hstring const& FullName) {
	return provider.GetXamlType(FullName);
}

winrt::com_array<XmlnsDefinition> App::GetXmlnsDefinitions() {
	return provider.GetXmlnsDefinitions();
}