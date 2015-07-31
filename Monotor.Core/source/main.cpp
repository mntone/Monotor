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

namespace Mntone {
namespace DirectShowSupport {

inline void OutputMediaType(::std::vector<MediaType> const& mts)
{
	using namespace std;

	int i = 0;
	for (auto&& mt : mts)
	{
		wcdbg << L'[' << i++ << L"] ";
		if (mt.IsVideo())
		{
			wcdbg << L"[Video] " << mt.SubTypeAsString() << L' ';

			if (mt.IsVideoInfoHeader())
			{
				auto& vi = mt.AsVideoInfoHeader();
				char fourcc[5] = { 0 };
				if (vi.bmiHeader.biCompression == BI_RGB)
				{
					memcpy(fourcc, "RGB", 3);
				}
				else
				{
					memcpy(fourcc, &vi.bmiHeader.biCompression, 4);
				}
				wcdbg
					<< vi.bmiHeader.biWidth << L'x' << vi.bmiHeader.biHeight << L", "
					<< vi.bmiHeader.biBitCount << L" bit, "
					<< (10000000.0 / static_cast<double>(vi.AvgTimePerFrame)) << L" fps, "
					<< fourcc;
			}
			else if (mt.IsVideoInfoHeader2())
			{
				auto& vi = mt.AsVideoInfoHeader2();
				char fourcc[5] = { 0 };
				if (vi.bmiHeader.biCompression == BI_RGB)
				{
					memcpy(fourcc, "RGB", 3);
				}
				else
				{
					memcpy(fourcc, &vi.bmiHeader.biCompression, 4);
				}
				wcdbg
					<< vi.bmiHeader.biWidth << L'x' << vi.bmiHeader.biHeight << L", "
					<< vi.bmiHeader.biBitCount << L" bit, "
					<< (10000000.0 / static_cast<double>(vi.AvgTimePerFrame)) << L" fps, "
					<< vi.dwPictAspectRatioX << L':' << vi.dwPictAspectRatioY << L", "
					<< fourcc;
			}

			wcdbg << endl;
		}
		else if (mt.IsAudio())
		{
			wcdbg << L"[Audio] ";
			if (mt.mt_->subtype == MEDIASUBTYPE_PCM) wcdbg << L"PCM ";

			if (mt.IsWaveFormatEx())
			{
				auto& wf = mt.AsWaveFormatEx();
				wcdbg << wf.nChannels << L" ch, " << wf.nSamplesPerSec << L" Hz, " << wf.wBitsPerSample << " bit";
			}

			wcdbg << endl;
		}
		else
		{
			wcdbg << "[Other] " << endl;
		}
	}
}

}
}

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