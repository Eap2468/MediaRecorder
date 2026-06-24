#include "pch.h"
#include "OutDirPage.h"

using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml;

extern PSIMPLE_MEDIA_RECORDER Recorder;

Page InitOutDirPage() {
	LSTATUS status = ERROR_SUCCESS;
	Page page;
	Grid grid;
	StackPanel MainPanel;
	StackPanel ButtonPanel;
	Border CurrentOutDirBlockBorder;
	TextBlock CurrentOutDirBlock;
	Button EditButton;
	Button ShowInExplorerButton;
	SolidColorBrush BlackBrush;
	Thickness BorderThickness;
	Thickness BorderPadding;
	Thickness GridMargin;
	LPSTR OutputDirectoryA = 0;
	LPWSTR OutputDirectoryW = 0;
	ULONG OutputDirectorySize = 0;

	status = GetOutputDirectoryA(OutputDirectoryA, OutputDirectorySize, &OutputDirectorySize);
	if (status == ERROR_SUCCESS) {
		OutputDirectoryA = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, OutputDirectorySize);
		if (GetOutputDirectoryA(OutputDirectoryA, OutputDirectorySize, 0) == ERROR_SUCCESS) {
			SetSimpleMediaOutputDirectory(Recorder, OutputDirectoryA);
			HeapFree(GetProcessHeap(), 0, OutputDirectoryA);
		}
	}

	BlackBrush.Color(winrt::Microsoft::UI::Colors::Black());

	CurrentOutDirBlockBorder.BorderBrush(BlackBrush);

	BorderThickness.Top = 2;
	BorderThickness.Bottom = 2;
	BorderThickness.Left = 2;
	BorderThickness.Right = 2;
	CurrentOutDirBlockBorder.BorderThickness(BorderThickness);

	BorderPadding.Top = 5;
	BorderPadding.Bottom = 5;
	BorderPadding.Left = 5;
	BorderPadding.Right = 5;
	CurrentOutDirBlockBorder.Padding(BorderPadding);

	CurrentOutDirBlock.Style(Application::Current().Resources().Lookup(winrt::box_value(L"BodyTextBlockStyle")).as<Style>());
	
	OutputDirectorySize = 0;
	status = GetOutputDirectoryW(OutputDirectoryW, OutputDirectorySize, &OutputDirectorySize);
	if (status == ERROR_SUCCESS) {
		OutputDirectoryW = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, OutputDirectorySize);
		if (GetOutputDirectoryW(OutputDirectoryW, OutputDirectorySize, 0) == ERROR_SUCCESS) {
			CurrentOutDirBlock.Text(OutputDirectoryW);
			HeapFree(GetProcessHeap(), 0, OutputDirectoryW);
		}
	}

	CurrentOutDirBlockBorder.Child(CurrentOutDirBlock);

	EditButton.Style(Application::Current().Resources().Lookup(winrt::box_value(L"AccentButtonStyle")).as<Style>());
	EditButton.HorizontalAlignment(HorizontalAlignment::Left);
	EditButton.Content(winrt::box_value(L"Edit"));
	EditButton.Click(EditOutDirLocation);
	EditButton.Tag(CurrentOutDirBlock);
	
	ShowInExplorerButton.Style(Application::Current().Resources().Lookup(winrt::box_value(L"DefaultButtonStyle")).as<Style>());
	ShowInExplorerButton.Content(winrt::box_value(L"Show in Explorer"));
	ShowInExplorerButton.Click(ShowOutDirInExplorer);
	ShowInExplorerButton.Tag(CurrentOutDirBlock);

	ButtonPanel.Orientation(Orientation::Horizontal);
	ButtonPanel.HorizontalAlignment(HorizontalAlignment::Right);
	ButtonPanel.Spacing(325);
	ButtonPanel.Children().Append(EditButton);
	ButtonPanel.Children().Append(ShowInExplorerButton);

	MainPanel.Orientation(Orientation::Vertical);
	MainPanel.Spacing(5);

	MainPanel.Children().Append(CurrentOutDirBlockBorder);
	MainPanel.Children().Append(ButtonPanel);

	grid.HorizontalAlignment(HorizontalAlignment::Center);
	grid.Children().Append(MainPanel);

	page.Content(grid);
	return page;
}

