#pragma once

#include "tracer.h"
#include "point.h"

namespace img_processing
{
	template<typename T>
	class matrix3x2
	{
		INLINE void set_product(
			const matrix3x2 &a,
			const matrix3x2 &b
			) NOEXCEPT
		{
			a11 = a.a11 * b.a11 + a.a12 * b.a21;
			a12 = a.a11 * b.a12 + a.a12 * b.a22;
			a21 = a.a21 * b.a11 + a.a22 * b.a21;
			a22 = a.a21 * b.a12 + a.a22 * b.a22;
			a31 = a.a31 * b.a11 + a.a32 * b.a21 + b.a31;
			a32 = a.a31 * b.a12 + a.a32 * b.a22 + b.a32;
		}

	public:
		using value_type = T;

		static_assert(std::is_floating_point<T>::value, "matrix can only be instantiated with floating point types");

		matrix3x2() NOEXCEPT  : a11{ 1 }, a12{ 0 }, a21{ 0 }, a22{ 1 }, a31{ 0 }, a32{ 0 }
		{}

		matrix3x2(T A11, T A12, T A21, T A22, T A31, T A32) NOEXCEPT : a11{ A11 }, a12{ A12 }, a21{ A21 }, a22{ A22 }, a31{ A31 }, a32{ A32 }
		{}

		matrix3x2(const matrix3x2& mat) NOEXCEPT : a11{ mat.a11 }, a12{ mat.a12 }, a21{ mat.a21 }, a22{ mat.a22 }, a31{ mat.a31 }, a32{ mat.a32 }
		{}

		INLINE matrix3x2& operator=(const matrix3x2& mat) NOEXCEPT
		{
			a11 = mat.a11;
			a12 = mat.a12;
			a21 = mat.a21;
			a22 = mat.a22;
			a31 = mat.a31;
			a32 = mat.a32;
			return *this;
		}

		INLINE matrix3x2& operator*=(const matrix3x2& mat) NOEXCEPT
		{
			*this = (*this) * mat;
			return *this;
		}

		INLINE matrix3x2& operator*(const matrix3x2& mat) NOEXCEPT
		{
			this->set_product(*this, mat);

			return *this;
		}

		INLINE matrix3x2& operator~() NOEXCEPT
		{
			auto inv_mat = inverse();
			*this = inv_mat;
			return *this;
		}

		static matrix3x2 rotation(T radians, point<T>& point) NOEXCEPT
		{
			T c = std::cos(radians);
			T s = std::sin(radians);

			T a31 = point.x  * (1 - radians) + point.y * std::sin(radians);
			T a32 = point.x  * -std::sin(radians) + point.y * (1 - std::cos(radians));


			return matrix3x2(c, s, -s, c, a31, a32);
		}

		static matrix3x2 rotation(T radians, T x = 0, T y = 0) NOEXCEPT
		{
			T c = std::cos(radians);
			T s = std::sin(radians);

			T a31 = x  * (1 - radians) + y * std::sin(radians);
			T a32 = x  * -std::sin(radians) + y * (1 - std::cos(radians));

			return matrix3x2(c, s, -s, c, a31, a32);
		}

		static matrix3x2 translation(const point<T>& t) NOEXCEPT
		{
			return matrix3x2(1, 0, 0, 1, t.x, t.y);
		}

		static matrix3x2 translation(T x, T y) NOEXCEPT
		{
			return matrix3x2(1, 0, 0, 1, x, y);
		}

		static matrix3x2 scale(const point<T>& s) NOEXCEPT
		{
			return matrix3x2(s.x, 0, 0, s.y, 0, 0);
		}

		static matrix3x2 scale(T x, T y) NOEXCEPT
		{
			return matrix3x2(x, 0, 0, y, 0, 0);
		}

		static matrix3x2 scale(T s) NOEXCEPT
		{
			return matrix3x2(s, 0, 0, s, 0, 0);
		}

		static matrix3x2 skew(T radX, T radY, const point<T>& center = point <T>{}) NOEXCEPT
		{
			T a12 = std::tan(radY);
			T a21 = std::tan(radX);
			T a31 = -(center.x) * a21;
			T a32 = -(center.y) * a12;

			return matrix3x2(1, a12, a21, 1, a31, a32);
		}

		static matrix3x2 identity() NOEXCEPT
		{
			return matrix3x2{};
		}

		INLINE matrix3x2 inverse() NOEXCEPT
		{
			T det = a11*a22 - a21*a12;

			T a11_n = a22 / det;
			T a12_n = -(a12) / det;
			T a21_n = -(a21) / det;
			T a22_n = a11 / det;
			T a31_n = (a32*a21 - a31*a22) / det;
			T a32_n = -(a32*a11 - a31*a12) / det;

			return matrix3x2(a11_n, a12_n, a21_n, a22_n, a31_n, a32_n);
		}

		T a11, a12, a21;
		T a22, a31, a32;


	};

	template<typename T, typename F>
	INLINE point<F> operator*(const point<T>& p, const matrix3x2<F>& m) NOEXCEPT
	{
		return point<F>(m.a11*p.x + m.a21*p.y + m.a31, m.a12*p.x + m.a22*p.y + m.a32);
	}

	template<typename F, typename F2>
	INLINE point<F> transform_point(const matrix3x2<F>& mat, const point<F2>& src) NOEXCEPT
	{
		return src * mat;
	}
}