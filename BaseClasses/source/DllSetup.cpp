#include "pch.hpp"
#include "DllSetup.hpp"

#include "UnknownObject.hpp"

using namespace std;

extern vector<pair<CLSID const&, function<HRESULT(ClassUnloadContext, IUnknown**)>>> g_Classes;

class ClassFactorySupport final
{
public:
	ClassFactorySupport::ClassFactorySupport()
		: count_(0)
	{ }

	HRESULT _stdcall CreateInstance(_In_ REFCLSID clsid, _Outptr_opt_ IClassFactory** ret);

	bool IsActive() { return InterlockedCompareExchange(&count_, 0u, 0u) != 0; }

	unsigned int AddRef() { PrintDebugLog(L"Start"); return InterlockedIncrement(&count_); }
	unsigned int MinusRef() { PrintDebugLog(L"Start"); return InterlockedDecrement(&count_); }

private:
	volatile unsigned int count_;
};

class SimpleClassFactory final
	: public UnknownObject
	, public IClassFactory
{
public:
	SimpleClassFactory(ClassFactorySupport& support, pair<CLSID const&, function<HRESULT(ClassUnloadContext, IUnknown**)>>& factory)
		: support_(&support)
		, factory_(&factory)
	{ }

	virtual HRESULT _stdcall QueryInterfaceOverride(IID const& iid, _COM_Outptr_ void** ret)
	{
		if (iid == IID_IClassFactory)
		{
			*ret = static_cast<void*>(this);
			return S_OK;
		}
		*ret = nullptr;
		return E_NOINTERFACE;
	}

	virtual HRESULT _stdcall CreateInstance(_In_opt_ IUnknown* pUnkOuter, _In_ REFIID riid, _COM_Outptr_ void** ret) override final
	{
		CheckPointer(ret);
		if (riid != IID_IUnknown) return E_NOINTERFACE;
		return factory_->second(ClassUnloadContext(*support_), reinterpret_cast<IUnknown**>(ret));
	}

	virtual HRESULT _stdcall LockServer(BOOL lock) override final
	{
		lock ? InterlockedIncrement(&count_) : InterlockedDecrement(&count_);
		return S_OK;
	}

	static bool IsLocked() { return InterlockedCompareExchange(&count_, 0, 0) == 0; }

	DECLARE_IUNKNOWN;

private:
	static volatile uint32_t count_;
	ClassFactorySupport* support_;
	pair<CLSID const&, function<HRESULT(ClassUnloadContext, IUnknown**)>>* factory_;
};
volatile uint32_t SimpleClassFactory::count_ = 0;

ClassUnloadContext::ClassUnloadContext(ClassFactorySupport& factory)
	: support_(&factory)
{
	support_->AddRef();
}

ClassUnloadContext::ClassUnloadContext(ClassUnloadContext&& other)
	: support_(other.support_)
{ }

ClassUnloadContext::~ClassUnloadContext()
{
	support_->MinusRef();
}

ClassUnloadContext& ClassUnloadContext::operator=(ClassUnloadContext&& other)
{
	support_ = other.support_;
	return *this;
}

HRESULT ClassFactorySupport::CreateInstance(REFCLSID clsid, IClassFactory** ret)
{
	PrintDebugLog(L"Start");
	for (auto&& c : g_Classes)
	{
		if (c.first == clsid)
		{
			SimpleClassFactory* const factory = new(nothrow) SimpleClassFactory(*this, c);
			if (factory == nullptr) return E_OUTOFMEMORY;

			factory->AddRef();
			*ret = static_cast<IClassFactory*>(factory);
			return S_OK;
		}
	}
	return CLASS_E_CLASSNOTAVAILABLE;
}

HINSTANCE g_hInst;
ClassFactorySupport classFactorySupportInstance;

extern "C"
{
	BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD reason, void* /*reserved*/)
	{
		PrintDebugLog(L"Start");
		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hInstance);
			g_hInst = hInstance;
			break;

		case DLL_PROCESS_DETACH:
			g_hInst = nullptr;
			break;
		}
		return TRUE;
	}

	HRESULT _stdcall DllGetClassObject(_In_ REFCLSID clsid, _In_ REFIID riid, _Outptr_opt_ void** ret)
	{
		PrintDebugLog(L"Start");
		CheckPointer(ret);
		if (riid != IID_IClassFactory) return E_NOINTERFACE;

		*ret = nullptr;
		return classFactorySupportInstance.CreateInstance(clsid, reinterpret_cast<IClassFactory**>(ret));
	}

	HRESULT _stdcall DllCanUnloadNow()
	{
		PrintDebugLog(L"Start");
		if (classFactorySupportInstance.IsActive() || SimpleClassFactory::IsLocked())
		{
			return S_FALSE;
		}
		return S_OK;
	}
}