void EditOutDirLocation(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs) {
	HRESULT result = S_OK;
	SIMPLE_MEDIA_EXIT_CODE SimpleMediaCode = SIMPLE_MEDIA_EXIT_SUCCESS;
	wil::com_ptr<IFileOpenDialog> FileOpenDialog;
	wil::com_ptr<IShellItem> ShellItem;
	wil::unique_cotaskmem_string SelectedDirectory;
	Button EditButton = sender.as<Button>();
	TextBlock CurrentOutDirBlock = EditButton.Tag().as<TextBlock>();
	LPSTR SelectedDirectoryCString = 0;
	DWORD Options = 0;

	log("[+] Edit OutDir button clicked\n");

	result = CoCreateInstance(CLSID_FileOpenDialog, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&FileOpenDialog));
	if (result != S_OK) {
		log("[-] Error 0x%lx creating IFileOpenDialog instance\n", result);
		return;
	}

	FileOpenDialog->GetOptions(&Options);
	FileOpenDialog->SetOptions(Options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);

	result = FileOpenDialog->Show(0);
	if (result != S_OK) {
		log("[-] Error 0x%lx during folder selection\n", result);
		return;
	}

	result = FileOpenDialog->GetResult(&ShellItem);
	if (result != S_OK) {
		log("[-] Error 0x%lx getting selection results\n", result);
		return;
	}

	result = ShellItem->GetDisplayName(SIGDN_FILESYSPATH, &SelectedDirectory);
	if (result != S_OK) {
		log("[-] Error 0x%lx getting directory path string from IShellItem\n", result);
		return;
	}

	SelectedDirectoryCString = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, wcslen(SelectedDirectory.get()) + 1);
	if (SelectedDirectoryCString == 0) {
		log("[-] Error %d allocating memory for selected directory path UTF-8 string\n", GetLastError());
		return;
	}

	if (WideCharToMultiByte(CP_UTF8, 0, SelectedDirectory.get(), -1, SelectedDirectoryCString, wcslen(SelectedDirectory.get()) + 1, 0, 0) == 0) {
		log("[-] Error %d converting selected directory string to UTF-8 string\n", GetLastError());
		HeapFree(GetProcessHeap(), 0, SelectedDirectoryCString);
	}

	SaveOutputDirectory(SelectedDirectoryCString);

	SimpleMediaCode = SetSimpleMediaOutputDirectory(Recorder, SelectedDirectoryCString);
	if (SimpleMediaCode != SIMPLE_MEDIA_EXIT_SUCCESS) {
		log("[-] SimpleMediaRecorder error %d setting output directory\n", SimpleMediaCode);
		return;
	}

	log("[+] New output directory %ws\n", SelectedDirectory.get());
	CurrentOutDirBlock.Text(SelectedDirectory.get());
}

void ShowOutDirInExplorer(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs) {
	log("[+] Show OutDir in explorer button clicked\n");

	/* 
	* When integrating this will use the library as a reference to the current out dir location,
	* just for now to get everything else surounding that working I'm gonna use the path in the
	* text block
	*/
	TextBlock CurrentOutDirBlock = sender.as<Button>().Tag().as<TextBlock>();
	STARTUPINFOW StartInfo = { 0 };
	PROCESS_INFORMATION ProcInfo = { 0 };
	wchar_t Cmdline[1024];

	memset(&StartInfo, 0, sizeof(StartInfo));
	StartInfo.cb = sizeof(StartInfo);

	memset(&ProcInfo, 0, sizeof(ProcInfo));

	swprintf_s(Cmdline, sizeof(Cmdline) / sizeof(Cmdline[0]), L"C:\\Windows\\explorer.exe %ws", CurrentOutDirBlock.Text().c_str());
	
	if (!CreateProcessW(L"C:\\Windows\\explorer.exe", Cmdline, 0, 0, FALSE, 0, 0, 0, &StartInfo, &ProcInfo)) {
		log("[-] Error %d launching explorer instance\n", GetLastError());
	}
	else {
		log("[+] Launched explorer instance to current output directory\n");
		CloseHandle(ProcInfo.hThread);
		CloseHandle(ProcInfo.hProcess);
	}
}