#pragma once

#include "tracer.h"
#include "point.h"
#include "matrix.h"
#include "image.h"
#include <ppl.h>
#include <sal.h>
#include <vector>

namespace img_processing
{
	using	byte_t = unsigned char;

	template<typename T>
	struct rect_t
	{
		T p[4];  // left, top, right, bottom
	};

	namespace details
	{
		template<typename T, typename Matrix>
		INLINE rect_t<T> new_dimension(_In_ const T width, _In_ const T height, _In_ const Matrix& mat) NOEXCEPT
		{

			using pt_t = point<T>;
			auto fn = [&mat](auto&& w, auto&& h) NOEXCEPT{ return  pt_round(transform_point(mat, pt_t(w, h))); };

			pt_t pt00, pt01, pt10, pt11;
			concurrency::parallel_invoke([&] { pt00 = fn(0, 0); },
				[&] { pt01 = fn(width, 0);},
				[&] { pt10 = fn(0, height);},
				[&] { pt11 = fn(width, height);}
			);

			auto pt_vec = std::vector < pt_t >{ pt00, pt01, pt10, pt11 };

			auto new_rect = rect_t<T>{};

			new_rect.p[0] = new_rect.p[2] = pt00.x;
			new_rect.p[1] = new_rect.p[3] = pt00.y;

			for (auto it = pt_vec.cbegin() + 1; it != pt_vec.end(); ++it)
			{
				if (new_rect.p[0] > it->x) new_rect.p[0] = it->x;
				if (new_rect.p[1] > it->y) new_rect.p[1] = it->y;
				if (new_rect.p[2] < it->x) new_rect.p[2] = it->x;
				if (new_rect.p[3] < it->y) new_rect.p[3] = it->y;
			}

			return new_rect;
		}
	}


	template<typename T, typename Matrix>
	void transform_pixels(_In_ const image_t<T>& src_img, _Inout_ image_t<T>& dest_img, _In_ const Matrix& in_mat) NOEXCEPT
	{
		ASSERT(dest_img.get() == nullptr);
		ASSERT(src_img.get_height() > 0 && src_img.get_height() < PTRDIFF_MAX);
		ASSERT(src_img.get_width()  > 0 && src_img.get_width()  < PTRDIFF_MAX);

		auto mat = in_mat;
		mat.a31 = mat.a32 = 0;

		auto const src_img_width = static_cast<ptrdiff_t>(src_img.get_width());
		auto const src_img_height = static_cast<ptrdiff_t>(src_img.get_height());

		auto const new_rect = details::new_dimension(src_img_width, src_img_height, mat);
		auto const dim_min = point<ptrdiff_t>{ new_rect.p[0], new_rect.p[1] };
		auto const dim_max = point<ptrdiff_t>{ new_rect.p[2], new_rect.p[3] };

		auto const channel_count = src_img.get_channel_count();
		auto const stride = src_img_width * channel_count;

		auto const new_width = dim_max.x - dim_min.x;
		auto const new_height = dim_max.y - dim_min.y;
		auto const new_img_size = new_width * new_height * channel_count;

		dest_img.allocate(new_width, new_height, channel_count);

		~mat;

		TIMER_INIT
		{
			TIMER_START
#if defined(SIMD)
			const __m128i mm_mask = { 0x00, 0x8F, 0x8F, 0x8F, 0x01, 0x8F, 0x8F, 0x8F, 0x02, 0x8F, 0x8F, 0x8F, 0x03, 0x8F, 0x8F, 0x8F };
#endif

		concurrency::parallel_for(dim_min.y, dim_max.y,[&](auto y) NOEXCEPT
		{
			for (auto x = dim_min.x; x != dim_max.x; ++x)
			{
				auto p0 = point<Matrix::value_type>{ static_cast<Matrix::value_type>(x), static_cast<Matrix::value_type>(y) };
				auto pt = transform_point(mat, p0);

				auto pf = point < ptrdiff_t >{ pt_floor(pt) };
				auto frac = point < Matrix::value_type >{ pt.x - pf.x, pt.y - pf.y };


				if (pf.x < 0 || pf.y < 0 || pf.x >= src_img_width || pf.y >= src_img_height)
				{
					continue;
				}

				byte_t mp[4]{};

				auto src_loc = src_img.get_pixel(pf.x, pf.y);

				auto const w1 = (1 - frac.x) * (1 - frac.y);
				auto const w2 = frac.x   * (1 - frac.y);
				auto const w3 = (1 - frac.x) * frac.y;
				auto const w4 = frac.x * frac.y;

				if (pf.x + 1 < src_img_width)
				{
					if (pf.y + 1 < src_img_height)
					{

#if defined(SIMD)

						auto mm_px1 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_loc)), mm_mask));
						auto mm_px2 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_loc + channel_count)), mm_mask));
						auto mm_px3 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_loc + stride)), mm_mask));
						auto mm_px4 = _mm_cvtepi32_ps(_mm_shuffle_epi8(_mm_lddqu_si128(reinterpret_cast<const __m128i*>(src_loc + stride + channel_count)), mm_mask));

						auto mm_mul1 = _mm_mul_ps(mm_px1, _mm_set1_ps(w1));
						auto mm_mul2 = _mm_mul_ps(mm_px2, _mm_set1_ps(w2));
						auto mm_mul3 = _mm_mul_ps(mm_px3, _mm_set1_ps(w3));
						auto mm_mul4 = _mm_mul_ps(mm_px4, _mm_set1_ps(w4));

						auto mm_mp = _mm_add_ps(_mm_add_ps(mm_mul1, mm_mul2), _mm_add_ps(mm_mul3, mm_mul4));

						mp[0] = static_cast<byte_t>(mm_mp.m128_f32[0]);
						mp[1] = static_cast<byte_t>(mm_mp.m128_f32[1]);
						mp[2] = static_cast<byte_t>(mm_mp.m128_f32[2]);

