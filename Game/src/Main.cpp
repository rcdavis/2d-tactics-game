#include "Utils/Log.h"
#include "Game.h"

int main() {
	Log::Init("2DT");

	Game game;
	game.Run();

	return 0;
}
