#pragma once
#include "BaseInputPin.hpp"

class TransformFilter;
class TransformInputPin final
	: public BaseInputPin
{
public:
	TransformInputPin(_In_ TransformFilter* const filter);

	// IPin
	virtual HRESULT _stdcall EndOfStream() override;
	virtual HRESULT _stdcall BeginFlush() override;
	virtual HRESULT _stdcall EndFlush() override;
	virtual HRESULT _stdcall NewSegment(_In_ REFERENCE_TIME startTime, _In_ REFERENCE_TIME stopTime, _In_ double rate) override;

	// IMemInputPin
	virtual HRESULT _stdcall Receive(_In_ IMediaSample* mediaSample) override;

	virtual HRESULT _stdcall CheckStreaming() override;

protected:
	virtual HRESULT _stdcall CheckConnection(_In_ IPin* receivePin) override;
	virtual HRESULT _stdcall CompleteConnection(_In_ IPin* receivePin) override;
	virtual HRESULT _stdcall BreakConnection() override;

	virtual HRESULT _stdcall CheckMediaType(_In_ AM_MEDIA_TYPE const& mediaType) override;
	virtual HRESULT _stdcall SetMediaType(_In_ AM_MEDIA_TYPE const& mediaType) override;
	virtual HRESULT _stdcall GetPreferredMediaType(_In_ ULONG position, _Inout_ AM_MEDIA_TYPE* mediaType) override;

private:
	TransformInputPin() = delete;
	TransformInputPin(TransformInputPin const&) = delete;
	TransformInputPin(TransformInputPin&&) = delete;

	TransformInputPin& operator=(TransformInputPin const&) = delete;
	TransformInputPin& operator=(TransformInputPin&&) = delete;
};