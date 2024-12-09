#include "Chozo/Core/Thread.h"

#include <pthread.h>

namespace Chozo {

	Thread::Thread(std::string  name)
		: m_Name(std::move(name))
	{
	}

	Thread::~Thread()
	{
		if (m_Thread.joinable())
			m_Thread.detach();
	}

	void Thread::SetName(const std::string& name)
	{
		m_Name = name;

		// Truncate the name if it exceeds the system limit (typically 16 characters on macOS)
		const std::string truncatedName = name.substr(0, 15);
		pthread_setname_np(truncatedName.c_str());
	}

	std::string Thread::GetName()
	{
		if (char name[16] = { 0 }; pthread_getname_np(m_Thread.native_handle(), name, sizeof(name)) == 0)
		{
			return {name};
		}
		return "Unknown";
	}

	void Thread::Join()
	{
		if (m_Thread.joinable())
			m_Thread.join();
	}

	// TODO: `ThreadSignal`
}
