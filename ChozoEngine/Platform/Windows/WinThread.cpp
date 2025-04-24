#include "Chozo/Core/Thread.h"

namespace Chozo {

	Thread::Thread(std::string  name)
		: m_Name(std::move(name))
	{
	}

	Thread::~Thread()
	{

	}

	void Thread::SetName(const std::string& name)
	{
		m_Name = name;
	}

	std::string Thread::GetName()
	{
		return "Unknown";
	}

	void Thread::Join()
	{
	}

	// TODO: `ThreadSignal`
}