#else

						mp[0] = static_cast<byte_t>(src_loc[0] * w1 + (src_loc + channel_count)[0] * w2 + (src_loc + stride)[0] * w3 + (src_loc + stride + channel_count)[0] * w4);
						mp[1] = static_cast<byte_t>(src_loc[1] * w1 + (src_loc + channel_count)[1] * w2 + (src_loc + stride)[1] * w3 + (src_loc + stride + channel_count)[1] * w4);
						mp[2] = static_cast<byte_t>(src_loc[2] * w1 + (src_loc + channel_count)[2] * w2 + (src_loc + stride)[2] * w3 + (src_loc + stride + channel_count)[2] * w4);

#endif // defined(SIMD)

					}
					else
					{
						mp[0] = static_cast<byte_t>(src_loc[0] * (1 - frac.x) + (src_loc + channel_count)[0] * frac.x);
						mp[1] = static_cast<byte_t>(src_loc[1] * (1 - frac.x) + (src_loc + channel_count)[1] * frac.x);
						mp[2] = static_cast<byte_t>(src_loc[2] * (1 - frac.x) + (src_loc + channel_count)[2] * frac.x);
					}
				}
				else
				{
					if (pf.y + 1 < src_img_height)
					{
						mp[0] = static_cast<byte_t>(src_loc[0] * (1 - frac.y) + (src_loc + stride)[0] * frac.y);
						mp[1] = static_cast<byte_t>(src_loc[1] * (1 - frac.y) + (src_loc + stride)[1] * frac.y);
						mp[2] = static_cast<byte_t>(src_loc[2] * (1 - frac.y) + (src_loc + stride)[2] * frac.y);
					}
					else
					{
						memcpy_s(mp, sizeof(mp), src_loc, sizeof(mp));
					}
				}

				auto dest_offset = dest_img.get_pixel(x - dim_min.x, y - dim_min.y);
				memcpy_s(dest_offset, channel_count, mp, channel_count);
			}
		}
		);
		TIMER_STOP(L"bilinear sampler end");
		}
	}
}
