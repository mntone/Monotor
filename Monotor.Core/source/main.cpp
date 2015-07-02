#include "pch.hpp"

#include "DebugWindow.hpp"

namespace DirectShowSupport
{

struct com_exception
{
	HRESULT hresult;

	com_exception(HRESULT hresult) : hresult(hresult) { }
};

inline void CreateSystemDeviceEnum(ICreateDevEnum** systemDeviceEnum)
{
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC, IID_ICreateDevEnum, reinterpret_cast<void**>(systemDeviceEnum));
	if (FAILED(hr)) throw com_exception(hr);
}

inline void CreateEnhancedVideoRenderer(IBaseFilter** enhancedVideoRenderer)
{
	HRESULT hr = CoCreateInstance(CLSID_EnhancedVideoRenderer, nullptr, CLSCTX_INPROC, IID_IBaseFilter, reinterpret_cast<void**>(enhancedVideoRenderer));
	if (FAILED(hr)) throw com_exception(hr);
}

}

int APIENTRY wWinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hprevious_instance, _In_ LPWSTR command_line, _In_ int command_show)
{
	::std::unique_ptr<DebugWindow> debugWindow;

	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
	assert(SUCCEEDED(hr));

	using namespace Microsoft::WRL;
	ComPtr<IGraphBuilder> graphBuilder;
	hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC, IID_IGraphBuilder, &graphBuilder);
	assert(SUCCEEDED(hr));

#if _DEBUG
	ComPtr<IRunningObjectTable> runningObjectTable;
	DWORD registerID;
	{
		hr = GetRunningObjectTable(0, &runningObjectTable);
		assert(SUCCEEDED(hr));

		ComPtr<IMoniker> moniker;
		WCHAR wsz[256];
		wsprintfW(wsz, L"FilterGraph %08x pid %08x", graphBuilder.Get(), GetCurrentProcessId());
		hr = CreateItemMoniker(L"!", wsz, &moniker);
		assert(SUCCEEDED(hr));

		hr = runningObjectTable->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, graphBuilder.Get(), moniker.Get(), &registerID);
		assert(SUCCEEDED(hr));
	}
#endif

	ComPtr<IMediaControl> mediaControl;
	hr = graphBuilder.As(&mediaControl);
	assert(SUCCEEDED(hr));

	ComPtr<ICaptureGraphBuilder2> captureGraphBuilder;
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, &captureGraphBuilder);
	assert(SUCCEEDED(hr));

	hr = captureGraphBuilder->SetFiltergraph(graphBuilder.Get());
	assert(SUCCEEDED(hr));

	ComPtr<ICreateDevEnum> devEnum;
	DirectShowSupport::CreateSystemDeviceEnum(&devEnum);

	ComPtr<IEnumMoniker> enumMoniker;
	hr = devEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumMoniker, 0);
	assert(SUCCEEDED(hr));
	assert(enumMoniker);

	ComPtr<IMoniker> moniker;
	hr = enumMoniker->Next(1, &moniker, nullptr);
	assert(SUCCEEDED(hr));

	ComPtr<IBaseFilter> baseFilter;
	hr = moniker->BindToObject(nullptr, nullptr, IID_IBaseFilter, &baseFilter);
	assert(SUCCEEDED(hr));

	hr = graphBuilder->AddFilter(baseFilter.Get(), L"Source");
	assert(SUCCEEDED(hr));

	ComPtr<IBaseFilter> rendererFilter;
	DirectShowSupport::CreateEnhancedVideoRenderer(&rendererFilter);

	hr = graphBuilder->AddFilter(rendererFilter.Get(), L"Renderer");
	assert(SUCCEEDED(hr));

	hr = captureGraphBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, nullptr, baseFilter.Get(), nullptr, rendererFilter.Get());
	assert(SUCCEEDED(hr));

	hr = mediaControl->Run();
	assert(SUCCEEDED(hr));

	// Set window handle
	{
		ComPtr<IMFGetService> getService;
		hr = rendererFilter.As(&getService);
		assert(SUCCEEDED(hr));

		ComPtr<IMFVideoDisplayControl> videoDisplayControl;
		hr = getService->GetService(MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, &videoDisplayControl);
		assert(SUCCEEDED(hr));

		HWND hwnd = (HWND)3345806;
		hr = videoDisplayControl->SetVideoWindow(hwnd);
#if _DEBUG
		// デバッグビルド時はウィンドウを立ち上げるなどする
		// 通信用にダミーウィンドウハンドルを作ってそれを使ってP2P?
		// WndProc は C# 側で監視させるのが得策かも。変更通知はダミーウィンドウで受け取る。
		if (FAILED(hr))
		{
			debugWindow = ::std::make_unique<DebugWindow>(videoDisplayControl);
			hr = debugWindow->initialize(hinstance);
			assert(SUCCEEDED(hr));
		}
#else
		assert(SUCCEEDED(hr));
#endif

		MFVideoNormalizedRect mvnr = { 0, 0, 1, 1 };
		RECT rect;
		GetClientRect(hwnd, &rect);
		hr = videoDisplayControl->SetVideoPosition(&mvnr, &rect);
		assert(SUCCEEDED(hr));
	}

	int32_t ret = 0;
	if (debugWindow)
	{
		ret = debugWindow->run();
	}
	else
	{
		MessageBoxW(nullptr, L"test", L"stop", MB_OK);
	}

#if _DEBUG
	hr = runningObjectTable->Revoke(registerID);
	assert(SUCCEEDED(hr));
#endif

	CoUninitialize();
	return ret;
}