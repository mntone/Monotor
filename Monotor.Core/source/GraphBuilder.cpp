#include "pch.hpp"
#include "GraphBuilder.hpp"

#include "../../AudioMixDownFilter/AudioMixDownFilter_h.h"
#include "../../AudioMixDownFilter/AudioMixDownFilter_i.c"
#include "../../AudioMixDownFilter/include/AudioMixDownFilterDefine.hpp"

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

	auto smartTeeFilter = ConnectSmartTee(sourceFilter, mediaType);

	if (mediaType.GetFourCC() == "MJPG")
	{
		auto lastTransformFilter = ConnectForMjpeg(smartTeeFilter, mediaType);
		return ConnectEvrRenderer(lastTransformFilter);
	}	
	else if (mediaType.GetFourCC() == "YUY2" || mediaType.GetFourCC() == "UYVY" || mediaType.GetFourCC() == "HDYC")
	{
		auto lastTransformFilter = ConnectForYuv(smartTeeFilter, mediaType);
		return ConnectEvrRenderer(lastTransformFilter);
	}
	else
	{
		throw AppException(L"Unknown format.");
	}
}

FilterProxy GraphBuilder::ConnectSmartTee(FilterProxy& sourceFilter, MediaType& mediaType)
{
	HRESULT hr = S_OK;

	auto sourceOutputPins = sourceFilter.GetOutputPins();
	if (sourceOutputPins.size() == 0) throw AppException(L"Cannot find Output Pin in Source Filter.");

	auto smartTeeFilter = FilterProxy::CreateInstanceFromClsid(CLSID_SmartTee, L"Smart Tee");
	graphBuilderProxy_.Add(smartTeeFilter);

	auto smartTeeInputPins = smartTeeFilter.GetInputPins();
	if (smartTeeInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in SmartTee Filter.");
	
	hr = graphBuilderProxy_->ConnectDirect(sourceOutputPins[0], smartTeeInputPins[0], mediaType.mt_);
	if (FAILED(hr)) throw HResultException(hr);

	return smartTeeFilter;
}

PinProxy GraphBuilder::GetSmartTeePreviewOrDefault(FilterProxy& targetFilter)
{
	if (targetFilter.Name() == L"Smart Tee")
	{
		auto smartTeeOutputPins = targetFilter.GetOutputPins();
		if (smartTeeOutputPins.size() <= 1) throw AppException(L"Cannot find Output Pin in SmartTee Filter.");

		return smartTeeOutputPins[1];
	}

	auto targetOutputPins = targetFilter.GetOutputPins();
	if (targetOutputPins.size() == 0) throw AppException(L"Cannot find Output Pin in Target Filter.");

	return targetOutputPins[0];
}

FilterProxy GraphBuilder::ConnectForMjpeg(FilterProxy& sourceFilter, MediaType& mediaType)
{
	HRESULT hr = S_OK;

	auto targetOutputPin = GetSmartTeePreviewOrDefault(sourceFilter);

	auto mjpegDecoderFilter = FilterProxy::CreateInstanceFromClsid(CLSID_MjpegDec, L"MJPEG Decoder");
	graphBuilderProxy_.Add(mjpegDecoderFilter);

	auto mjpegDecoderInputPins = mjpegDecoderFilter.GetInputPins();
	if (mjpegDecoderInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in Mjpeg Decoder Filter.");

	hr = graphBuilderProxy_->ConnectDirect(targetOutputPin, mjpegDecoderInputPins[0], mediaType.mt_);
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

FilterProxy GraphBuilder::ConnectForYuv(FilterProxy& sourceFilter, MediaType& mediaType)
{
	HRESULT hr = S_OK;

	auto targetOutputPin = GetSmartTeePreviewOrDefault(sourceFilter);

	auto aviDecoderFilter = FilterProxy::CreateInstanceFromClsid(CLSID_AVIDec, L"AVI Decompressor");
	graphBuilderProxy_.Add(aviDecoderFilter);

	auto aviDecoderInputPins = aviDecoderFilter.GetInputPins();
	if (aviDecoderInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in AVI Decompressor Filter.");

	hr = graphBuilderProxy_->ConnectDirect(targetOutputPin, aviDecoderInputPins[0], mediaType.mt_);
	if (FAILED(hr)) throw HResultException(hr);

	return aviDecoderFilter;
}

FilterProxy GraphBuilder::ConnectEvrRenderer(FilterProxy& lastTransformFilter)
{
	auto lastTransformOutputPins = lastTransformFilter.GetOutputPins();
	if (lastTransformOutputPins.size() == 0) throw AppException(L"Cannot find Output Pin in Last Transform Filter.");

	auto lastTransformTargetOutputPin = lastTransformOutputPins[0];
	auto lastTransformTargetOutputPinMediaTypes = lastTransformTargetOutputPin.GetPreferedMediaType();
	if (lastTransformTargetOutputPinMediaTypes.size() == 0) throw AppException(L"Cannot get MediaType for Output Pin in Last Transform Filter.");

	auto rendererFilter = FilterProxy::CreateInstanceFromClsid(CLSID_EnhancedVideoRenderer, L"Enhanced Video Renderer");
	graphBuilderProxy_.Add(rendererFilter);

	auto rendererInputPins = rendererFilter.GetInputPins();
	if (rendererInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in EVR Filter.");

	HRESULT hr = S_OK;
	for (uint8_t i = 0; i < lastTransformTargetOutputPinMediaTypes.size(); ++i)
	{
		hr = graphBuilderProxy_->ConnectDirect(lastTransformTargetOutputPin, rendererInputPins[0], lastTransformTargetOutputPinMediaTypes[i].mt_);
		if (SUCCEEDED(hr)) break;
	}
	if (FAILED(hr)) throw HResultException(hr);

	return rendererFilter;
}


FilterProxy GraphBuilder::ConnectForAudio(wstring deviceName, MediaType& mediaType)
{
	HRESULT hr = S_OK;

	if (!mediaType.IsWaveFormatEx()) throw AppException(L"Invalid media type.");
	auto wf = mediaType.AsWaveFormatEx();

	ComPtr<IMoniker> sourceMoniker;
	hr = enumrator_.TryGetAudioInputDeviceMoniker(deviceName, &sourceMoniker);
	if (FAILED(hr)) throw HResultException(hr);

	auto sourceFilter = FilterProxy::CreateInsntanceFromMoniker(sourceMoniker.Get(), deviceName);
	graphBuilderProxy_.Add(sourceFilter);

	auto smartTeeFilter = ConnectSmartTee(sourceFilter, mediaType);

	if (wf.nChannels == 6)
	{
		auto lastTransformFilter = ConnectForSurround(smartTeeFilter, mediaType);
		return ConnectWaveOutRenderer(smartTeeFilter);
	}
	else if (wf.nChannels == 2 || wf.nChannels == 1)
	{
		return ConnectWaveOutRenderer(smartTeeFilter);
	}
	else
	{
		throw AppException(L"Unknown format.");
	}
}

FilterProxy GraphBuilder::ConnectForSurround(FilterProxy& sourceFilter, MediaType& mediaType)
{
	HRESULT hr = S_OK;

	auto targetOutputPin = GetSmartTeePreviewOrDefault(sourceFilter);

	filterEnumrator_.SetMajorType(MEDIATYPE_Audio);
	filterEnumrator_.SetMajorType(GUID_NULL);
	filterEnumrator_.SetMajorType(GUID_NULL);

	ComPtr<IMoniker> mixDownMoniker;
	hr = filterEnumrator_.TryGetDeviceMoniker(NAME_AudioMixDownFilter, &mixDownMoniker);
	if (FAILED(hr)) throw HResultException(hr);

	auto mixDownFilter = FilterProxy::CreateInsntanceFromMoniker(mixDownMoniker.Get(), L"Mix-down");
	graphBuilderProxy_.Add(mixDownFilter);

	auto mixDownInputPins = mixDownFilter.GetInputPins();
	if (mixDownInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in Mix-down Filter.");

	hr = graphBuilderProxy_->ConnectDirect(targetOutputPin, mixDownInputPins[0], mediaType.mt_);
	if (FAILED(hr)) throw HResultException(hr);

	return mixDownFilter;
}

FilterProxy GraphBuilder::ConnectWaveOutRenderer(FilterProxy& lastTransformFilter)
{
	auto lastTransformTargetOutputPin = GetSmartTeePreviewOrDefault(lastTransformFilter);

	auto lastTransformTargetOutputPinMediaTypes = lastTransformTargetOutputPin.GetPreferedMediaType();
	if (lastTransformTargetOutputPinMediaTypes.size() == 0) throw AppException(L"Cannot get MediaType for Output Pin in Last Transform Filter.");

	auto rendererFilter = FilterProxy::CreateInstanceFromClsid(CLSID_AudioRender, L"WaveOut Renderer");
	graphBuilderProxy_.Add(rendererFilter);

	auto rendererInputPins = rendererFilter.GetInputPins();
	if (rendererInputPins.size() == 0) throw AppException(L"Cannot find Input Pin in WaveOut Renderer Filter.");

	HRESULT hr = graphBuilderProxy_->ConnectDirect(lastTransformTargetOutputPin, rendererInputPins[0], lastTransformTargetOutputPinMediaTypes[0].mt_);
	if (FAILED(hr)) throw HResultException(hr);

	return rendererFilter;
}