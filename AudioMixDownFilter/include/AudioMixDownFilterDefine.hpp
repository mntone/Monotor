#pragma once

#ifdef AUDIOMIXDOWNFILTER_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif


namespace {

const wchar_t* const NAME_AudioMixDownFilter = L"Audio Mix-down Filter (mntone)";

}