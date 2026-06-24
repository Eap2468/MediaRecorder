#pragma once
/* 
* Most of this header file is taken from this github repo
* which was used as a major reference point to branch from
* https://github.com/sotanakamura/winui3-without-xaml/
*/

#include "pch.h"
#include "InputListPage.h"
#include "OutDirPage.h"
#include "Config.h"
#include "SimpleMediaRecorder.h"
#include "StartStopPage.h"

using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::XamlTypeInfo;
using namespace winrt::Microsoft::UI::Xaml::Markup;
using namespace winrt::Windows::UI::Xaml::Interop;
using namespace winrt::Windows::Foundation;

extern Window MainWindow;

BOOL CloseWindow(HWND hWnd, LPARAM lParam);

class App : public ApplicationT<App, IXamlMetadataProvider> {
public:
	void OnLaunched(LaunchActivatedEventArgs const&);
	void OnClose(winrt::Microsoft::UI::Windowing::AppWindow const& sender, winrt::Microsoft::UI::Windowing::AppWindowClosingEventArgs const& args);
	IXamlType GetXamlType(TypeName const& Type);
	IXamlType GetXamlType(winrt::hstring const& FullName);
	winrt::com_array<XmlnsDefinition> GetXmlnsDefinitions();
private:
	XamlControlsXamlMetaDataProvider provider;
};