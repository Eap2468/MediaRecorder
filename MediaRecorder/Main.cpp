#include "pch.h"
#include "App.h"
#include "Logging.h"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
	InitializeLogging();
	init_apartment();
	Application::Start([](auto&&) { make<App>(); });
	
	return 0;
}