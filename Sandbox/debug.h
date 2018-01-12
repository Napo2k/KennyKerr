#pragma once

#include "stdafx.h"
#include <windows.h>
#include <crtdbg.h>

#define ASSERT _ASSERTE

#if _DEBUG
#define VERIFY ASSERT
#else
#define VERIFY(expression) (expression)
#endif

struct LastException
{
	DWORD result;
	LastException() : result{ GetLastError() } {}
};

#ifdef _DEBUG
inline auto Trace(wchar_t const * format, ...) -> void
{
	va_list args;
	va_start(args, format);

	wchar_t buffer[256];

	ASSERT(-1 != _vsnwprintf_s(buffer,
		_countof(buffer) - 1,
		format,
		args));

	va_end(args);

	OutputDebugString(buffer);
}

struct Tracer
{
	char const * m_filename;
	unsigned m_line;

	Tracer(char const * filename, unsigned const line) :
		m_filename{ filename },
		m_line{ line }
	{

	}

	template<typename... Args>
	auto operator()(wchar_t const * format, Args... args) const -> void
	{
		wchar_t buffer[256];
		auto count = swprintf_s(buffer,
			L"%S(%d): ",
			m_filename,
			m_line);

		ASSERT(-1 != count);

		ASSERT(-1 != _snwprintf_s(buffer + count,
			_countof(buffer) - count,
			_countof(buffer) - count - 1,
			format,
			args...));

		OutputDebugString(buffer);
	}
};
#endif

#ifdef _DEBUG
#define TRACE Tracer(__FILE__, __LINE__)
#else
#define TRACE __noop
#endif