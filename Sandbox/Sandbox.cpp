// Sandbox.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <crtdbg.h>
#include <cstdio>
#include <memory>

#include "debug.h"
#include "manualresetevent.h"
#include "hen.h"

namespace KennyKerr
{
	template <typename Traits>
	class unique_handle
	{
		typedef typename Traits::pointer pointer;

		pointer m_value;

		auto close() noexcept -> void 
		{
			if(*this)
			{
				Traits::close(m_value);
			}
		}

		unique_handle(unique_handle const &);
		auto operator=(unique_handle const &)->unique_handle &;

	public:

		explicit unique_handle(pointer value = Traits::invalid()) throw(): 
		m_value { value }
		{
		}

		unique_handle(unique_handle && other) throw() :
		m_value { other.release() }
		{

			TRACE(L"move ctor\n");
		}

		auto operator=(unique_handle && other) throw() -> unique_handle &
		{
			TRACE(L"move assignment\n");
			if (this != &other)
			{
				reset(other.release());
			}

			return *this;
		}

		~unique_handle() noexcept
		{
			TRACE(L"destructor\n");
			close();
		}

		explicit operator bool() const noexcept
		{
			return m_value != Traits::invalid();
		}

		auto get() const throw() -> pointer
		{ 
			return m_value;
		}
		
		auto release() throw() -> pointer
		{
			auto value = m_value;
			m_value = Traits::invalid();
			return value;
		}

		// we can replace a managed handle with another
		// if this unique handle happens to own a handle, reset must ensure that the previously owned handle gets closed 
		auto reset(pointer value = Traits::invalid()) throw() -> bool
		{
			if (m_value != value)
			{
				close();
				m_value = value;
			}
			return static_cast<bool>(m_value);
		}

		auto swap(unique_handle<Traits> &other) throw() -> void
		{
			std::swap(m_value, other.m_value);
		}
	};

	template <typename Traits>
	auto swap(unique_handle<Traits> & left,
		unique_handle<Traits> & right) throw() -> void
	{
		TRACE(L"nonmember swap\n");
		left.swap(right);
	}

	template<typename Traits>
	auto operator==(unique_handle<Traits> const & left,
					unique_handle<Traits> const & right) throw () -> bool 
	{
		return left.get == right.get();
	}

	template<typename Traits>
	auto operator!=(unique_handle<Traits> const & left,
		unique_handle<Traits> const & right) throw () -> bool
	{
		return left.get != right.get();
	}

	template<typename Traits>
	auto operator>(unique_handle<Traits> const & left,
				   unique_handle<Traits> const & right) throw () -> bool
	{
		return left.get > right.get();
	}

	template<typename Traits>
	auto operator<(unique_handle<Traits> const & left,
				   unique_handle<Traits> const & right) throw () -> bool
	{
		return left.get < right.get();
	}

	template<typename Traits>
	auto operator>=(unique_handle<Traits> const & left,
					unique_handle<Traits> const & right) throw () -> bool
	{
		return left.get >= right.get();
	}

	template<typename Traits>
	auto operator<=(unique_handle<Traits> const & left,
					unique_handle<Traits> const & right) throw () -> bool
	{
		return left.get <= right.get();
	}

	struct null_handle_traits
	{
		typedef HANDLE pointer;
	
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			VERIFY(CloseHandle(value));
		}
	};

	struct invalid_handle_traits
	{
		typedef HANDLE pointer;

		static auto invalid() throw() -> pointer
		{
			return INVALID_HANDLE_VALUE;
		}

		static auto close(pointer value) throw() -> void
		{
			VERIFY(CloseHandle(value));
		}
	};

	typedef unique_handle<null_handle_traits> null_handle;
	typedef unique_handle<invalid_handle_traits> invalid_handle;
}

#include <utility>
#include <algorithm>

using namespace std;
using namespace KennyKerr;

/*
struct work_deleter
{
	typedef PTP_WORK pointer;
	auto operator()(pointer value) const throw()->void
	{
		CloseThreadpoolWork(value);
	}
};

struct map_view_deleter
{
	typedef char const * pointer;

	auto operator()(pointer value) const throw() -> void
	{
		VERIFY(UnmapViewOfFile(value));
	}
}; */

