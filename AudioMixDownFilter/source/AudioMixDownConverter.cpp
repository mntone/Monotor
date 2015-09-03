#include "pch.hpp"
#include "AudioMixDownConverter.hpp"

using namespace std;

constexpr double Pow(double b, int64_t e)
{
	return e != 1 ? b * Pow(b, e - 1) : b;
}

template<typename T>
constexpr T SqrtAux(T s, T x, T prev)
{
	return x != prev ? SqrtAux(s, (x + s / x) / 2.0, x) : x;
}

template<typename T>
constexpr T Sqrt(T s)
{
	return SqrtAux(s, s / 2.0, s);
}

template<typename T>
constexpr void Transform5chTo1ch(T a, T k, T shift, T fl, T fr, T fc, T bl, T br, T* c2)
{
	*c2 = (k * (((fl + fc + fr) << shift) + a * (bl + br))) >> shift;
}

template<typename T>
void Transform5chTo2ch(T a, T k, T shift, double shiftD, T fl, T fr, T fc, T bl, T br, T* l2, T* r2)
{
	double os2 = shiftD / Sqrt(2.0);
	T os2i = static_cast<T>(os2);
	*l2 = (k * ((fl << shift) + fc * os2i + a * bl)) >> shift;
	*r2 = (k * ((fr << shift) + fc * os2i + a * br)) >> shift;
}


template<typename S, typename D, int16_t C>
void Transform5chTo(void const*& src, void*& dst);


template<>
void Transform5chTo<int16_t, int16_t, 1>(void const*& src, void*& dst)
{
	int16_t const*& s = *reinterpret_cast<int16_t const**>(&src);
	int16_t*& d = *reinterpret_cast<int16_t**>(&dst);

	constexpr int32_t shift = 7;
	constexpr double sh = Pow(2, shift);
	constexpr double a = sh / Sqrt(2.0);
	constexpr double k = sh / (1.0 + 1.0 / Sqrt(2.0) + a);
	constexpr int32_t ai = static_cast<int32_t>(a);
	constexpr int32_t ki = static_cast<int32_t>(k);

	int32_t c = 0;
	Transform5chTo1ch<int32_t>(ai, ki, shift, s[0], s[1], s[3], s[4], s[5], &c);
	d[0] = static_cast<int16_t>(c);

	s += 6;
	d += 1;
}

template<>
void Transform5chTo<int16_t, int16_t, 2>(void const*& src, void*& dst)
{
	int16_t const*& s = *reinterpret_cast<int16_t const**>(&src);
	int16_t*& d = *reinterpret_cast<int16_t**>(&dst);

	constexpr int32_t shift = 7;
	constexpr double sh = Pow(2, shift);
	constexpr double a = sh / Sqrt(2.0);
	constexpr double k = sh / (1.0 + 1.0 / Sqrt(2.0) + a);
	constexpr int32_t ai = static_cast<int32_t>(a);
	constexpr int32_t ki = static_cast<int32_t>(k);

	int32_t l = 0, r = 0;
	Transform5chTo2ch<int32_t>(ai, ki, shift, sh, s[0], s[1], s[3], s[4], s[5], &l, &r);
	d[0] = static_cast<int16_t>(l);
	d[1] = static_cast<int16_t>(r);

	s += 6;
	d += 2;
}

template<>
void Transform5chTo<int32_t, int16_t, 1>(void const*& src, void*& dst)
{
	int32_t const*& s = *reinterpret_cast<int32_t const**>(&src);
	int16_t*& d = *reinterpret_cast<int16_t**>(&dst);

	constexpr int64_t shift = 14;
	constexpr double sh = Pow(2, shift);
	constexpr double a = sh / Sqrt(2.0);
	constexpr double k = sh / (1.0 + 1.0 / Sqrt(2.0) + a);
	constexpr int64_t ai = static_cast<int64_t>(a);
	constexpr int64_t ki = static_cast<int64_t>(k);

	int64_t c = 0;
	Transform5chTo1ch<int64_t>(ai, ki, shift, s[0], s[1], s[3], s[4], s[5], &c);
	d[0] = static_cast<int16_t>(c);

	s += 6;
	d += 1;
}

