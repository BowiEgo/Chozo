#include "Launch.h"

int main(int argc, char **argv)
{
	std::string Str = "Hello World!";
	std::cout << "Hello, Chozo Engine!" << std::endl;
	std::cout << Str << std::endl;

	Chozo::EngineLoop engineLoop;

	engineLoop.Init();
	engineLoop.Tick();
    engineLoop.Exit();
}