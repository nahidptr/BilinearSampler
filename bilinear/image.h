#pragma once
 
#include "tracer.h"

namespace img_processing
{
	template<typename T>
	class image_iterator
	{
	public:
		using container_type = T;
		using value_type = typename T::value_type;
		using pointer = typename T::pointer;
		using const_pointer = typename T::const_pointer;
		using reference = typename T::reference;
		using const_reference = typename T::const_reference;
		using size_type = typename T::size_type;
		using difference_type = typename T::difference_type;
		using self_type = image_iterator;

		image_iterator(container_type* base_img, size_t start_pos) NOEXCEPT : base_img_{ base_img }, pos_{ start_pos }
		{
		}

		INLINE self_type at_x(difference_type offset)  NOEXCEPT
		{
			self_type tmp = (*this);
			tmp.pos_ = pos_ + offset * base_img_->channel_count_;
			return tmp;
		}

		INLINE self_type at_y(difference_type offset)  NOEXCEPT
		{
			self_type tmp = (*this);
			tmp.pos_ = pos_ + (base_img_->width_ * base_img_->channel_count_) + offset * base_img_->channel_count_;
			return tmp;
		}


		INLINE decltype(auto) operator*() NOEXCEPT
		{
			return (*base_img_)[pos_];
		}


		INLINE decltype(auto) operator->() NOEXCEPT
		{
			return this;
		}

		INLINE decltype(auto) operator[](size_type index) NOEXCEPT
		{
			return ((*base_img_)[pos_])[index];   //returns byte_t* then index into that
		}

		INLINE self_type& operator++() NOEXCEPT
		{
			pos_ += base_img_->channel_count_;
			return *this;
		}

		INLINE self_type operator++(int) NOEXCEPT
		{
			self_type tmp = (*this);
			++(*this);
			return tmp;
		}

		INLINE self_type operator+(difference_type n) NOEXCEPT
		{
			self_type tmp = (*this);
			tmp.pos_ += (n * base_img_->channel_count_);
			return tmp;
		}

		INLINE self_type& operator+=(difference_type n) NOEXCEPT
		{
			pos_ += (n * base_img_->channel_count_);
			return *this;
		}

		INLINE self_type& operator--() NOEXCEPT
		{
			pos_ -= base_img_->channel_count_;
			return *this;
		}

		INLINE self_type operator--(int) NOEXCEPT
		{
			self_type tmp = (*this);
			--(*this);
			return tmp;
		}

		INLINE bool operator==(const self_type &other)  const NOEXCEPT
		{
			return _img == other._img;
		}

		INLINE bool operator!=(const self_type &other)  const NOEXCEPT
		{
			!(*this == other);
		}

	private:
		container_type*		base_img_;
		size_t				pos_;
	};


	template<typename T>
	class image_t
	{
	public:
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using self_type = image_t;
		using iterator = image_iterator<self_type>;
		using const_iterator = image_iterator<const image_t>;

		template<typename U>
		friend class image_iterator;


		INLINE iterator begin() NOEXCEPT
		{
			return iterator(this, 0);
		}
		INLINE iterator end() NOEXCEPT
		{
			return iterator(this, size());
		}

		INLINE const_iterator begin() const NOEXCEPT
		{
			return const_iterator(this, 0);
		}

		INLINE iterator end() const NOEXCEPT
		{
			return const_iterator(this, size());
		}

		INLINE size_type size() const NOEXCEPT
		{
			return width_*height_*channel_count_;
		}

		INLINE size_type get_width() const NOEXCEPT
		{
			return width_;
		}

		INLINE size_type get_height() const NOEXCEPT
		{
			return height_;
		}

		INLINE size_type get_channel_count() const NOEXCEPT
		{
			return channel_count_;
		}

		explicit image_t(const size_type width = 0, size_type const height = 0, size_type const channel_count = 4) : width_{ width }, height_{ height },
			channel_count_{ channel_count }, source_{ nullptr },
			is_referenced_{ false }
		{
		}

		image_t(const image_t&) = delete;
		auto operator=(const image_t&)->image_t& = delete;

		image_t(const image_t&& rhs) NOEXCEPT : width_{ rhs.width_ }, height_{ rhs.height_ }, channel_count_{ rhs.channel_count_ }, source_{ rhs.source_ }, is_referenced_{ rhs.is_referenced_ }
		{
			rhs.source_ = nullptr;
			rhs.width_ = rhs.height_ = rhs.channel_count_ = 0;
			rhs.is_referenced_ = false;
		}

		auto operator=(image_t&& rhs) NOEXCEPT -> image_t&
		{
			if (!is_referenced_)
			{
				delete[] source_;
			}

			source_ = rhs.source_;
			rhs.source_ = nullptr;

			width_ = rhs.width_;
			height = rhs.height_;
			channel_count_ = rhs.channel_count_;

			return *this;
		}

		INLINE auto allocate(size_type width, size_type height, size_type channel_count)
		{
			ASSERT(width > 0 && height > 0 && channel_count > 0);

			width_ = width;
			height_ = height;
			channel_count_ = channel_count;

			ASSERT(source_ == nullptr);
			ASSERT(!is_referenced_);

			source_ = new T[this->size()];
		}

		INLINE auto copy_from(pointer src_ptr, size_type size)
		{
			ASSERT(source_ == nullptr);
			ASSERT(this->size() == size);
			allocate(size);
			memcpy_s(source_, this->size(), src_ptr, size);
		}

		INLINE auto reference_from(pointer src_ptr) NOEXCEPT
		{
			ASSERT(source_ == nullptr);
			source_ = src_ptr;
			is_referenced_ = true;
		}

		INLINE value_type* get() const NOEXCEPT
		{
			return source_;
		}

		INLINE value_type** get_address_of() NOEXCEPT
		{
			return &source_;
		}

		~image_t() NOEXCEPT
		{
			if (source_  && !is_referenced_)
			{
				TRACE(L"img dtor is_ref = false\n");
				delete[] source_;
			}
		}

		INLINE value_type* get_pixel(difference_type x, difference_type y) const NOEXCEPT
		{
			return source_ + (y * width_ + x) * channel_count_;
		}

		INLINE pointer operator[](size_type index) NOEXCEPT
		{
			return (source_ + index);
		}

		INLINE const_pointer operator[](size_t index) const NOEXCEPT
		{
			return (source_ + index);
		}

	private:
		value_type*			source_;
		size_type			width_;
		size_type			height_;
		size_type			channel_count_;
		bool				is_referenced_;
	};
}
