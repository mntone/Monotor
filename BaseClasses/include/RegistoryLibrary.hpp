#pragma once

class HResultException final
{
public:
	HResultException(HRESULT hr)
		: hresult_(hr)
	{ }

	HResultException(HRESULT hr, ::std::wstring message)
		: hresult_(hr)
		, message_(message)
	{ }

	HRESULT HResult() const { return hresult_; }
	::std::wstring Message() const { return message_; }

private:
	HRESULT hresult_;
	::std::wstring message_;
};

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

class Registory final
{
public:
	static RegistoryKey ClassesRoot() { return RegistoryKey(HKEY_CLASSES_ROOT, L"HKEY_CLASSES_ROOT", false); }
	static RegistoryKey CurrentConfig() { return RegistoryKey(HKEY_CURRENT_CONFIG, L"HKEY_CURRENT_CONFIG", false); }
	static RegistoryKey CurrentUser() { return RegistoryKey(HKEY_CURRENT_USER, L"HKEY_CURRENT_USER", false); }
};