template<>
void Transform5chTo<int32_t, int16_t, 2>(void const*& src, void*& dst)
{
	int32_t const*& s = *reinterpret_cast<int32_t const**>(&src);
	int16_t*& d = *reinterpret_cast<int16_t**>(&dst);

	constexpr int64_t shift = 14;
	constexpr double sh = Pow(2, shift);
	constexpr double a = sh / Sqrt(2.0);
	constexpr double k = sh / (1.0 + 1.0 / Sqrt(2.0) + a);
	constexpr int64_t ai = static_cast<int64_t>(a);
	constexpr int64_t ki = static_cast<int64_t>(k);

	int64_t l = 0, r = 0;
	Transform5chTo2ch<int64_t>(ai, ki, shift, sh, s[0], s[1], s[3], s[4], s[5], &l, &r);
	d[0] = static_cast<int16_t>(l);
	d[1] = static_cast<int16_t>(r);

	s += 6;
	d += 2;
}

template<>
void Transform5chTo<int32_t, int32_t, 1>(void const*& src, void*& dst)
{
	int32_t const*& s = *reinterpret_cast<int32_t const**>(&src);
	int32_t*& d = *reinterpret_cast<int32_t**>(&dst);

	constexpr int64_t shift = 14;
	constexpr double sh = Pow(2, shift);
	constexpr double a = sh / Sqrt(2.0);
	constexpr double k = sh / (1.0 + 1.0 / Sqrt(2.0) + a);
	constexpr int64_t ai = static_cast<int64_t>(a);
	constexpr int64_t ki = static_cast<int64_t>(k);

	int64_t c = 0;
	Transform5chTo1ch<int64_t>(ai, ki, shift, s[0], s[1], s[3], s[4], s[5], &c);
	d[0] = static_cast<int32_t>(c);

	s += 6;
	d += 1;
}

template<>
void Transform5chTo<int32_t, int32_t, 2>(void const*& src, void*& dst)
{
	int32_t const*& s = *reinterpret_cast<int32_t const**>(&src);
	int32_t*& d = *reinterpret_cast<int32_t**>(&dst);

	constexpr int64_t shift = 14;
	constexpr double sh = Pow(2, shift);
	constexpr double a = sh / Sqrt(2.0);
	constexpr double k = sh / (1.0 + 1.0 / Sqrt(2.0) + a);
	constexpr int64_t ai = static_cast<int64_t>(a);
	constexpr int64_t ki = static_cast<int64_t>(k);

	int64_t l = 0, r = 0;
	Transform5chTo2ch<int64_t>(ai, ki, shift, sh, s[0], s[1], s[3], s[4], s[5], &l, &r);
	d[0] = static_cast<int32_t>(l);
	d[1] = static_cast<int32_t>(r);

	s += 6;
	d += 2;
}

function<void(void const*&, void*&)> AudioMixDownConverter::GetTransform5chFunction(uint16_t srcBit, uint16_t dstBit, uint16_t dstChannel)
{
	switch (srcBit)
	{
	case 16:
		switch (dstBit)
		{
		case 16: return dstChannel == 2 ? Transform5chTo<int16_t, int16_t, 2> : Transform5chTo<int16_t, int16_t, 1>;
		}
		break;

	case 32:
		switch (dstBit)
		{
		case 16: return dstChannel == 2 ? Transform5chTo<int32_t, int16_t, 2> : Transform5chTo<int32_t, int16_t, 1>;
		case 32: return dstChannel == 2 ? Transform5chTo<int32_t, int32_t, 2> : Transform5chTo<int32_t, int32_t, 1>;
		}
		break;
	}
	throw exception();
}