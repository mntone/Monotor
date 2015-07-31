#include "pch.hpp"
#include "RunningObjectTableRegistrar.hpp"

#include "HResultException.hpp"

using namespace Mntone::DirectShowSupport;

RunningObjectTableRegistrar::RunningObjectTableRegistrar() noexcept
	: initialized_(false)
	, registerID_(0)
{ }

RunningObjectTableRegistrar::RunningObjectTableRegistrar(IGraphBuilder* graphBuilder)
	: initialized_(false)
	, registerID_(0)
{
	Initialize(graphBuilder);
}

RunningObjectTableRegistrar::~RunningObjectTableRegistrar() noexcept
{
	if (!initialized_) return;

	HRESULT hr = runningObjectTable_->Revoke(registerID_);
	assert(SUCCEEDED(hr));
}

void RunningObjectTableRegistrar::Initialize(IGraphBuilder* graphBuilder)
{
	HRESULT hr = S_OK;

	hr = GetRunningObjectTable(0, &runningObjectTable_);
	if (FAILED(hr)) throw HResultException(hr);

	::Microsoft::WRL::ComPtr<IMoniker> moniker;
	wchar_t wsz[128];
	wsprintfW(wsz, L"FilterGraph %08x pid %08x", graphBuilder, GetCurrentProcessId());
	hr = CreateItemMoniker(L"!", wsz, &moniker);
	if (FAILED(hr)) throw HResultException(hr);

	hr = runningObjectTable_->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, graphBuilder, moniker.Get(), &registerID_);
	if (FAILED(hr)) throw HResultException(hr);

	initialized_ = true;
}