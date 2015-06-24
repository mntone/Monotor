#include "pch.hpp"
#include "MediaType.hpp"

MediaType::MediaType()
{
	Initialize();
}

MediaType::~MediaType()
{
	DeleteMediaType(*mt_);
}

HRESULT MediaType::Initialize()
{
	mt_ = CreateMediaType();
	InitializeMediaType(*mt_);
	return S_OK;
}

HRESULT MediaType::Set(AM_MEDIA_TYPE const& otherMediaType)
{
	if (&otherMediaType == mt_) return S_OK;
	FreeMediaType(*mt_);
	return CopyMediaType(otherMediaType, *mt_);
}

HRESULT MediaType::Set(MediaType const& otherMediaType)
{
	if (otherMediaType.mt_ == mt_) return S_OK;
	FreeMediaType(*mt_);
	return CopyMediaType(*otherMediaType.mt_, *mt_);
}

HRESULT MediaType::CopyTo(AM_MEDIA_TYPE& otherMediaType) const
{
	if (&otherMediaType == mt_) return S_OK;
	return CopyMediaType(*mt_, otherMediaType);
}

HRESULT MediaType::CopyTo(MediaType& otherMediaType) const
{
	if (otherMediaType.mt_ == mt_) return S_OK;
	return CopyMediaType(*mt_, *otherMediaType.mt_);
}

bool MediaType::MatchesPartial(AM_MEDIA_TYPE const& otherMediaType) const
{
	if (otherMediaType.majortype != GUID_NULL && mt_->majortype != otherMediaType.majortype)
	{
		return false;
	}
	if (otherMediaType.subtype != GUID_NULL && mt_->subtype != otherMediaType.subtype)
	{
		return false;
	}
	if (otherMediaType.formattype != GUID_NULL)
	{
		if (mt_->formattype != otherMediaType.formattype) return false;
		if (mt_->cbFormat != otherMediaType.cbFormat) return false;
		if (mt_->cbFormat != 0 && memcmp(mt_->pbFormat, otherMediaType.pbFormat, mt_->cbFormat) != 0)return false;
	}
	return true;
}

HRESULT MediaType::CreateInstance(MediaType** ret)
{
	CheckPointer(ret);

	*ret = new(::std::nothrow) MediaType();
	if (*ret == nullptr) return E_OUTOFMEMORY;

	return S_OK;
}

extern "C" {
	AM_MEDIA_TYPE* _stdcall CreateMediaType()
	{
		return static_cast<AM_MEDIA_TYPE*>(CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE)));
	}

	HRESULT _stdcall InitializeMediaType(AM_MEDIA_TYPE& mediaType)
	{
		mediaType = { 0 };
		mediaType.lSampleSize = 1;
		mediaType.bFixedSizeSamples = TRUE;
		return S_OK;
	}

	HRESULT _stdcall CreatePcmAudioType(WORD channel, DWORD sampleRate, WORD bits, AM_MEDIA_TYPE* mediaType)
	{
		CheckPointer(mediaType);
		FreeMediaType(*mediaType);

		auto& mt = *mediaType;
		mt.majortype = MEDIATYPE_Audio;
		mt.subtype = MEDIASUBTYPE_PCM;
		mt.formattype = FORMAT_WaveFormatEx;

		mt.cbFormat = sizeof(WAVEFORMATEX);
		mt.pbFormat = static_cast<BYTE*>(CoTaskMemAlloc(sizeof(WAVEFORMATEX)));

		WAVEFORMATEX& wav = *reinterpret_cast<WAVEFORMATEX*>(mt.pbFormat);
		wav.wFormatTag = WAVE_FORMAT_PCM;
		wav.nChannels = channel;
		wav.nSamplesPerSec = sampleRate;
		wav.wBitsPerSample = bits;
		wav.cbSize = 0;

		wav.nBlockAlign = channel * (bits / 8);
		wav.nAvgBytesPerSec = sampleRate * wav.nBlockAlign;
		return S_OK;
	}

	HRESULT _stdcall CopyMediaType(AM_MEDIA_TYPE const& source, AM_MEDIA_TYPE& target)
	{
		assert(&source != &target);

		target = source;
		if (source.cbFormat != 0)
		{
			assert(source.pbFormat);
			target.pbFormat = static_cast<BYTE*>(CoTaskMemAlloc(source.cbFormat));
			if (target.pbFormat == nullptr)
			{
				target.cbFormat = 0;
				return E_OUTOFMEMORY;
			}

			memcpy_s(target.pbFormat, target.cbFormat, source.pbFormat, source.cbFormat);
		}
		if (target.pUnk != nullptr)
		{
			target.pUnk->AddRef();
		}
		return S_OK;
	}

	void _stdcall DeleteMediaType(AM_MEDIA_TYPE& mediaType)
	{
		FreeMediaType(mediaType);
		CoTaskMemFree(&mediaType);
	}

	void _stdcall FreeMediaType(AM_MEDIA_TYPE& mediaType)
	{
		if (mediaType.cbFormat != 0)
		{
			CoTaskMemFree(mediaType.pbFormat);
			mediaType.cbFormat = 0;
			mediaType.pbFormat = nullptr;
		}
		if (mediaType.pUnk != nullptr)
		{
			mediaType.pUnk->Release();
			mediaType.pUnk = nullptr;
		}
	}
}