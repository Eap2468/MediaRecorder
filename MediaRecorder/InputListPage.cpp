#include "pch.h"
#include "InputListPage.h"

using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml;

extern PSIMPLE_MEDIA_RECORDER Recorder;

void LaunchAddInputWindow(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
void RemoveInput(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

ListView InputList = { 0 };

Page InitInputListPage() {
	SIMPLE_MEDIA_EXIT_CODE SimpleMediaCode = SIMPLE_MEDIA_EXIT_SUCCESS;
	PMEDIA_RECORDER_SESSION_INFO SavedSessionInfo = 0;
	Page page;
	StackPanel MainPanel;
	StackPanel ButtonPanel;
	TextBlock RegisteredInputsBlock;
	TextBlock SavedListEntryBlock = { 0 };
	Button AddButton;
	Button RemoveButton;
	Thickness ButtonPanelMargin;
	Thickness MainPanelSpacing;
	LPWSTR SavedName = 0;
	LPWSTR SavedIp = 0;
	wchar_t EntryInfo[1024];

	RegisteredInputsBlock.Text(L"Registered Inputs:");

	InputList = ListView();
	if (GetInputListFromConfig(&SavedSessionInfo) == ERROR_SUCCESS) {
		for (ULONG i = 0; i < SavedSessionInfo->NumberOfEntries; i++) {
			SimpleMediaCode = AddSimpleMediaInput(Recorder, SavedSessionInfo->Entries[i].Name,
				SavedSessionInfo->Entries[i].Ip, SavedSessionInfo->Entries[i].Port,
				SavedSessionInfo->Entries[i].Username, SavedSessionInfo->Entries[i].Password, 0);
			if (SimpleMediaCode != SIMPLE_MEDIA_EXIT_SUCCESS) {
				log("[-] SimpleMediaError %d adding saved input %s to recorder\n", SimpleMediaCode, SavedSessionInfo->Entries[i].Name);
				continue;
			}

			SavedName = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (strlen(SavedSessionInfo->Entries[i].Name) * 2) + 2);
			if (SavedName == 0) {
				log("[-] Error %d allocating memory for saved entry name %s\n", GetLastError(), SavedSessionInfo->Entries[i].Name);
				continue;
			}

			if (MultiByteToWideChar(CP_ACP, 0, SavedSessionInfo->Entries[i].Name, -1, SavedName, (strlen(SavedSessionInfo->Entries[i].Name) * 2) + 2) == 0) {
				log("[-] Error %d converting saved input name %s to wide character string\n", GetLastError(), SavedSessionInfo->Entries[i].Name);
				HeapFree(GetProcessHeap(), 0, SavedName);
				continue;
			}

			SavedIp = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (strlen(SavedSessionInfo->Entries[i].Ip) * 2) + 2);
			if (SavedIp == 0) {
				log("[-] Error %d allocating memory for saved entry IP %s\n", GetLastError(), SavedSessionInfo->Entries[i].Ip);
				HeapFree(GetProcessHeap(), 0, SavedName);
				continue;
			}

			if (MultiByteToWideChar(CP_ACP, 0, SavedSessionInfo->Entries[i].Ip, -1, SavedIp, (strlen(SavedSessionInfo->Entries[i].Ip) * 2) + 2) == 0) {
				log("[-] Error %d converting saved input IP %s to wide character string\n", GetLastError(), SavedSessionInfo->Entries[i].Ip);
				HeapFree(GetProcessHeap(), 0, SavedName);
				HeapFree(GetProcessHeap(), 0, SavedIp);
				continue;
			}

			SavedListEntryBlock = TextBlock();
			swprintf_s(EntryInfo, sizeof(EntryInfo) / sizeof(EntryInfo[0]), L"%s: rtsp://%s:%d", SavedName, SavedIp, SavedSessionInfo->Entries[i].Port);
			SavedListEntryBlock.Text(EntryInfo);

			InputList.Items().Append(SavedListEntryBlock);

			HeapFree(GetProcessHeap(), 0, SavedName);
			HeapFree(GetProcessHeap(), 0, SavedIp);
		}
		CleanupInputList(SavedSessionInfo);
	}

	AddButton.Content(winrt::box_value(L"Add"));
	AddButton.Style(Application::Current().Resources().Lookup(winrt::box_value(L"DefaultButtonStyle")).as<Style>());
	AddButton.Click(LaunchAddInputWindow);

	RemoveButton.Content(winrt::box_value(L"Remove"));
	RemoveButton.Style(Application::Current().Resources().Lookup(winrt::box_value(L"DefaultButtonStyle")).as<Style>());
	RemoveButton.Click(RemoveInput);

	ButtonPanelMargin.Top = 0;
	ButtonPanelMargin.Bottom = 0;
	ButtonPanelMargin.Left = 4;
	ButtonPanelMargin.Right = 0;

	ButtonPanel.Orientation(Orientation::Horizontal);
	ButtonPanel.Margin(ButtonPanelMargin);
	ButtonPanel.Spacing(170);
	ButtonPanel.Children().Append(AddButton);
	ButtonPanel.Children().Append(RemoveButton);

	MainPanel.Orientation(Orientation::Vertical);
	MainPanel.HorizontalAlignment(HorizontalAlignment::Right);
	MainPanel.Width(300);
	MainPanel.Children().Append(RegisteredInputsBlock);
	MainPanel.Children().Append(InputList);
	MainPanel.Children().Append(ButtonPanel);

	page.Content(MainPanel);
	return page;
}

