#include "pch.hpp"
#include "GraphBuilder.hpp"

#include "DeviceEnumerator.hpp"
#include "HResultException.hpp"
#include "AppException.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace Mntone::DirectShowSupport;
using namespace Mntone::Monotor;

void GraphBuilder::Run()
{
	HRESULT hr = graphBuilderProxy_.MediaControl()->Run();
	if (FAILED(hr)) throw HResultException(hr);
}

FilterProxy GraphBuilder::ConnectForVideo(wstring deviceName, MediaType& mediaType)
{
	HRESULT hr = S_OK;

	ComPtr<IMoniker> sourceMoniker;
	hr = enumrator_.TryGetVideoInputDeviceMoniker(deviceName, &sourceMoniker);
	if (FAILED(hr)) throw HResultException(hr);

	auto sourceFilter = FilterProxy::CreateInsntanceFromMoniker(sourceMoniker.Get(), deviceName);
	graphBuilderProxy_.Add(sourceFilter);

	auto sourceOutputPins = sourceFilter.GetOutputPins();
	if (sourceOutputPins.size() == 0) throw AppException(L"Cannot find Output Pin in Source Filter.");

	auto smartTeeFilter = FilterProxy::CreateInstanceFromClsid(CLSID_SmartTee, L"Smart Tee");
	graphBuilderProxy_.Add(smartTeeFilter);

	auto smartTeeInputPins = smartTeeFilter.GetInputPins();
	if (smartTeeInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in SmartTee Filter.");

	hr = graphBuilderProxy_->ConnectDirect(sourceOutputPins[0], smartTeeInputPins[0], mediaType.mt_);
	if (FAILED(hr)) throw HResultException(hr);

	if (mediaType.GetFourCC() == L"MJPG")
	{
		auto lastTransformFilter = ConnectForMjpeg(smartTeeFilter, mediaType);
		return ConnectForEvrRenderer(lastTransformFilter);
	}
	else
	{
		throw AppException(L"Unknown format.");
	}
}

FilterProxy GraphBuilder::ConnectForMjpeg(FilterProxy& sourceFilter, MediaType& mediaType)
{
	HRESULT hr = S_OK;

	auto smartTeeOutputPins = sourceFilter.GetOutputPins();
	if (smartTeeOutputPins.size() <= 1) throw AppException(L"Cannot find Output Pin in SmartTee Filter.");

	auto smartTeePreviewOutputPin = smartTeeOutputPins[0];

	auto mjpegDecoderFilter = FilterProxy::CreateInstanceFromClsid(CLSID_MjpegDec, L"MJPEG Decoder");
	graphBuilderProxy_.Add(mjpegDecoderFilter);

	auto mjpegDecoderInputPins = mjpegDecoderFilter.GetInputPins();
	if (mjpegDecoderInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in Mjpeg Decoder Filter.");

	hr = graphBuilderProxy_->ConnectDirect(smartTeePreviewOutputPin, mjpegDecoderInputPins[0], mediaType.mt_);
	if (FAILED(hr)) throw HResultException(hr);

	auto mjpegDecoderOutputPins = mjpegDecoderFilter.GetOutputPins();
	if (mjpegDecoderOutputPins.size() == 0) throw AppException(L"Cannot find Output Pin in Mjpeg Decoder Filter.");

	auto mjpegDecoderTargetOutputPin = mjpegDecoderOutputPins[0];
	auto mjpegDecoderTargetOutputPinMediaTypes = mjpegDecoderTargetOutputPin.GetPreferedMediaType();
	if (mjpegDecoderTargetOutputPinMediaTypes.size() == 0) throw AppException(L"Cannot get MediaType for Output Pin in Mjpeg Decoder Filter.");

	auto colorSpaceConverterFilter = FilterProxy::CreateInstanceFromClsid(CLSID_Colour, L"Color Space Converter");
	graphBuilderProxy_.Add(colorSpaceConverterFilter);

	auto colorSpaceConverterInputPins = colorSpaceConverterFilter.GetInputPins();
	if (colorSpaceConverterInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in Color Space Converter Filter.");

	hr = graphBuilderProxy_->ConnectDirect(mjpegDecoderTargetOutputPin, colorSpaceConverterInputPins[0], mjpegDecoderTargetOutputPinMediaTypes[0].mt_);
	if (FAILED(hr)) throw HResultException(hr);

	return colorSpaceConverterFilter;
}

FilterProxy GraphBuilder::ConnectForEvrRenderer(FilterProxy& lastTransformFilter)
{
	auto lastTransformOutputPins = lastTransformFilter.GetOutputPins();
	if (lastTransformOutputPins.size() == 0) throw AppException(L"Cannot find Output Pin in Last Transform Filter.");

	auto lastTransformTargetOutputPin = lastTransformOutputPins[0];
	auto lastTransformTargetOutputPintMediaTypes = lastTransformTargetOutputPin.GetPreferedMediaType();
	if (lastTransformTargetOutputPintMediaTypes.size() == 0) throw AppException(L"Cannot get MediaType for Output Pin in Last Transform Filter.");

	auto rendererFilter = FilterProxy::CreateInstanceFromClsid(CLSID_EnhancedVideoRenderer, L"Enhanced Video Renderer");
	graphBuilderProxy_.Add(rendererFilter);

	auto rendererInputPins = rendererFilter.GetInputPins();
	if (rendererInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in EVR Filter.");

	HRESULT hr = graphBuilderProxy_->ConnectDirect(lastTransformTargetOutputPin, rendererInputPins[0], lastTransformTargetOutputPintMediaTypes[0].mt_);
	if (FAILED(hr)) throw HResultException(hr);

	return rendererFilter;
}