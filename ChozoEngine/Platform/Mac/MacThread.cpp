#include "Chozo/Core/Thread.h"

#include <pthread.h>

namespace Chozo {

	Thread::Thread(const std::string& name)
		: m_Name(name)
	{
	}

	void Thread::SetName(const std::string& name)
	{
        // Truncate the name if it exceeds the system limit (typically 16 characters on macOS)
        std::string truncatedName = name.substr(0, 15);

        pthread_setname_np(truncatedName.c_str());
	}

	void Thread::Join()
	{
		m_Thread.join();
	}

	// TODO(Emily): `ThreadSignal`
}
