#pragma once

namespace Mntone {
namespace ComSupport {

class ComInitializeWrapper final
{
public:
	ComInitializeWrapper(DWORD flags = COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
	~ComInitializeWrapper();
};

}
}