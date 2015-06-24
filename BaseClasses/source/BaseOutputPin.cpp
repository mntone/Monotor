#include "pch.hpp"
#include "BaseOutputPin.hpp"

using namespace std;

BaseOutputPin::BaseOutputPin(BaseFilter* const filter)
	: BasePin(filter)
{
	name_ = L"Out";
}

HRESULT BaseOutputPin::ReceiveConnection(IPin* /*pin*/, AM_MEDIA_TYPE const* /*mediaType*/) { PrintDebugLog(L"Start"); return E_UNEXPECTED; }

HRESULT BaseOutputPin::QueryPinInfo(PIN_INFO* info)
{
	PrintDebugLog(L"Start");
	HRESULT hr = BasePin::QueryPinInfo(info);
	if (FAILED(hr)) return hr;

	info->dir = PIN_DIRECTION::PINDIR_OUTPUT;
	return S_OK;
}

HRESULT BaseOutputPin::QueryDirection(PIN_DIRECTION* pinDirection)
{
	PrintDebugLog(L"Start");
	CheckPointer(pinDirection);
	*pinDirection = PIN_DIRECTION::PINDIR_OUTPUT;
	return S_OK;
}

HRESULT BaseOutputPin::EndOfStream() { PrintDebugLog(L"Start"); return E_UNEXPECTED; }
HRESULT BaseOutputPin::BeginFlush() { PrintDebugLog(L"Start"); return E_UNEXPECTED; }
HRESULT BaseOutputPin::EndFlush() { PrintDebugLog(L"Start"); return E_UNEXPECTED; }

HRESULT BaseOutputPin::Notify(IBaseFilter* /*selfFilter*/, Quality /*quality*/)
{
	PrintDebugLog(L"Start");
	return E_NOTIMPL;
}

HRESULT BaseOutputPin::CheckConnection(IPin* pin)
{
	PrintDebugLog(L"Start");
	return pin->QueryInterface(memInputPin_.GetAddressOf());
}

HRESULT BaseOutputPin::BreakConnection()
{
	PrintDebugLog(L"Start");
	HRESULT hr = S_OK;

	if (memAllocator_)
	{
		hr = memAllocator_->Decommit();
		if (SUCCEEDED(hr)) memAllocator_.Reset();
	}

	memInputPin_.Reset();
	return hr;
}

HRESULT BaseOutputPin::Active()
{
	PrintDebugLog(L"Start");
	if (memAllocator_ == nullptr) return VFW_E_NO_ALLOCATOR;
	return memAllocator_->Commit();
}

HRESULT BaseOutputPin::Inactive()
{
	PrintDebugLog(L"Start");
	error_ = false;
	if (memAllocator_ == nullptr) return VFW_E_NO_ALLOCATOR;
	return memAllocator_->Decommit();
}

HRESULT BaseOutputPin::DeliverEndOfStream()
{
	PrintDebugLog(L"Start");
	if (pin_ == nullptr) return VFW_E_NOT_CONNECTED;
	return pin_->EndOfStream();
}

HRESULT BaseOutputPin::DeliverBeginFlush()
{
	PrintDebugLog(L"Start");
	if (pin_ == nullptr) return VFW_E_NOT_CONNECTED;
	return pin_->BeginFlush();
}

HRESULT BaseOutputPin::DeliverEndFlush()
{
	PrintDebugLog(L"Start");
	if (pin_ == nullptr) return VFW_E_NOT_CONNECTED;
	return pin_->EndFlush();
}

HRESULT BaseOutputPin::DeliverNewSegment(REFERENCE_TIME startTime, REFERENCE_TIME stopTime, double rate)
{
	PrintDebugLog(L"Start");
	if (pin_ == nullptr) return VFW_E_NOT_CONNECTED;
	return pin_->NewSegment(startTime, stopTime, rate);
}

HRESULT BaseOutputPin::CompleteConnection(IPin* /*receivePin*/)
{
	PrintDebugLog(L"Start");
	return DecideAllocator(*memAllocator_.GetAddressOf());
}

HRESULT BaseOutputPin::DecideAllocator(IMemAllocator*& memAllocator)
{
	PrintDebugLog(L"Start");
	HRESULT hr = S_OK;

	ALLOCATOR_PROPERTIES properties = { 0 };
	hr = memInputPin_->GetAllocatorRequirements(&properties);
	if (properties.cbAlign == 0) properties.cbAlign = 1;

	hr = memInputPin_->GetAllocator(&memAllocator);
	if (SUCCEEDED(hr))
	{
		hr = DecideBufferSize(memAllocator, &properties);
		if (SUCCEEDED(hr))
		{
			hr = memInputPin_->NotifyAllocator(memAllocator, FALSE);
			return hr;
		}
	}

	// error process
	if (memAllocator)
	{
		memAllocator->Release();
		memAllocator = nullptr;
	}

	// try
	hr = CreateMemoryAllocator(memAllocator);
	if (SUCCEEDED(hr))
	{
		hr = DecideBufferSize(memAllocator, &properties);
		if (SUCCEEDED(hr))
		{
			hr = memInputPin_->NotifyAllocator(memAllocator, FALSE);
			return hr;
		}
	}

	// error process
	if (memAllocator)
	{
		memAllocator->Release();
		memAllocator = nullptr;
	}
	return hr;
}

HRESULT BaseOutputPin::CreateBuffer(REFERENCE_TIME* startTime, REFERENCE_TIME* endTime, DWORD flags, IMediaSample*& mediaSample)
{
	PrintDebugLog(L"Start");
	assert(memAllocator_ != nullptr);
	return memAllocator_->GetBuffer(&mediaSample, startTime, endTime, flags);
}

#if _DEBUG
HRESULT BaseOutputPin::DeliverBuffer(IMediaSample* mediaSample)
{
	PrintDebugLog(L"Start");
	if (memInputPin_ == nullptr) return VFW_E_NOT_CONNECTED;
	return memInputPin_->Receive(mediaSample);
}
#endif