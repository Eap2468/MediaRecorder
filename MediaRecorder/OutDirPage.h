#pragma once
#include "pch.h"
#include "App.h"
#include "Config.h"
#include "Logging.h"

winrt::Microsoft::UI::Xaml::Controls::Page InitOutDirPage();
void EditOutDirLocation(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs);
void ShowOutDirInExplorer(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs EventArgs);
