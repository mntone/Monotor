#pragma once
#include "int24_t.hpp"

namespace AudioMixDownConverter
{

extern ::std::function<void(void const*&, void*&)> GetTransform5chFunction(uint16_t srcBit, uint16_t dstBit, uint16_t dstChannel);

}