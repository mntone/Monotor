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