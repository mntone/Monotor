#include "pch.hpp"
#include "AudioMixDownFilter.hpp"

#include "AudioMixDownFilterDefine.hpp"
#include "AudioMixDownConverter.hpp"

using namespace std;

HRESULT AudioMixDownFilter::CreateInstance(ClassUnloadContext ctx, IUnknown** ret)
{
	CheckPointer(ret);

	AudioMixDownFilter* const filter = new(nothrow) AudioMixDownFilter(move(ctx));
	if (filter == nullptr) return E_OUTOFMEMORY;

	filter->AddRefInternal();
	*ret = reinterpret_cast<IUnknown*>(static_cast<IFalseUnknown*>(filter));
	return S_OK;
}

AudioMixDownFilter::AudioMixDownFilter(ClassUnloadContext ctx)
	: TransformFilter(CLSID_AudioMixDownFilter)
	, ctx_(move(ctx))
	, inputBit_(0)
	, outputBit_(0)
	, outputChannel_(0)
{
	name_ = NAME_AudioMixDownFilter;
}

HRESULT AudioMixDownFilter::GetInputPreferredMediaType(ULONG position, AM_MEDIA_TYPE* mediaType)
{
	PrintDebugLog(L"Start");
	CheckPointer(mediaType);

	// 5.1 ch only
	if (position == 0) return CreatePcmAudioType(6, 48000, 16, mediaType);
	if (position == 1) return CreatePcmAudioType(6, 48000, 24, mediaType);
	if (position == 2) return CreatePcmAudioType(6, 48000, 32, mediaType);
	return VFW_S_NO_MORE_ITEMS;
}

HRESULT AudioMixDownFilter::SetInputPinMediaType(AM_MEDIA_TYPE const* mediaType)
{
	PrintDebugLog(L"Start");
	MediaType mt;
	mt.Set(*mediaType);
	assert(mt.IsPcmAudio());
	inputBit_ = mt.AsWaveFormatEx().wBitsPerSample;
	assert(inputBit_ == 16 || inputBit_ == 24 || inputBit_ == 32);
	return S_OK;
}

