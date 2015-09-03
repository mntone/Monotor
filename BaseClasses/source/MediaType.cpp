#include "pch.hpp"
#include "MediaType.hpp"
#include "ExtendedGuid.hpp"

MediaType::MediaType()
{
	Initialize();
}

MediaType::MediaType(MediaType const& other)
{
	Initialize();
	CopyMediaType(*other.mt_, *mt_);
}

MediaType::MediaType(MediaType&& other)
{
	mt_ = other.mt_;
	other.mt_ = nullptr;
}

MediaType::~MediaType()
{
	if (mt_ != nullptr)
	{
		DeleteMediaType(*mt_);
		mt_ = nullptr;
	}
}

MediaType& MediaType::operator=(MediaType const& other)
{
	Initialize();
	CopyMediaType(*other.mt_, *mt_);
	return *this;
}

MediaType& MediaType::operator=(MediaType&& other)
{
	mt_ = other.mt_;
	other.mt_ = nullptr;
	return *this;
}

HRESULT MediaType::Initialize() noexcept
{
	mt_ = CreateMediaType();
	InitializeMediaType(*mt_);
	return S_OK;
}

HRESULT MediaType::Set(AM_MEDIA_TYPE const& other) noexcept
{
	if (&other == mt_) return S_OK;
	FreeMediaType(*mt_);
	return CopyMediaType(other, *mt_);
}

HRESULT MediaType::Set(AM_MEDIA_TYPE&& other) noexcept
{
	if (&other == mt_) return S_OK;
	FreeMediaType(*mt_);
	mt_ = &other;
	return S_OK;
}

HRESULT MediaType::Set(MediaType const& other) noexcept
{
	if (other.mt_ == mt_) return S_OK;
	FreeMediaType(*mt_);
	return CopyMediaType(*other.mt_, *mt_);
}

HRESULT MediaType::CopyTo(AM_MEDIA_TYPE& otherMediaType) const noexcept
{
	if (&otherMediaType == mt_) return S_OK;
	return CopyMediaType(*mt_, otherMediaType);
}

HRESULT MediaType::CopyTo(MediaType& otherMediaType) const noexcept
{
	if (otherMediaType.mt_ == mt_) return S_OK;
	return CopyMediaType(*mt_, *otherMediaType.mt_);
}

bool MediaType::MatchesPartial(AM_MEDIA_TYPE const& otherMediaType) const noexcept
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

::std::wstring MediaType::SubTypeAsString() const noexcept
{
	if (mt_->subtype == MEDIASUBTYPE_RGB1) return L"RGB1";
	if (mt_->subtype == MEDIASUBTYPE_RGB4) return L"RGB4";
	if (mt_->subtype == MEDIASUBTYPE_RGB8) return L"RGB8";
	if (mt_->subtype == MEDIASUBTYPE_RGB555) return L"X1R5G5B5";
	if (mt_->subtype == MEDIASUBTYPE_RGB565) return L"R5G6B5";
	if (mt_->subtype == MEDIASUBTYPE_RGB24) return L"R8G8B8";
	if (mt_->subtype == MEDIASUBTYPE_RGB32) return L"X8R8G8B8";
	if (mt_->subtype == MEDIASUBTYPE_ARGB1555) return L"A1R5G5B5";
	if (mt_->subtype == MEDIASUBTYPE_ARGB4444) return L"A4R4G4B4";
	if (mt_->subtype == MEDIASUBTYPE_ARGB32) return L"A8R8G8B8";
	if (mt_->subtype == MEDIASUBTYPE_A2R10G10B10) return L"A2R10G10B10";
	if (mt_->subtype == MEDIASUBTYPE_A2B10G10R10) return L"A2B10G10R10";
	if (mt_->subtype.Data2 == 0x0000 && mt_->subtype.Data3 == 0x0010 && *reinterpret_cast<uint64_t*>(mt_->subtype.Data4) == 0x719b3800aa000080)
	{
		wchar_t buf[5] = { 0 };
		size_t s = 0;
		mbstowcs_s(&s, buf, reinterpret_cast<char const*>(&mt_->subtype.Data1), 4);
		return buf;
	}
	return L"Unknown";
}

