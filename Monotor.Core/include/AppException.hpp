#pragma once

namespace Mntone {
namespace Monotor {

class AppException final
{
public:
	AppException(::std::wstring message)
		: message_(message)
	{ }

public:
	::std::wstring Message() const noexcept { return message_; }

private:
	::std::wstring message_;
};

}
}