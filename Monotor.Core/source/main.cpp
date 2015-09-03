#include "pch.hpp"

#include "HResultException.hpp"
#include "MediaType.hpp"
#include "ComInitializeWrapper.hpp"
#include "Variant.hpp"
#include "RunningObjectTableRegistrar.hpp"
#include "DirectShowHelper.hpp"
#include "DeviceEnumerator.hpp"
#include "PinProxy.hpp"
#include "FilterProxy.hpp"
#include "DebugWindow.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hprevious_instance, _In_ LPWSTR command_line, _In_ int command_show)
{
	using namespace std;
	using namespace Microsoft::WRL;
	using namespace Mntone::ComSupport;
	using namespace Mntone::DirectShowSupport;
	using namespace Mntone::Monotor;

#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HRESULT hr = S_OK;
	ComInitializeWrapper init;

#if _DEBUG
	// デバッグビルド時はウィンドウを立ち上げる
	unique_ptr<DebugWindow> debugWindow;
	debugWindow = make_unique<DebugWindow>();
	hr = debugWindow->initialize(hinstance);
	assert(SUCCEEDED(hr));

	int32_t ret = 0;
	ret = debugWindow->run();
#else
	MessageBoxW(nullptr, L"test", L"stop", MB_OK);
#endif

	return ret;
}