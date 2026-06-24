#pragma once
#include "pch.h"

winrt::Windows::Foundation::IAsyncAction CreateErrorMessageBox(winrt::Microsoft::UI::Xaml::XamlRoot xamlroot, std::wstring Title, std::wstring ErrorMessage);
