#include "pch.h"
#include "ErrorMessageBox.h"

using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;

winrt::Windows::Foundation::IAsyncAction CreateErrorMessageBox(XamlRoot xamlroot, std::wstring Title, std::wstring ErrorMessage) {
	ContentDialog ErrorDialog;

	ErrorDialog.Title(winrt::box_value(Title.c_str()));
	ErrorDialog.Content(winrt::box_value(ErrorMessage.c_str()));
	ErrorDialog.CloseButtonText(L"Ok");
	ErrorDialog.DefaultButton(ContentDialogButton::Close);
	ErrorDialog.XamlRoot(xamlroot);

	co_await ErrorDialog.ShowAsync();
}