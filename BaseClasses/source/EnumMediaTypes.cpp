#include "pch.hpp"
#include "EnumMediaTypes.hpp"

#include "MediaType.hpp"

class EnumMediaTypes final
	: public IEnumMediaTypes
{
public:
	EnumMediaTypes(_In_::std::vector<AM_MEDIA_TYPE*> types);
	~EnumMediaTypes();

	virtual HRESULT _stdcall QueryInterface(IID const& iid, _COM_Outptr_ void** ret) override final;
	virtual ULONG _stdcall AddRef() override final;
	virtual ULONG _stdcall Release() override final;

	virtual HRESULT _stdcall Next(_In_ ULONG mediaTypesCount, _Out_writes_to_(mediaTypeCount, *fetched) AM_MEDIA_TYPE** mediaTypes, _Out_opt_ ULONG* fetched) override final;
	virtual HRESULT _stdcall Skip(_In_ ULONG mediaTypesCount) override final;
	virtual HRESULT _stdcall Reset() override final;
	virtual HRESULT _stdcall Clone(_Out_ IEnumMediaTypes** enumMediaTypes) override final;

private:
	EnumMediaTypes() = delete;
	EnumMediaTypes(EnumMediaTypes const&) = delete;
	EnumMediaTypes(EnumMediaTypes&&) = delete;

	EnumMediaTypes& operator=(EnumMediaTypes const&) = delete;
	EnumMediaTypes& operator=(EnumMediaTypes&&) = delete;

private:
	volatile uint32_t referenceCount_;
	uint32_t index_;
	::std::vector<AM_MEDIA_TYPE*> types_;
};

using namespace std;

HRESULT EnumMediaTypesFactory::CreateInstance(vector<AM_MEDIA_TYPE*> types, IEnumMediaTypes** ret)
{
	PrintDebugLog(L"Start");
	CheckPointer(ret);

	IEnumMediaTypes* const enumMediaTypes = new(nothrow) EnumMediaTypes(move(types));
	if (enumMediaTypes == nullptr) return E_OUTOFMEMORY;

	enumMediaTypes->AddRef();
	*ret = enumMediaTypes;
	return S_OK;
}


EnumMediaTypes::EnumMediaTypes(vector<AM_MEDIA_TYPE*> types)
	: referenceCount_(0)
	, index_(0)
	, types_(move(types))
{ }

EnumMediaTypes::~EnumMediaTypes()
{
	for (auto&& type : types_)
	{
		DeleteMediaType(*type);
	}
}

HRESULT EnumMediaTypes::QueryInterface(IID const& iid, _COM_Outptr_ void** ret)
{
	PrintDebugLog(L"Start");
	CheckPointer(ret);

	if (iid == IID_IUnknown || iid == IID_IEnumMediaTypes)
	{
		AddRef();
		*ret = this;
		return S_OK;
	}
	*ret = nullptr;
	return E_NOINTERFACE;
}

ULONG EnumMediaTypes::AddRef()
{
	PrintDebugLog(L"Start");
	return InterlockedIncrement(&referenceCount_);
}

ULONG EnumMediaTypes::Release()
{
	PrintDebugLog(L"Start");
	ULONG ret = InterlockedDecrement(&referenceCount_);
	if (ret == 0)
	{
		delete this;
	}
	return ret;
}

HRESULT EnumMediaTypes::Next(ULONG mediaTypesCount, AM_MEDIA_TYPE** mediaTypes, ULONG* fetched)
{
	PrintDebugLog(L"Start");
	CheckPointer(mediaTypes);

	if (fetched != nullptr)
	{
		*fetched = 0;
	}
	else if (mediaTypesCount > 1)
	{
		return E_INVALIDARG;
	}

	uint32_t leftCount = types_.size() - index_;
	uint32_t copyCount = mediaTypesCount > leftCount ? leftCount : mediaTypesCount;

	for (ULONG i = 0; i < copyCount; ++i)
	{
		mediaTypes[i] = CreateMediaType();
		CopyMediaType(*(types_[index_++]), *(mediaTypes[i]));
	}

	if (fetched != nullptr)	*fetched = copyCount;
	return copyCount != 0 ? S_OK : S_FALSE;
}

HRESULT EnumMediaTypes::Skip(ULONG mediaTypesCount)
{
	PrintDebugLog(L"Start");
	uint32_t leftCount = mediaTypesCount - index_;
	if (types_.size() > leftCount)
	{
		return S_FALSE;
	}
	return S_OK;
}

HRESULT EnumMediaTypes::Reset()
{
	PrintDebugLog(L"Start");
	index_ = 0;
	return S_OK;
}

HRESULT EnumMediaTypes::Clone(IEnumMediaTypes** enumMediaTypes)
{
	PrintDebugLog(L"Start");
	::std::vector<AM_MEDIA_TYPE*> copiedTypes(types_.size());
	for (uint32_t i = 0; i < types_.size(); ++i)
	{
		copiedTypes[i] = CreateMediaType();
		InitializeMediaType(*copiedTypes[i]);
		CopyMediaType(*(types_[index_++]), *(copiedTypes[i]));
	}
	return EnumMediaTypesFactory::CreateInstance(copiedTypes, enumMediaTypes);
}