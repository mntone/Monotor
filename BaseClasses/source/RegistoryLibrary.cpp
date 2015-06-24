#include "pch.hpp"
#include "RegistoryLibrary.hpp"

using namespace std;

RegistoryKey::RegistoryKey(HKEY handle)
	: handle_(handle)
	, needUnload_(true)
{ }

RegistoryKey::RegistoryKey(HKEY handle, wstring name, bool needUnload)
	: handle_(handle)
	, name_(name)
	, needUnload_(needUnload)
{ }

RegistoryKey::~RegistoryKey()
{
	if (needUnload_)
	{
		RegCloseKey(handle_);
	}
}

RegistoryKey RegistoryKey::CreateSubKey(wstring subkey, bool openIfExists)
{
	return CreateSubKey(subkey, RegistoryOptions::None, openIfExists);
}

RegistoryKey RegistoryKey::CreateSubKey(wstring subkey, RegistoryOptions options, bool openIfExists)
{
	HKEY newHandle;
	DWORD disposition = 0;
	LSTATUS ls = RegCreateKeyExW(
		handle_,
		subkey.c_str(),
		0,
		nullptr,
		options == RegistoryOptions::Volatile ? REG_OPTION_VOLATILE : REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		nullptr,
		&newHandle,
		&disposition);
	if (ls != ERROR_SUCCESS) throw HResultException(HRESULT_FROM_WIN32(ls));
	if (!openIfExists && disposition == REG_OPENED_EXISTING_KEY) throw HResultException(E_FAIL);
	return RegistoryKey(newHandle, subkey);
}

RegistoryKey RegistoryKey::OpenSubKey(wstring subkey)
{
	HKEY newHandle;
	LSTATUS ls = RegOpenKeyExW(handle_, subkey.c_str(), 0, KEY_ALL_ACCESS, &newHandle);
	if (ls != ERROR_SUCCESS) throw HResultException(HRESULT_FROM_WIN32(ls));
	return RegistoryKey(newHandle, subkey);
}

void RegistoryKey::DeleteSubKey(wstring subkey, bool happenException)
{
	LSTATUS ls = RegDeleteKeyW(handle_, subkey.c_str());
	if (happenException && ls != ERROR_SUCCESS) throw HResultException(HRESULT_FROM_WIN32(ls));
}

void RegistoryKey::DeleteSubKeyTree(wstring subkey, bool happenException)
{
	RegistoryKey targetSubKey = OpenSubKey(subkey);
	for (auto&& childSubKeyName : targetSubKey.GetSubKeyNames())
	{
		RegistoryKey childSubKey = targetSubKey.OpenSubKey(childSubKeyName);
		for (auto&& childChildSubKeyName : childSubKey.GetSubKeyNames())
		{
			childSubKey.DeleteSubKeyTree(childChildSubKeyName, happenException);
		}
	}
}

vector<wstring> RegistoryKey::GetSubKeyNames()
{
	vector<wstring> ret;
	LSTATUS ls = ERROR_SUCCESS;
	uint32_t index = 0;
	while (index <= numeric_limits<uint32_t>::max())
	{
		wchar_t buffer[512];
		DWORD length = sizeof(wchar_t) * _ARRAYSIZE(buffer);
		FILETIME filetime = { 0 };
		ls = RegEnumKeyExW(handle_, index++, buffer, &length, nullptr, nullptr, nullptr, &filetime);
		assert(ls == ERROR_SUCCESS || ls == ERROR_NO_MORE_ITEMS);
		if (ls != ERROR_SUCCESS) break;
		ret.emplace_back(buffer, length);
	}
	return move(ret);
}

vector<wstring> RegistoryKey::GetValueNames()
{
	vector<wstring> ret;
	LSTATUS ls = ERROR_SUCCESS;
	uint32_t index = 0;
	while (index <= numeric_limits<uint32_t>::max())
	{
		wchar_t buffer[512];
		DWORD length = sizeof(wchar_t) * _ARRAYSIZE(buffer);
		FILETIME filetime = { 0 };
		ls = RegEnumValueW(handle_, index++, buffer, &length, nullptr, nullptr, nullptr, nullptr);
		assert(ls == ERROR_SUCCESS || ls == ERROR_NO_MORE_ITEMS);
		if (ls != ERROR_SUCCESS) break;
		ret.emplace_back(buffer, length);
	}
	return ret;
}

void RegistoryKey::SetValue(wstring name, wstring value)
{
	LSTATUS ls = RegSetValueExW(handle_, name.c_str(), 0, REG_SZ, reinterpret_cast<BYTE const*>(value.c_str()), sizeof(wchar_t) * value.length());
	if (ls != ERROR_SUCCESS) throw HResultException(HRESULT_FROM_WIN32(ls));
}

void RegistoryKey::SetValue(wstring name, uint32_t value)
{
	LSTATUS ls = RegSetValueExW(handle_, name.c_str(), 0, REG_DWORD, reinterpret_cast<BYTE const*>(&value), sizeof(::std::uint32_t));
	if (ls != ERROR_SUCCESS) throw HResultException(HRESULT_FROM_WIN32(ls));
}

void RegistoryKey::SetValue(wstring name, vector<uint8_t> value)
{
	LSTATUS ls = RegSetValueExW(handle_, name.c_str(), 0, REG_BINARY, reinterpret_cast<BYTE const*>(value.data()), value.size());
	if (ls != ERROR_SUCCESS) throw HResultException(HRESULT_FROM_WIN32(ls));
}

void RegistoryKey::DeleteValue(wstring name, bool happenException)
{
	LSTATUS ls = RegDeleteValueW(handle_, name.c_str());
	if (happenException && ls != ERROR_SUCCESS) throw HResultException(HRESULT_FROM_WIN32(ls));
}