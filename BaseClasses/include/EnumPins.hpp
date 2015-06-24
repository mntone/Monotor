#pragma once

class EnumPinsFactory final
{
public:
	static HRESULT _stdcall CreateInstance(_In_ IPin* pin1, _Out_ IEnumPins** ret);
	static HRESULT _stdcall CreateInstance(_In_ IPin* pin1, _In_ IPin* pin2, _Out_ IEnumPins** ret);
};