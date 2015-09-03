#pragma once
#include "HResultException.hpp"

namespace Mntone {
namespace DirectShowSupport {

extern wchar_t const* constant_friendly_name;

inline void CreateGraphBuilder(IGraphBuilder** graphBuilder)
{
	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC, IID_IGraphBuilder, reinterpret_cast<void**>(graphBuilder));
	if (FAILED(hr)) throw HResultException(hr);
}

inline void CreateSystemDeviceEnum(ICreateDevEnum** systemDeviceEnum)
{
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC, IID_ICreateDevEnum, reinterpret_cast<void**>(systemDeviceEnum));
	if (FAILED(hr)) throw HResultException(hr);
}

inline void CreateFilterMapper(IFilterMapper2** systemDeviceEnum)
{
	HRESULT hr = CoCreateInstance(CLSID_FilterMapper2, nullptr, CLSCTX_INPROC, IID_IFilterMapper2, reinterpret_cast<void**>(systemDeviceEnum));
	if (FAILED(hr)) throw HResultException(hr);
}

inline void CreateBaseFilter(CLSID const& filterClsid, IBaseFilter** baseFilter)
{
	HRESULT hr = CoCreateInstance(filterClsid, nullptr, CLSCTX_INPROC, IID_IBaseFilter, reinterpret_cast<void**>(baseFilter));
	if (FAILED(hr)) throw HResultException(hr);
}

inline void GetVideoDisplayControl(IBaseFilter* rendererFilter, IMFVideoDisplayControl** videoDisplayControl)
{
	HRESULT hr = S_OK;

	::Microsoft::WRL::ComPtr<IMFGetService> getService;
	hr = rendererFilter->QueryInterface(getService.GetAddressOf());
	if (FAILED(hr)) throw HResultException(hr);

	::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> tempVideoDisplayControl;
	hr = getService->GetService(MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl, &tempVideoDisplayControl);
	if (FAILED(hr)) throw HResultException(hr);

	hr = tempVideoDisplayControl.CopyTo(videoDisplayControl);
	if (FAILED(hr)) throw HResultException(hr);
}

}
}