::std::string MediaType::GetFourCC() const noexcept
{
	char fourcc[5] = { 0 };
	if (IsVideoInfoHeader())
	{
		auto& vi = AsVideoInfoHeader();
		if (vi.bmiHeader.biCompression == BI_RGB)
		{
			memcpy(fourcc, "RGB", 3);
		}
		else
		{
			memcpy(fourcc, &vi.bmiHeader.biCompression, 4);
		}
	}
	else if (IsVideoInfoHeader2())
	{
		auto& vi = AsVideoInfoHeader2();
		if (vi.bmiHeader.biCompression == BI_RGB)
		{
			memcpy(fourcc, "RGB", 3);
		}
		else
		{
			memcpy(fourcc, &vi.bmiHeader.biCompression, 4);
		}
	}

	return fourcc;
}

HRESULT MediaType::CreateInstance(MediaType** ret) noexcept
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
		mediaType.formattype == FORMAT_None;
		mediaType.lSampleSize = 1;
		mediaType.bFixedSizeSamples = TRUE;
		return S_OK;
	}

	HRESULT _stdcall CreateMjpegVideoType(WORD height, WORD width, float framerate, AM_MEDIA_TYPE* mediaType)
	{
		CheckPointer(mediaType);
		FreeMediaType(*mediaType);

		auto& mt = *mediaType;
		mt.majortype = MEDIATYPE_Video;
		mt.subtype = MEDIASUBTYPE_MJPG;
		mt.formattype = FORMAT_VideoInfo;

		mt.cbFormat = sizeof(VIDEOINFOHEADER);
		mt.pbFormat= static_cast<BYTE*>(CoTaskMemAlloc(sizeof(VIDEOINFOHEADER)));

		mt.lSampleSize = (24 / 8) * height * ((width + 3) & ~3);

		VIDEOINFOHEADER& vi = *reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
		vi.rcSource = { 0, 0, 0, 0 };
		vi.rcTarget = { 0, 0, 0, 0 };
		vi.dwBitRate = (24 / 8) * 240 * height * width;
		vi.dwBitErrorRate = 0;
		vi.AvgTimePerFrame = static_cast<REFERENCE_TIME>(10000000.0 / framerate);

		vi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vi.bmiHeader.biWidth = width;
		vi.bmiHeader.biHeight = height;
		vi.bmiHeader.biPlanes = 1;
		vi.bmiHeader.biBitCount = 24;
		memcpy_s(&vi.bmiHeader.biCompression, 4, "MJPG", 4);
		vi.bmiHeader.biSizeImage = (24 / 8) * height * ((width + 3) & ~3);
		vi.bmiHeader.biXPelsPerMeter = 0;
		vi.bmiHeader.biYPelsPerMeter = 0;
		vi.bmiHeader.biClrUsed = 0;
		vi.bmiHeader.biClrImportant = 0;
		return S_OK;
	}

	HRESULT _stdcall CreateRgbVideoType(WORD height, WORD width, WORD bits, float framerate, _Outptr_opt_ AM_MEDIA_TYPE* mediaType)
	{
		CheckPointer(mediaType);
		FreeMediaType(*mediaType);

		auto& mt = *mediaType;
		mt.majortype = MEDIATYPE_Video;
		mt.subtype = MEDIASUBTYPE_MJPG;
		mt.formattype = FORMAT_VideoInfo;
		mt.lSampleSize = (bits / 8) * height * ((width + 3) & ~3);
		mt.bFixedSizeSamples = TRUE;

		mt.cbFormat = sizeof(VIDEOINFOHEADER);
		mt.pbFormat = static_cast<BYTE*>(CoTaskMemAlloc(sizeof(VIDEOINFOHEADER)));

		VIDEOINFOHEADER& vi = *reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
		vi.rcSource = { 0, 0, 0, 0 };
		vi.rcTarget = { 0, 0, 0, 0 };
		vi.dwBitRate = static_cast<DWORD>(static_cast<float>(bits * height * ((width + 3) & ~3)) / framerate);
		vi.dwBitErrorRate = 0;
		vi.AvgTimePerFrame = static_cast<REFERENCE_TIME>(10000000.0 / framerate);

		vi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vi.bmiHeader.biWidth = width;
		vi.bmiHeader.biHeight = height;
		vi.bmiHeader.biPlanes = 1;
		vi.bmiHeader.biBitCount = bits;
		vi.bmiHeader.biCompression = BI_RGB;
		vi.bmiHeader.biSizeImage = (bits / 8) * height * ((width + 3) & ~3);
		vi.bmiHeader.biXPelsPerMeter = 0;
		vi.bmiHeader.biYPelsPerMeter = 0;
		vi.bmiHeader.biClrUsed = 0;
		vi.bmiHeader.biClrImportant = 0;
		return S_OK;
	}

	HRESULT _stdcall CreateYuvVideoType(WORD height, WORD width, float framerate, _In_ char* fourcc, _Outptr_opt_ AM_MEDIA_TYPE* mediaType)
	{
		CheckPointer(mediaType);
		FreeMediaType(*mediaType);

		GUID subtype;
		char copiedFourcc[4];
		WORD bits = 0;
		if (memcmp(fourcc, "YUY2", 4) == 0)
		{
			subtype = MEDIASUBTYPE_YUY2;
			memcpy(copiedFourcc, fourcc, 4);
			bits = 16;
		}
		else if (memcmp(fourcc, "UYVY", 4) == 0)
		{
			subtype = MEDIASUBTYPE_UYVY;
			memcpy(copiedFourcc, fourcc, 4);
			bits = 16;
		}
		else if (memcmp(fourcc, "HDYC", 4) == 0)
		{
			subtype = MEDIASUBTYPE_HDYC;
			memcpy(copiedFourcc, fourcc, 4);
			bits = 16;
		}
		else
		{
			return E_FAIL;
		}

		auto& mt = *mediaType;
		mt.majortype = MEDIATYPE_Video;
		mt.subtype = subtype;
		mt.formattype = FORMAT_VideoInfo;
		mt.lSampleSize = (bits / 8) * height * ((width + 3) & ~3);
		mt.bFixedSizeSamples = TRUE;

		mt.cbFormat = sizeof(VIDEOINFOHEADER);
		mt.pbFormat = static_cast<BYTE*>(CoTaskMemAlloc(sizeof(VIDEOINFOHEADER)));

		VIDEOINFOHEADER& vi = *reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
		vi.rcSource = { 0, 0, 0, 0 };
		vi.rcTarget = { 0, 0, 0, 0 };
		vi.dwBitRate = static_cast<DWORD>(static_cast<float>(bits * height * ((width + 3) & ~3)) * framerate);
		vi.dwBitErrorRate = 0;
		vi.AvgTimePerFrame = static_cast<REFERENCE_TIME>(10000000.0 / framerate);

		vi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		vi.bmiHeader.biWidth = width;
		vi.bmiHeader.biHeight = height;
		vi.bmiHeader.biPlanes = 1;
		vi.bmiHeader.biBitCount = bits;
		memcpy(&vi.bmiHeader.biCompression, copiedFourcc, 4);
		vi.bmiHeader.biSizeImage = (bits / 8) * height * ((width + 3) & ~3);
		vi.bmiHeader.biXPelsPerMeter = 0;
		vi.bmiHeader.biYPelsPerMeter = 0;
		vi.bmiHeader.biClrUsed = 0;
		vi.bmiHeader.biClrImportant = 0;
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
		mt.lSampleSize = channel * (bits / 8);
		mt.bFixedSizeSamples = TRUE;

		mt.cbFormat = sizeof(WAVEFORMATEX);
		mt.pbFormat = static_cast<BYTE*>(CoTaskMemAlloc(sizeof(WAVEFORMATEX)));

		WAVEFORMATEX& wav = *reinterpret_cast<WAVEFORMATEX*>(mt.pbFormat);
		wav.wFormatTag = WAVE_FORMAT_PCM;
		wav.nChannels = channel;
		wav.nSamplesPerSec = sampleRate;
		wav.wBitsPerSample = bits;

		wav.nBlockAlign = channel * (bits / 8);
		wav.nAvgBytesPerSec = sampleRate * wav.nBlockAlign;
		wav.cbSize = 0;
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
		if (mediaType.pbFormat != nullptr)
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