#pragma once

class MediaType final
{
public:
	MediaType();
	MediaType(MediaType const& other);
	MediaType(MediaType&&);
	~MediaType();

	MediaType& operator=(MediaType const& other);
	MediaType& operator=(MediaType&&);

	HRESULT Initialize() noexcept;
	HRESULT Set(AM_MEDIA_TYPE const& other) noexcept;
	HRESULT Set(AM_MEDIA_TYPE&& other) noexcept;
	HRESULT Set(MediaType const& other) noexcept;

	HRESULT CopyTo(AM_MEDIA_TYPE& otherMediaType) const noexcept;
	HRESULT CopyTo(MediaType& otherMediaType) const noexcept;

	bool MatchesPartial(AM_MEDIA_TYPE const& otherMediaType) const noexcept;

	bool IsPartiallySpecified() const noexcept { return mt_->majortype == GUID_NULL || mt_->subtype == GUID_NULL || mt_->formattype == GUID_NULL; }
	bool IsVideo() const noexcept { return mt_->majortype == MEDIATYPE_Video; }
	bool IsAudio() const noexcept { return mt_->majortype == MEDIATYPE_Audio; }
	bool IsPcmAudio() const noexcept
	{
		return IsAudio()
			&& mt_->subtype == MEDIASUBTYPE_PCM
			&& IsWaveFormatEx()
			&& mt_->pbFormat != nullptr
			&& AsWaveFormatEx().wFormatTag == WAVE_FORMAT_PCM;
	}

	bool IsFixedSize() const noexcept { return mt_->bFixedSizeSamples == TRUE; }

	bool IsVideoInfoHeader() const noexcept { return mt_->formattype == FORMAT_VideoInfo && mt_->cbFormat >= sizeof(VIDEOINFOHEADER); }
	VIDEOINFOHEADER& AsVideoInfoHeader() noexcept { return *reinterpret_cast<VIDEOINFOHEADER*>(mt_->pbFormat); }
	VIDEOINFOHEADER const& AsVideoInfoHeader() const noexcept { return *reinterpret_cast<VIDEOINFOHEADER const*>(mt_->pbFormat); }

	bool IsVideoInfoHeader2() const noexcept { return mt_->formattype == FORMAT_VideoInfo2 && mt_->cbFormat >= sizeof(VIDEOINFOHEADER2); }
	VIDEOINFOHEADER2& AsVideoInfoHeader2() noexcept { return *reinterpret_cast<VIDEOINFOHEADER2*>(mt_->pbFormat); }
	VIDEOINFOHEADER2 const& AsVideoInfoHeader2() const noexcept { return *reinterpret_cast<VIDEOINFOHEADER2 const*>(mt_->pbFormat); }

	bool IsWaveFormatEx() const noexcept { return mt_->formattype == FORMAT_WaveFormatEx && mt_->cbFormat == sizeof(WAVEFORMATEX); }
	WAVEFORMATEX& AsWaveFormatEx() noexcept { return *reinterpret_cast<WAVEFORMATEX*>(mt_->pbFormat); }
	WAVEFORMATEX const& AsWaveFormatEx() const noexcept { return *reinterpret_cast<WAVEFORMATEX const*>(mt_->pbFormat); }

	GUID const& Type() const noexcept { return mt_->majortype; }
	void SetType(GUID const& value) noexcept { mt_->majortype = value; }

	GUID const& SubType() const noexcept { return mt_->subtype; }
	void SetSubType(GUID const& value) noexcept { mt_->subtype = value; }

	::std::wstring SubTypeAsString() const noexcept;

	::std::string GetFourCC() const noexcept;

	static HRESULT CreateInstance(_Out_ MediaType** ret) noexcept;

public:
	AM_MEDIA_TYPE* mt_;
};

extern "C" {
	AM_MEDIA_TYPE* _stdcall CreateMediaType();
	HRESULT _stdcall InitializeMediaType(_In_ AM_MEDIA_TYPE& mediaType);
	HRESULT _stdcall CreateMjpegVideoType(_In_ WORD height, _In_ WORD width, _In_ float framerate, _Outptr_opt_ AM_MEDIA_TYPE* mediaType);
	HRESULT _stdcall CreateMjpegVideoType2(_Outptr_opt_ AM_MEDIA_TYPE* mediaType);
	HRESULT _stdcall CreateRgbVideoType(_In_ WORD height, _In_ WORD width, _In_ WORD bits, _In_ float framerate, _Outptr_opt_ AM_MEDIA_TYPE* mediaType);
	HRESULT _stdcall CreateRgbVideoType2(_Outptr_opt_ AM_MEDIA_TYPE* mediaType);
	HRESULT _stdcall CreateYuvVideoType(_In_ WORD height, _In_ WORD width, _In_ float framerate, _In_ char* fourcc, _Outptr_opt_ AM_MEDIA_TYPE* mediaType);
	HRESULT _stdcall CreatePcmAudioType(_In_ WORD channel, _In_ DWORD sampleRate, _In_ WORD bits, _Outptr_opt_ AM_MEDIA_TYPE* mediaType);
	HRESULT _stdcall CopyMediaType(_In_ AM_MEDIA_TYPE const& source, _Out_ AM_MEDIA_TYPE& target);
	void _stdcall DeleteMediaType(_Inout_ AM_MEDIA_TYPE& mediaType);
	void _stdcall FreeMediaType(_Inout_ AM_MEDIA_TYPE& mediaType);
}