auto main() -> int
{
	/*
	typedef unique_ptr<PTP_WORK, work_deleter> work;

	auto w = work
	{
		CreateThreadpoolWork([] (PTP_CALLBACK_INSTANCE, void *, PTP_WORK)
		{
			TRACE(L"oHAI!\n");
		},
		nullptr, nullptr)
	};

	if (w)
	{
		SubmitThreadpoolWork(w.get());
		WaitForThreadpoolWorkCallbacks(w.get(), false);
	}

	// shared pointers

	auto sp = shared_ptr<int>{};

	ASSERT(!sp);
	ASSERT(sp.use_count() == 0);
	ASSERT(!sp.unique());

	sp.reset(new int{ 123 });
	sp = make_shared<int>(123);

	ASSERT(sp);
	ASSERT(sp.use_count() == 1);
	ASSERT(sp.unique());

	auto sp2 = sp;
	ASSERT(sp.use_count() == 2);
	ASSERT(!sp.unique());

	ASSERT(sp2.use_count() == 2);
	ASSERT(!sp2.unique());

	int copy = *sp;
	int &ref = *sp;
	int * ptr = sp.get();

	ASSERT(sp.get() == sp2.get());
	ASSERT(sp == sp2);

	// weak pointer

	auto sp3 = make_shared<int>(123);
	// weak pointers are like understudies, filling in for someone more important (in this case a shared pointer)
	auto wp = weak_ptr<int>{ sp3 };

	ASSERT(!wp.expired());
	ASSERT(wp.use_count() == 1);

	if (auto locked = wp.lock())
	{
		TRACE(L"locked! %d\n", *locked);
	}

	sp3 = nullptr;

	ASSERT(wp.expired());
	ASSERT(wp.use_count() == 0);

	if (auto locked = wp.lock())
	{
		TRACE(L"locked! %d\n", *locked);
	} 
	else
	{
		wp.reset();
	}

	auto event = null_handle
	{
		CreateEvent(nullptr,
					true,
					false,
					nullptr) 
	};

	if(event)
	{
		VERIFY(SetEvent(event.get()));
	}

	if (event.reset(CreateEvent(nullptr, false,false,nullptr)))
	{
		TRACE(L"RESET!\n");
	}

	ASSERT(event);

	auto other = null_handle{ move(event) };

	ASSERT(!event);
	ASSERT(other);

	event = move(other);

	ASSERT(event);
	ASSERT(!other);

	event.release();

	auto filename = LR"(C:\Users\ignacio.fernandez\documents\visual studio 2015\projects\sandbox\sandbox\sandbox.cpp)";

	auto file = invalid_handle
	{
		CreateFile(filename,
					GENERIC_READ,
					FILE_SHARE_READ,
					nullptr,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					nullptr)
	};

	if(!file)
	{
		printf("CreateFile failed %d\n", GetLastError());
		return 0;
	}

	auto size = LARGE_INTEGER{};

	if (!GetFileSizeEx(file.get(), &size))
	{
		printf("GetFileSizeEx failed %d\n", GetLastError());
		return 0;
	}

	if (size.QuadPart == 0)
	{
		printf("File is empty\n");
		return 0;
	}

	auto map = null_handle
	{
		CreateFileMapping(file.get(),
							nullptr,
							PAGE_READONLY,
							0,0,
							nullptr)
	};

	if(!map)
	{
		printf("CreateFileMapping failed %d\n", GetLastError());
		return 0;
	}

	file.reset();

	auto view = unique_ptr<char, map_view_deleter>
	{
		static_cast<char*>(MapViewOfFile(map.get(),
										FILE_MAP_READ,
										0,0,0))
	};

	if (!view)
	{
		printf("MapViewOfFile failed %d\n", GetLastError());
		return 0;
	}

	printf("%.*s\n", static_cast<unsigned>(size.QuadPart), view.get());*/

	
}
