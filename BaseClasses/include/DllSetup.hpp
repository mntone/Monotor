#pragma once

extern HINSTANCE g_hInst;

class ClassFactorySupport;
class ClassUnloadContext final
{
public:
	ClassUnloadContext(ClassFactorySupport& factory);
	ClassUnloadContext(ClassUnloadContext&&);

	~ClassUnloadContext();

	ClassUnloadContext& operator=(ClassUnloadContext&&);

private:
	ClassUnloadContext() = delete;
	ClassUnloadContext(ClassUnloadContext const&) = delete;

	ClassUnloadContext& operator=(ClassUnloadContext const&) = delete;

private:
	ClassFactorySupport* support_;
};