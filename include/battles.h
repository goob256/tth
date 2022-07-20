#ifndef BATTLES_H
#define BATTLES_H

#include <wedge3/inventory.h>

#include "battle_game.h"

class Battle_Enemy;

class Battle_TTH : public Battle_Game
{
public:
	Battle_TTH(std::vector<Battle_Enemy *> enemies, std::string bg);
	virtual ~Battle_TTH();

	bool start();

private:
	wedge::Object get_found_object();

	std::vector<Battle_Enemy *> enemies;
};
#endif // BATTLES_H
