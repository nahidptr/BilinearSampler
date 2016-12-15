#pragma once
 
#include <windows.h>
#include <crtdbg.h>

#define ASSERT _ASSERTE 

struct Tracer {
	unsigned		m_line;
	const char*		m_filename;

	Tracer(const char* filename, unsigned const line) : m_line{ line }, m_filename{ filename }
	{}

	template<typename... Args>
	auto operator()(wchar_t const* format, Args... args) const -> void {

		wchar_t buffer[256];

		auto count = 0;
		ASSERT(-1 != count);
		ASSERT(-1 != _snwprintf_s(buffer + count, _countof(buffer) - count, _countof(buffer) - count - 1,
			format, args...));

		::OutputDebugStringW(buffer);
	}

};


#ifdef _DEBUG 
#define TRACE Tracer(__FILE__, __LINE__)
#else 
#define TRACE __noop
#endif

#ifdef _DEBUG 
#define VERIFY(x) ASSERT(x)
#define VERIFY_(x,y) ASSERT(x==y)
#else 
#define VERIFY(x) (x)
#define VERIFY_(x, y) (x)
#endif



#ifdef _DEBUG 

#define TIMER_INIT															\
    LARGE_INTEGER frequency;												\
    LARGE_INTEGER t1,t2;													\
    double elapsedTime;														\
    ::QueryPerformanceFrequency(&frequency);


#define TIMER_START ::QueryPerformanceCounter(&t1);


#define TIMER_STOP(message)													\
    ::QueryPerformanceCounter(&t2);											\
    elapsedTime=(double)(t2.QuadPart-t1.QuadPart)/frequency.QuadPart;		\
    TRACE(L"ElapsedTime  %ls %lf \n", message, elapsedTime);

#else

#define TIMER_INIT	__noop;														
#define TIMER_START __noop;
#define TIMER_STOP(x) __noop;													

#endif


namespace img_processing
{

#define INLINE __forceinline

#if _MSC_VER  > 1800 
#define NOEXCEPT noexcept
#else
#define NOEXCEPT throw()
#endif

	template<typename T>
	INLINE bool float_compare(const T& a, const T& b) NOEXCEPT
	{
		constexpr T EPSILON = 0.0'00'001f;
		return std::fabs(a - b) < EPSILON;
	}

}