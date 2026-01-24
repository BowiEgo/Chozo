#include "Launch.h"

DEFINE_LOG_CATEGORY_STATIC(LogLaunch, Info);

int main(int argc, char **argv)
{
	std::string msg = "Starting up Chozo Engine..";
	CZ_LOG(LogLaunch, Info, msg);

	Chozo::FEngineLoop engineLoop;

	engineLoop.Init();
	
	while (true) 
    {
        engineLoop.Tick();

        if (engineLoop.ShouldClose()) break;
    }

    engineLoop.Exit();

	return 0;
}