void AddInput(std::wstring Name, std::wstring Ip, USHORT Port, std::wstring Username, std::wstring Password) {
	/* Adding input to library and config happens here, for now just gonna do the UI portion */
	SIMPLE_MEDIA_EXIT_CODE SimpleMediaCode = SIMPLE_MEDIA_EXIT_SUCCESS;
	TextBlock EntryBlock;
	wchar_t EntryInfo[1024];
	char* NameString = 0;
	char* IpString = 0;
	char* UsernameString = 0;
	char* PasswordString = 0;

	NameString = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Name.length() + 1);
	if (NameString == 0) {
		log("[-] Error %d allocating memory for name string\n", GetLastError());
		return;
	}

	if (WideCharToMultiByte(CP_UTF8, 0, Name.c_str(), -1, NameString, Name.length() + 1, 0, 0) == 0) {
		log("[-] Error %d converting name string to UTF-8 string\n", GetLastError());
		goto cleanup;
	}

	IpString = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Ip.length() + 1);
	if (IpString == 0) {
		log("[-] Error %d allocating memory for ip string\n", GetLastError());
		goto cleanup;
	}
	if (WideCharToMultiByte(CP_UTF8, 0, Ip.c_str(), -1, IpString, Ip.length() + 1, 0, 0) == 0) {
		log("[-] Error %d converting ip string to UTF-8 string\n", GetLastError());
		goto cleanup;
	}

	UsernameString = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Username.length() + 1);
	if (UsernameString == 0) {
		log("[-] Error %d allocating memory for username string\n", GetLastError());
		goto cleanup;
	}
	if (WideCharToMultiByte(CP_UTF8, 0, Username.c_str(), -1, UsernameString, Username.length() + 1, 0, 0) == 0) {
		log("[-] Error %d converting username string to UTF-8 string\n", GetLastError());
		goto cleanup;
	}

	PasswordString = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Password.length() + 1);
	if (PasswordString == 0) {
		log("[-] Error %d allocating memory for password string\n", GetLastError());
		goto cleanup;
	}
	if (WideCharToMultiByte(CP_UTF8, 0, Password.c_str(), -1, PasswordString, Password.length() + 1, 0, 0) == 0) {
		log("[-] Error %d converting password string to UTF-8 string\n", GetLastError());
		goto cleanup;
	}

	SaveInputToConfig(NameString, IpString, Port, UsernameString, PasswordString);

	SimpleMediaCode = AddSimpleMediaInput(Recorder, NameString, IpString, Port, UsernameString, PasswordString, 0);
	if (SimpleMediaCode != SIMPLE_MEDIA_EXIT_SUCCESS) {
		log("[-] Error %d creating simple media input\n", SimpleMediaCode);
		goto cleanup;
	}

	swprintf_s(EntryInfo, sizeof(EntryInfo) / sizeof(EntryInfo[0]), L"%s: rtsp://%s:%d", Name.c_str(), Ip.c_str(), Port);
	EntryBlock.Text(EntryInfo);

	InputList.Items().Append(EntryBlock);

cleanup:
	if (NameString)
		HeapFree(GetProcessHeap(), 0, NameString);
	if (IpString)
		HeapFree(GetProcessHeap(), 0, IpString);
	if (UsernameString)
		HeapFree(GetProcessHeap(), 0, UsernameString);
	if (PasswordString)
		HeapFree(GetProcessHeap(), 0, PasswordString);
}

