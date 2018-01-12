#pragma once

#include "debug.h"

struct ManualResetEvent
{
	HANDLE m_handle;

	ManualResetEvent()
	{
		m_handle = CreateEvent(nullptr,
			true,
			false,
			nullptr);

		if (!m_handle)
		{
			throw LastException();
		}
	}

	~ManualResetEvent()
	{
		VERIFY(CloseHandle(m_handle));
	}
};
