#pragma once
#include "HResultException.hpp"

enum class RegistoryOptions
{
	None = 0,
	Volatile,
};

class RegistoryKey final
{
	friend class Registory;

public:
	RegistoryKey(HKEY handle);
	RegistoryKey(RegistoryKey&&) = default;
	~RegistoryKey();

	RegistoryKey& operator=(RegistoryKey&&) = default;

	RegistoryKey CreateSubKey(::std::wstring subkey, bool openIfExists = true);
	RegistoryKey CreateSubKey(::std::wstring subkey, RegistoryOptions options, bool openIfExists = true);
	RegistoryKey OpenSubKey(::std::wstring subkey);
	void DeleteSubKey(::std::wstring subkey, bool happenException = true);
	void DeleteSubKeyTree(::std::wstring subkey, bool happenException = true);
	::std::vector<::std::wstring> GetSubKeyNames();

	template<typename T> auto GetValue(::std::wstring name, ::std::size_t size = 256) const -> typename ::std::enable_if<::std::is_same<::std::wstring, T>::value, T>::type;
	template<typename T> auto GetValue(::std::wstring name) const -> typename ::std::enable_if<::std::is_same<::std::uint32_t, T>::value, T>::type;
	template<typename T> auto GetValue(::std::wstring name, ::std::size_t size = 256) const -> typename ::std::enable_if<::std::is_same<::std::vector<::std::uint8_t>, T>::value, T>::type;

	::std::vector<::std::wstring> GetValueNames();
	void SetValue(::std::wstring name, ::std::wstring value);
	void SetValue(::std::wstring name, ::std::uint32_t value);
	void SetValue(::std::wstring name, ::std::vector<::std::uint8_t> value);
	void DeleteValue(::std::wstring name, bool happenException = true);

	static RegistoryKey FromHandle(HKEY handle) { return RegistoryKey(handle); }

public:
	HKEY Handle() const { return handle_; }
	::std::wstring Name() const { return name_; }

private:
	RegistoryKey(HKEY handle, ::std::wstring name, bool needUnload = true);

	RegistoryKey(RegistoryKey const&) = delete;

	RegistoryKey& operator=(RegistoryKey const&) = delete;

private:
	bool const needUnload_;
	HKEY handle_;
	::std::wstring name_;
};

template<> ::std::wstring RegistoryKey::GetValue<::std::wstring>(::std::wstring name, ::std::size_t) const;
template<> ::std::uint32_t RegistoryKey::GetValue<::std::uint32_t>(::std::wstring name) const;
template<> ::std::vector<::std::uint8_t> RegistoryKey::GetValue<::std::vector<::std::uint8_t>>(::std::wstring name, ::std::size_t size) const;

class Registory final
{
public:
	static RegistoryKey ClassesRoot() { return RegistoryKey(HKEY_CLASSES_ROOT, L"HKEY_CLASSES_ROOT", false); }
	static RegistoryKey CurrentConfig() { return RegistoryKey(HKEY_CURRENT_CONFIG, L"HKEY_CURRENT_CONFIG", false); }
	static RegistoryKey CurrentUser() { return RegistoryKey(HKEY_CURRENT_USER, L"HKEY_CURRENT_USER", false); }
};