void AddInputButtonCallback(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) {
	Button ApplyButton = sender.as<Button>();
	StackPanel MainPanel = ApplyButton.Parent().as<StackPanel>();
	TextBox NameBox = MainPanel.Children().GetAt(0).as<TextBox>();
	TextBox IpBox = MainPanel.Children().GetAt(1).as<TextBox>();
	TextBox PortBox = MainPanel.Children().GetAt(2).as<TextBox>();
	TextBox UsernameBox = MainPanel.Children().GetAt(3).as<TextBox>();
	TextBox PasswordBox = MainPanel.Children().GetAt(4).as<TextBox>();
	std::wstring Name;
	std::wstring IpString;
	std::wstring PortString;
	std::wstring Username;
	std::wstring Password;
	IN_ADDR Ip;
	ULONGLONG Port = 0;

	Name = NameBox.Text().c_str();
	IpString = IpBox.Text().c_str();
	PortString = PortBox.Text().c_str();
	Username = UsernameBox.Text().c_str();
	Password = PasswordBox.Text().c_str();

	/* Need to add a check for conflicting names */
	if (wcslen(Name.c_str()) == 0) {
		CreateErrorMessageBox(MainPanel.XamlRoot(), L"Add Input Error", L"No input name specified");
		return;
	}
	else if (wcschr(Name.c_str(), L':') != 0 ||
			wcschr(Name.c_str(), L'\\') != 0 ||
			wcschr(Name.c_str(), L'/') != 0 ||
			wcschr(Name.c_str(), L'"') != 0 ||
			wcschr(Name.c_str(), L'\'') != 0 ||
			wcschr(Name.c_str(), L'?') != 0 ||
			wcschr(Name.c_str(), L'*') != 0 ||
			wcschr(Name.c_str(), L'<') != 0 ||
			wcschr(Name.c_str(), L'>') != 0) {
		CreateErrorMessageBox(MainPanel.XamlRoot(), L"Add Input Error", L"Invalid character in name");
		return;
	}
	else if (wcslen(Name.c_str()) > 80) {
		CreateErrorMessageBox(MainPanel.XamlRoot(), L"Add Input Error", L"Input name too long, maximum 80 characters");
		return;
	}
	else if (wcslen(IpString.c_str()) == 0) {
		CreateErrorMessageBox(MainPanel.XamlRoot(), L"Add Input Error", L"No IP address specified");
		return;
	}
	else if (!InetPtonW(AF_INET, IpString.c_str(), &Ip)) {
		CreateErrorMessageBox(MainPanel.XamlRoot(), L"Add Input Error", L"Invalid IP address specified");
		return;
	}
	
	if (wcslen(PortString.c_str())) {
		Port = _wtoi64(PortString.c_str());
		if (Port > 65535) {
			CreateErrorMessageBox(MainPanel.XamlRoot(), L"Add Input Error", L"Invalid port specified, must be between 1 and 65535");
			return;
		}
	}
	else {
		Port = 554;
	}

	log("[*] Add input message sent for name: %ws, ip: %ws, port: %d, username: %ws, password: %ws\n", Name.c_str(), IpString.c_str(), Port, Username.c_str(), Password.c_str());
	
	AddInput(Name, IpString, (USHORT)Port, Username, Password);

	ApplyButton.Tag().as<Window>().Close();
}

void AddInputWindowEnterHandler(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e) {
	TextBox SenderBox = sender.as<TextBox>();
	if (e.Key() == winrt::Windows::System::VirtualKey::Enter) {
		AddInputButtonCallback(SenderBox.Tag().as<Button>(), winrt::Microsoft::UI::Xaml::RoutedEventArgs());
		e.Handled(true);
	}
}

