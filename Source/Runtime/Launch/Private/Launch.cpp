#include "Launch.h"

DEFINE_LOG_CATEGORY_STATIC(LogLaunch, Info);

int main(int argc, char **argv)
{
	std::string msg = "Starting up Chozo Engine..";
	CZ_LOG(LogLaunch, Info, msg);

	Chozo::FEngineLoop engineLoop;

	engineLoop.Init();
	engineLoop.Tick();
    engineLoop.Exit();
}