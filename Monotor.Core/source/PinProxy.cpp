#include "pch.hpp"
#include "PinProxy.hpp"

#include "HResultException.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace Mntone::DirectShowSupport;

#define MEDIA_TYPE_RETRIVE_COUNT 16

PinProxy::PinProxy(bool cacheEnabled) noexcept
	: cacheEnabled_(cacheEnabled)
	, directionCached_(false)
	, direction_(static_cast<PIN_DIRECTION>(0))
	, idCached_(false)
	, mediaTypeCached_(false)
{ }

PinProxy::PinProxy(IPin* pin, bool cacheEnabled) noexcept
	: PinProxy(cacheEnabled)
{
	pin_ = pin;
}

void PinProxy::ResetCache() noexcept
{
	directionCached_ = false;
	direction_ = static_cast<PIN_DIRECTION>(0);
	idCached_ = false;
	id_.clear();
	mediaTypeCached_ = false;
	mediaTypes_.clear();
}

wstring PinProxy::GetId()
{
	if (idCached_) return id_;

	wchar_t* id;
	HRESULT hr = pin_->QueryId(&id);
	if (FAILED(hr)) throw HResultException(hr);

	if (!cacheEnabled_) return id;

	id_ = id;
	idCached_ = true;
	return id_;
}

vector<MediaType> PinProxy::GetPreferedMediaType()
{
	if (mediaTypeCached_) return mediaTypes_;

	HRESULT hr = S_OK;
	ComPtr<IEnumMediaTypes> enumMediaTypes;
	hr = pin_->EnumMediaTypes(&enumMediaTypes);
	if (FAILED(hr)) throw HResultException(hr);

	vector<MediaType> mediaTypes;
	mediaTypes.reserve(MEDIA_TYPE_RETRIVE_COUNT);
	while (true)
	{
		ULONG retrived = MEDIA_TYPE_RETRIVE_COUNT;
		AM_MEDIA_TYPE* mediaTypeBuffer[MEDIA_TYPE_RETRIVE_COUNT];
		hr = enumMediaTypes->Next(MEDIA_TYPE_RETRIVE_COUNT, &mediaTypeBuffer[0], &retrived);
		if (FAILED(hr)) break;
		for (ULONG i = 0; i < retrived; ++i)
		{
			MediaType mt;
			mt.Set(::std::move(*mediaTypeBuffer[i]));
			mediaTypes.push_back(mt);
		}
		if (hr == S_FALSE) break;
	}

	if (!cacheEnabled_) return ::std::move(mediaTypes);

	mediaTypes_ = ::std::move(mediaTypes);
	mediaTypeCached_ = true;
	return mediaTypes_;
}

PIN_DIRECTION PinProxy::GetDirectionInternal()
{
	if (directionCached_) return direction_;

	PIN_DIRECTION direction;
	HRESULT hr = pin_->QueryDirection(&direction);
	if (FAILED(hr)) throw HResultException(hr);

	if (!cacheEnabled_) return direction;

	direction_ = direction;
	directionCached_ = true;
	return direction_;
}