HRESULT AudioMixDownFilter::CompleteOutputPinConnection(IPin* receivePin)
{
	PrintDebugLog(L"Start");
	try
	{
		converter_ = AudioMixDownConverter::GetTransform5chFunction(inputBit_, outputBit_, outputChannel_);
	}
	catch (exception const& ex)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT AudioMixDownFilter::GetOutputPreferredMediaType(ULONG position, AM_MEDIA_TYPE* mediaType)
{
	PrintDebugLog(L"Start");
	CheckPointer(mediaType);

	if (!IsInputPinConnected()) return VFW_S_NO_MORE_ITEMS;

	MediaType inputPinMediaType;
	HRESULT hr = inputPin_.ConnectionMediaType(inputPinMediaType.mt_);
	assert(SUCCEEDED(hr));
	if (!inputPinMediaType.IsPcmAudio()) return VFW_S_NO_MORE_ITEMS;

	// 1 or 2 ch only
	auto const& wav = inputPinMediaType.AsWaveFormatEx();
	assert(wav.nSamplesPerSec == 48000);
	if (wav.wBitsPerSample == 32)
	{
		if (position == 0) return CreatePcmAudioType(2, 48000, 32, mediaType);
		if (position == 1) return CreatePcmAudioType(1, 48000, 32, mediaType);
		if (position == 2) return CreatePcmAudioType(2, 48000, 24, mediaType);
		if (position == 3) return CreatePcmAudioType(1, 48000, 24, mediaType);
		if (position == 4) return CreatePcmAudioType(2, 48000, 16, mediaType);
		if (position == 5) return CreatePcmAudioType(1, 48000, 16, mediaType);
	}
	else if (wav.wBitsPerSample == 24)
	{
		if (position == 0) return CreatePcmAudioType(2, 48000, 24, mediaType);
		if (position == 1) return CreatePcmAudioType(1, 48000, 24, mediaType);
		if (position == 2) return CreatePcmAudioType(2, 48000, 16, mediaType);
		if (position == 3) return CreatePcmAudioType(1, 48000, 16, mediaType);
	}
	else if (wav.wBitsPerSample == 16)
	{
		if (position == 0) return CreatePcmAudioType(2, 48000, 16, mediaType);
		if (position == 1) return CreatePcmAudioType(1, 48000, 16, mediaType);
	}
	return VFW_S_NO_MORE_ITEMS;
}

HRESULT AudioMixDownFilter::SetOutputPinMediaType(AM_MEDIA_TYPE const* mediaType)
{
	PrintDebugLog(L"Start");
	MediaType mt;
	mt.Set(*mediaType);
	assert(mt.IsPcmAudio());
	outputBit_ = mt.AsWaveFormatEx().wBitsPerSample;
	assert(inputBit_ == 16 && outputBit_ == 16
		   || inputBit_ == 24 && (outputBit_ == 16 || outputBit_ == 24)
		   || inputBit_ == 32 && (outputBit_ == 16 || outputBit_ == 24 || outputBit_ == 32));
	outputChannel_ = mt.AsWaveFormatEx().nChannels;
	assert(outputChannel_ == 2 || outputChannel_ == 1);
	return S_OK;
}

HRESULT AudioMixDownFilter::DecideBufferSize(IMemAllocator* memAllocator, ALLOCATOR_PROPERTIES* properties)
{
	PrintDebugLog(L"Start");
	CheckPointer(memAllocator);
	CheckPointer(properties);

	::Microsoft::WRL::ComPtr<IMemAllocator> inputMemallocator;
	inputPin_.GetAllocator(&inputMemallocator);

	ALLOCATOR_PROPERTIES inputPinProperties = { 0 };
	inputMemallocator->GetProperties(&inputPinProperties);

	// calc buffer
	properties->cBuffers = inputPinProperties.cBuffers;
	properties->cbBuffer = 2 * 48000 * (16 / 8);

	lock_guard<recursive_mutex> lock(mutex_);

	ALLOCATOR_PROPERTIES actual = { 0 };
	HRESULT hr = memAllocator->SetProperties(properties, &actual);
	if (FAILED(hr)) return hr;
	if (actual.cbBuffer < properties->cbBuffer) return E_FAIL;
	return S_OK;
}

HRESULT AudioMixDownFilter::CheckMediaType(AM_MEDIA_TYPE const* inputPinMediaType, AM_MEDIA_TYPE const* outputPinMediaType)
{
	PrintDebugLog(L"Start");

	// Check input MediaType
	MediaType inputPinMediaType2;
	if (inputPinMediaType == nullptr) return E_FAIL;
	inputPinMediaType2.Set(*inputPinMediaType);
	if (!inputPinMediaType2.IsPcmAudio()) return S_FALSE;

	auto const& inWav = inputPinMediaType2.AsWaveFormatEx();
	if (inWav.nChannels != 6
		|| inWav.nSamplesPerSec != 48000
		|| (inWav.wBitsPerSample != 16 && inWav.wBitsPerSample != 24 && inWav.wBitsPerSample != 32))
	{
		return S_FALSE;
	}

	// Return if output-mt is null
	if (outputPinMediaType == nullptr) return S_OK;

	// Check output MediaType
	MediaType outputPinMediaType2;
	outputPinMediaType2.Set(*outputPinMediaType);
	if (!outputPinMediaType2.IsPcmAudio()) return S_FALSE;

	auto const& outWav = outputPinMediaType2.AsWaveFormatEx();
	if ((outWav.nChannels != 2 && outWav.nChannels != 1)
		|| outWav.nSamplesPerSec != 48000
		|| (inWav.wBitsPerSample == 16 && outWav.wBitsPerSample != 16)
		|| (inWav.wBitsPerSample == 24 && outWav.wBitsPerSample != 16 && outWav.wBitsPerSample != 24)
		|| (inWav.wBitsPerSample == 32 && outWav.wBitsPerSample != 16 && outWav.wBitsPerSample != 24 && outWav.wBitsPerSample != 32))
	{
		return S_FALSE;
	}
	return S_OK;
}

HRESULT AudioMixDownFilter::TransformOverride(BYTE* src, int32_t srcSize, BYTE* dst, int32_t* dstSize)
{
	PrintDebugLog(L"Start");
	auto buffer = srcSize / (6 * (16 / 8));

	void const* src2 = reinterpret_cast<int16_t*>(src);
	void* dst2 = reinterpret_cast<int16_t*>(dst);
	for (auto i = 0; i < buffer; ++i)
	{
		converter_(src2, dst2);
	}
	*dstSize = srcSize / 6 * 2;
	return S_OK;
}