void LaunchAddInputWindow(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) {
	log("Add input button clicked\n");

	Window AddInputWindow;
	StackPanel MainPanel;
	TextBox NameBox;
	TextBox IpBox;
	TextBox PortBox;
	TextBox UsernameBox;
	TextBox PasswordBox;
	Button ApplyButton;
	winrt::Windows::Graphics::SizeInt32 WindowSize;

	NameBox.PlaceholderText(L"Name");
	NameBox.Style(Application::Current().Resources().Lookup(winrt::box_value(L"DefaultTextBoxStyle")).as<Style>());
	NameBox.KeyDown(AddInputWindowEnterHandler);
	NameBox.Tag(ApplyButton);
	
	IpBox.PlaceholderText(L"IP");
	IpBox.KeyDown(AddInputWindowEnterHandler);
	IpBox.Tag(ApplyButton);

	PortBox.PlaceholderText(L"Port (optional, default is 554)");
	PortBox.KeyDown(AddInputWindowEnterHandler);
	PortBox.Tag(ApplyButton);

	UsernameBox.PlaceholderText(L"Username (optional)");
	UsernameBox.KeyDown(AddInputWindowEnterHandler);
	UsernameBox.Tag(ApplyButton);
	
	PasswordBox.PlaceholderText(L"Password (optional)");
	PasswordBox.KeyDown(AddInputWindowEnterHandler);
	PasswordBox.Tag(ApplyButton);

	ApplyButton.Content(winrt::box_value(L"Apply"));
	ApplyButton.Style(Application::Current().Resources().Lookup(winrt::box_value(L"AccentButtonStyle")).as<Style>());
	ApplyButton.Click(AddInputButtonCallback);
	ApplyButton.Tag(AddInputWindow);

	MainPanel.Orientation(Orientation::Vertical);
	MainPanel.Spacing(5);
	MainPanel.Children().Append(NameBox);
	MainPanel.Children().Append(IpBox);
	MainPanel.Children().Append(PortBox);
	MainPanel.Children().Append(UsernameBox);
	MainPanel.Children().Append(PasswordBox);
	MainPanel.Children().Append(ApplyButton);

	WindowSize.Height = 400;
	WindowSize.Width = 400;
	AddInputWindow.AppWindow().Resize(WindowSize);
	AddInputWindow.SystemBackdrop(Media::MicaBackdrop());
	AddInputWindow.Content(MainPanel);
	AddInputWindow.Activate();

	log("Add window closed\n");
}

void RemoveInput(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) {
	SIMPLE_MEDIA_EXIT_CODE SimpleMediaCode = SIMPLE_MEDIA_EXIT_SUCCESS;
	TextBlock SelectedTextBlock = { 0 };
	LPSTR SelectedName = 0;
	wchar_t ErrorBuffer[1024];
	
	log("Remove input button clicked\n");
	
	if (InputList.SelectedItem() != 0) {
		SelectedTextBlock = InputList.Items().GetAt(InputList.SelectedIndex()).as<TextBlock>();

		SelectedName = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, wcslen(SelectedTextBlock.Text().c_str()) + 1);
		if (SelectedName == 0) {
			log("[-] Error %d allocating memory for selected text name UTF-8 string\n", GetLastError());
			CreateErrorMessageBox(InputList.XamlRoot(), L"Remove Input Internal Error", L"Unable to allocate memory for SelectedName string");
			return;
		}

		if (WideCharToMultiByte(CP_UTF8, 0, SelectedTextBlock.Text().c_str(), -1, SelectedName, wcslen(SelectedTextBlock.Text().c_str()) + 1, 0, 0) == 0) {
			log("[-] Error %d converting selected text block name to UTF-8 string\n", GetLastError());
			CreateErrorMessageBox(InputList.XamlRoot(), L"Remove Input Internal Error", L"Unable to convert selected text block text to UTF-8 string");
			
			HeapFree(GetProcessHeap(), 0, SelectedName);
			return;
		}

		for (ULONG i = 0; i < strlen(SelectedName); i++) {
			if (SelectedName[i] == ':') {
				SelectedName[i] = 0;
				break;
			}
		}

		SimpleMediaCode = RemoveSimpleMediaInputByName(Recorder, SelectedName);
		
		if (SimpleMediaCode != SIMPLE_MEDIA_EXIT_SUCCESS) {
			log("[-] SimpleMediaRecorder error %d removing input by name\n", SimpleMediaCode);

			memset(ErrorBuffer, 0, sizeof(ErrorBuffer));

			if (SimpleMediaCode == SIMPLE_MEDIA_EXIT_INPUT_NOT_FOUND)
				swprintf_s(ErrorBuffer, sizeof(ErrorBuffer) / sizeof(ErrorBuffer[0]), L"SimpleMediaRecorder input not found");
			else if (SimpleMediaCode == SIMPLE_MEDIA_EXIT_RECORDING_IN_PROGRESS)
				swprintf_s(ErrorBuffer, sizeof(ErrorBuffer) / sizeof(ErrorBuffer[0]), L"SimpleMediaRecorder input is currently recording and must be stopped before removing");
			else
				swprintf_s(ErrorBuffer, sizeof(ErrorBuffer) / sizeof(ErrorBuffer[0]), L"SimpleMediaRecorder error %d removing media input", SimpleMediaCode);

			CreateErrorMessageBox(InputList.XamlRoot(), L"Remove Input Error", ErrorBuffer);
			HeapFree(GetProcessHeap(), 0, SelectedName);
			return;
		}

		RemoveInputFromConfig(SelectedName);
		InputList.Items().RemoveAt(InputList.SelectedIndex());
		HeapFree(GetProcessHeap(), 0, SelectedName);
	}
}