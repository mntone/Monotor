#pragma once

class MediaType final
{
public:
	MediaType();
	MediaType(MediaType&&) = default;
	~MediaType();

	MediaType& operator=(MediaType&&) = default;

	HRESULT _stdcall Initialize();
	HRESULT _stdcall Set(AM_MEDIA_TYPE const& otherMediaType);
	HRESULT _stdcall Set(MediaType const& otherMediaType);

	HRESULT _stdcall CopyTo(AM_MEDIA_TYPE& otherMediaType) const;
	HRESULT _stdcall CopyTo(MediaType& otherMediaType) const;

	bool MatchesPartial(AM_MEDIA_TYPE const& otherMediaType) const;

	bool IsPartiallySpecified() const { return mt_->majortype == GUID_NULL || mt_->subtype == GUID_NULL || mt_->formattype == GUID_NULL; }
	bool IsPcmAudio() const
	{
		return mt_->majortype == MEDIATYPE_Audio
			&& mt_->subtype == MEDIASUBTYPE_PCM
			&& mt_->formattype == FORMAT_WaveFormatEx
			&& mt_->cbFormat == sizeof(WAVEFORMATEX)
			&& mt_->pbFormat != nullptr;
	}

	bool IsFixedSize() const { return mt_->bFixedSizeSamples == TRUE; }

	WAVEFORMATEX& AsWaveFormatEx() { return *reinterpret_cast<WAVEFORMATEX*>(mt_->pbFormat); }
	WAVEFORMATEX const& AsWaveFormatEx() const { return *reinterpret_cast<WAVEFORMATEX const*>(mt_->pbFormat); }

	GUID const& Type() const { return mt_->majortype; }
	void SetType(GUID const& value) { mt_->majortype = value; }

	GUID const& SubType() const { return mt_->subtype; }
	void SetSubType(GUID const& value) { mt_->subtype = value; }

	static HRESULT _stdcall CreateInstance(_Out_ MediaType** ret);

private:
	MediaType(MediaType const&) = delete;

	MediaType& operator=(MediaType const&) = delete;

public:
	AM_MEDIA_TYPE* mt_;
};

extern "C" {
	AM_MEDIA_TYPE* _stdcall CreateMediaType();
	HRESULT _stdcall InitializeMediaType(_In_ AM_MEDIA_TYPE& mediaType);
	HRESULT _stdcall CreatePcmAudioType(_In_ WORD channel, _In_ DWORD sampleRate, _In_ WORD bits, _Outptr_opt_ AM_MEDIA_TYPE* mediaType);
	HRESULT _stdcall CopyMediaType(_In_ AM_MEDIA_TYPE const& source, _Out_ AM_MEDIA_TYPE& target);
	void _stdcall DeleteMediaType(_Inout_ AM_MEDIA_TYPE& mediaType);
	void _stdcall FreeMediaType(_Inout_ AM_MEDIA_TYPE& mediaType);
}