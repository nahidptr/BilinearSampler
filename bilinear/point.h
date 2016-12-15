#pragma once

#include <numeric>
#include "tracer.h"

namespace img_processing
{
	template<typename T>
	class point
	{
	public:
		using value_type = T;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;

		T x;
		T y;

		point() NOEXCEPT : x{}, y{}
		{}

		point(T newX, T newY) NOEXCEPT : x{ newX }, y{ newY }
		{}

		point(const point& rhs) NOEXCEPT : x{ rhs.x }, y{ rhs.y }
		{}

		~point() = default;

		INLINE point& operator=(const point& rhs) NOEXCEPT
		{
			x = rhs.x;
			y = rhs.y;
			return *this;
		}

		INLINE point& operator+=(const point& rhs) NOEXCEPT
		{
			x += rhs.x;
			y += rhs.y;
			return *this;
		}

		INLINE point& operator-=(const point& rhs) NOEXCEPT
		{
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}

	};

	template<typename T>
	INLINE bool operator==(const point<T>& lhs, const point<T>& rhs) NOEXCEPT
	{
		return (lhs.x == rhs.x && lhs.y == rhs.y);
	}

	template<typename T>
	INLINE bool operator!=(const point<T>& lhs, const point<T>& rhs) NOEXCEPT
	{
		return !(lhs == rhs);
	}


	template<typename T>
	INLINE std::ptrdiff_t pt_round(T x) NOEXCEPT
	{
		T add = static_cast<T>(0.5);
		return static_cast<ptrdiff_t>(x + (x < 0.0f ? -add : add));
	}

	template<typename T>
	INLINE point<ptrdiff_t> pt_round(const point<T>& pt) NOEXCEPT
	{
		return point<ptrdiff_t>(pt_round(pt.x), pt_round(pt.y));
	}

	template<typename T>
	INLINE std::ptrdiff_t pt_floor(T x) NOEXCEPT
	{
		return static_cast<ptrdiff_t>(std::floor(x));
	}

	template<typename T>
	INLINE point<ptrdiff_t> pt_floor(const point<T>& pt) NOEXCEPT
	{
		return point<ptrdiff_t>(pt_floor(pt.x), pt_floor(pt.y));
	}
}
 