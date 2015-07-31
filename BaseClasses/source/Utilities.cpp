#include "pch.hpp"
#include "Utilties.hpp"

#include "RegistoryLibrary.hpp"

using namespace std;

#if _DEBUG
void _PrintDebugLog(wchar_t const* message, char const* func, char const* filename, int line)
{
	using namespace ::std;
	using namespace ::std::chrono;

	auto time = system_clock::now();
	typedef duration<int, ratio<60 * 60 * 24>> days;
	auto day = time_point_cast<days>(time);
	auto hour = time_point_cast<hours>(time - day.time_since_epoch());
	auto min = time_point_cast<minutes>(time - day.time_since_epoch() - hour.time_since_epoch());
	auto sec = time_point_cast<seconds>(time - day.time_since_epoch() - hour.time_since_epoch() - min.time_since_epoch());
	auto ms = time_point_cast<milliseconds>(time - day.time_since_epoch() - hour.time_since_epoch() - min.time_since_epoch() - sec.time_since_epoch());
	string sfn = filename;
	sfn = sfn.substr(sfn.find_last_of('\\') + 1);
	wstringstream buf;
	buf << L'['
		<< hour.time_since_epoch().count()
		<< L':'
		<< (min.time_since_epoch().count() < 10 ? L"0" : L"")
		<< min.time_since_epoch().count()
		<< L':'
		<< (sec.time_since_epoch().count() < 10 ? L"0" : L"")
		<< sec.time_since_epoch().count()
		<< L'.'
		<< (ms.time_since_epoch().count() < 10 ? L"00" : (ms.time_since_epoch().count() < 100 ? L"0" : L""))
		<< ms.time_since_epoch().count()
		<< L"] "
		<< func
		<< L" <"
		<< message
		<< L"> ["
		<< sfn.c_str()
		<< L", line: "
		<< line
		<< L']'
		<< endl;
	OutputDebugStringW(buf.str().c_str());
}
#endif

wstring GetGuidString(GUID guid)
{
	OLECHAR buffer[CHARS_IN_GUID];
	HRESULT hr = StringFromGUID2(guid, buffer, CHARS_IN_GUID);
	assert(hr);
	return ::std::wstring(buffer);
}

wstring GetAssemblyName(HINSTANCE hinstance)
{
	wchar_t buffer[512];
	DWORD length = GetModuleFileNameW(hinstance, buffer, sizeof(buffer));
	assert(length != 0);
	return wstring(buffer, length);
}

extern "C" {

	HRESULT _stdcall CreateMemoryAllocator(_Outptr_opt_ IMemAllocator*& memAllocator)
	{
		CheckPointer(memAllocator);

		return CoCreateInstance(CLSID_MemoryAllocator, nullptr, CLSCTX_INPROC_SERVER, IID_IMemAllocator, reinterpret_cast<void**>(memAllocator));
	}

	HRESULT _stdcall GetGlobalString(::std::wstring source, wchar_t** ret)
	{
		CheckPointer(ret);

		*ret = nullptr;

		auto len = source.length() + sizeof(wchar_t);
		auto str = static_cast<wchar_t*>(CoTaskMemAlloc(len));
		if (str == nullptr)
		{
			return E_OUTOFMEMORY;
		}

		memcpy_s(str, len, source.c_str(), source.length());
		str[source.length()] = 0;

		*ret = str;
		return S_OK;
	}

	HRESULT _stdcall SetupRegisterServer(CLSID clsid, wstring description, wstring filename, wstring threadingModel, wstring serverType)
	{
		auto clsidString = GetGuidString(clsid);
		try
		{
			auto classesRootRegistory = Registory::ClassesRoot();
			auto selfClsidSubKey = classesRootRegistory.CreateSubKey(L"CLSID\\" + clsidString);
			selfClsidSubKey.SetValue(L"", description);

			auto serverTypeSubKey = selfClsidSubKey.CreateSubKey(serverType);
			serverTypeSubKey.SetValue(L"", filename);
			serverTypeSubKey.SetValue(L"ThreadingModel", threadingModel);
		}
		catch (HResultException& hex)
		{
			return hex.HResult();
		}

		return S_OK;
	}

	HRESULT _stdcall SetupUnregisterServer(CLSID clsid)
	{
		auto clsidString = GetGuidString(clsid);
		try
		{
			auto classesRootRegistory = Registory::ClassesRoot();
			auto clsidSubKey = classesRootRegistory.OpenSubKey(L"CLSID");
			clsidSubKey.DeleteSubKeyTree(clsidString, false);
		}
		catch (HResultException& hex)
		{
			return hex.HResult();
		}

		return S_OK;
	}

}