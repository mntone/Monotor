#include "pch.hpp"
#include "EnumPins.hpp"
#include "UnknownObject.hpp"

template<uint8_t pinCount>
struct PinsDefinition
{ };

template<>
struct PinsDefinition<1>
{
	::Microsoft::WRL::ComPtr<IPin> p1;
};
template<>
struct PinsDefinition<2>
{
	::Microsoft::WRL::ComPtr<IPin> p1;
	::Microsoft::WRL::ComPtr<IPin> p2;
};

template<uint8_t C>
class EnumPins final
	: public IEnumPins
{
	friend class EnumPinsFactory;

public:
	virtual HRESULT _stdcall QueryInterface(IID const& iid, _COM_Outptr_ void** ret) override final;
	virtual ULONG _stdcall AddRef() override final;
	virtual ULONG _stdcall Release() override final;

	virtual HRESULT _stdcall Next(_In_ ULONG pinCount, _Out_writes_to_(pinCount, *fetched) IPin** pins, _Out_opt_ ULONG* fetched) override final;
	virtual HRESULT _stdcall Skip(_In_ ULONG pinCount) override final;
	virtual HRESULT _stdcall Reset() override final;
	virtual HRESULT _stdcall Clone(_Out_ IEnumPins** enumPins) override final;

private:
	EnumPins(_In_ IPin* pin1);
	EnumPins(_In_ IPin* pin1, _In_ IPin* pin2);

	EnumPins() = delete;
	EnumPins(EnumPins const&) = delete;
	EnumPins(EnumPins&&) = delete;

	EnumPins& operator=(EnumPins const&) = delete;
	EnumPins& operator=(EnumPins&&) = delete;

private:
	volatile uint32_t referenceCount_;
	uint8_t index_;
	PinsDefinition<C> pins_;
};

HRESULT EnumPinsFactory::CreateInstance(IPin* pin1, IEnumPins** ret)
{
	PrintDebugLog(L"Start");
	CheckPointer(ret);

	IEnumPins* const enumPins = new(::std::nothrow) EnumPins<1>(pin1);
	if (enumPins == nullptr) return E_OUTOFMEMORY;

	enumPins->AddRef();
	*ret = enumPins;
	return S_OK;
}

HRESULT EnumPinsFactory::CreateInstance(IPin* pin1, IPin* pin2, IEnumPins** ret)
{
	PrintDebugLog(L"Start");
	CheckPointer(ret);

	IEnumPins* const enumPins = new(::std::nothrow) EnumPins<2>(pin1, pin2);
	if (enumPins == nullptr) return E_OUTOFMEMORY;

	enumPins->AddRef();
	*ret = enumPins;
	return S_OK;
}


template<uint8_t C>
HRESULT EnumPins<C>::QueryInterface(IID const& iid, _COM_Outptr_ void** ret)
{
	PrintDebugLog(L"Start");
	CheckPointer(ret);

	if (iid == IID_IUnknown || iid == IID_IEnumPins)
	{
		AddRef();
		*ret = this;
		return S_OK;
	}
	*ret = nullptr;
	return E_NOINTERFACE;
}

template<uint8_t C>
ULONG EnumPins<C>::AddRef()
{
	PrintDebugLog(L"Start");
	return InterlockedIncrement(&referenceCount_);
}

template<uint8_t C>
ULONG EnumPins<C>::Release()
{
	PrintDebugLog(L"Start");
	ULONG ret = InterlockedDecrement(&referenceCount_);
	if (ret == 0)
	{
		delete this;
	}
	return ret;
}

template<uint8_t C>
HRESULT EnumPins<C>::Skip(ULONG pinCount)
{
	PrintDebugLog(L"Start");
	uint32_t leftCount = pinCount - index_;
	if (C > leftCount)
	{
		return S_FALSE;
	}
	return S_OK;
}

template<uint8_t C>
HRESULT EnumPins<C>::Reset()
{
	PrintDebugLog(L"Start");
	index_ = 0;
	return S_OK;
}

template<>
EnumPins<1>::EnumPins(IPin* pin1)
{
	referenceCount_ = 0;
	index_ = 0;
	pins_.p1 = pin1;
}

template<>
EnumPins<2>::EnumPins(IPin* pin1, IPin* pin2)
{
	referenceCount_ = 0;
	index_ = 0;
	pins_.p1 = pin1;
	pins_.p2 = pin2;
}

template<>
HRESULT EnumPins<1>::Next(ULONG pinCount, IPin** pins, ULONG* fetched)
{
	PrintDebugLog(L"Start");
	CheckPointer(pins);

	if (fetched != nullptr)
	{
		*fetched = 0;
	}
	else if (pinCount > 1)
	{
		return E_INVALIDARG;
	}

	if (index_ == 0 && pinCount >= 1)
	{
		index_ = 1;
		pins_.p1.CopyTo(pins);

		if (fetched != nullptr) *fetched = 1;
		return pinCount == 1 ? S_OK : S_FALSE;
	}

	return pinCount == 0 ? S_OK : S_FALSE;
}

HRESULT EnumPins<1>::Clone(IEnumPins** enumPins)
{
	PrintDebugLog(L"Start");
	return EnumPinsFactory::CreateInstance(pins_.p1.Get(), enumPins);
}

HRESULT EnumPins<2>::Next(ULONG pinCount, IPin** pins, ULONG* fetched)
{
	PrintDebugLog(L"Start");
	CheckPointer(pins);

	if (fetched != nullptr)
	{
		*fetched = 0;
	}
	else if (pinCount > 1)
	{
		return E_INVALIDARG;
	}

	if (index_ == 0)
	{
		if (pinCount == 1)
		{
			index_ = 1;
			pins_.p1.CopyTo(pins);

			if (fetched != nullptr) *fetched = 1;
			return S_OK;
		}
		if (pinCount >= 2)
		{
			index_ = 2;
			pins_.p1.CopyTo(pins);
			pins_.p2.CopyTo(pins + 1);

			*fetched = 2;
			return pinCount == 2 ? S_OK : S_FALSE;
		}
	}
	if (index_ == 1 && pinCount >= 1)
	{
		index_ = 2;
		pins_.p2.CopyTo(pins);

		if (fetched != nullptr) *fetched = 1;
		return pinCount == 1 ? S_OK : S_FALSE;
	}

	return pinCount == 0 ? S_OK : S_FALSE;
}

HRESULT EnumPins<2>::Clone(IEnumPins** enumPins)
{
	PrintDebugLog(L"Start");
	return EnumPinsFactory::CreateInstance(pins_.p1.Get(), pins_.p2.Get(), enumPins);
}