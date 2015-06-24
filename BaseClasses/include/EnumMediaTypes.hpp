#pragma once

class EnumMediaTypesFactory final
{
public:
	static HRESULT _stdcall CreateInstance(_In_ ::std::vector<AM_MEDIA_TYPE*> types, _Out_ IEnumMediaTypes** ret);
};