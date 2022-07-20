#include <wedge3/generic_callback.h>
#include <wedge3/globals.h>

#include "battles.h"
#include "enemies.h"
#include "general.h"
#include "inventory.h"

Battle_TTH::Battle_TTH(std::vector<Battle_Enemy *> enemies, std::string bg) :
	Battle_Game(bg, 0),
	enemies(enemies)
{
}

Battle_TTH::~Battle_TTH()
{
}

bool Battle_TTH::start()
{
	if (Battle_Game::start() == false) {
		return false;
	}

	for (size_t i = 0; i < enemies.size(); i++) {
		enemies[i]->start();
	}

	std::vector< util::Size<int> > sizes;
	for (size_t i = 0; i < enemies.size(); i++) {
		sizes.push_back(enemies[i]->get_sprite()->get_current_image()->size);
	}

	std::vector< util::Point<int> > positions;
	std::vector< util::Point<int> > positions_1;
	positions_1.push_back(util::Point<int>(shim::screen_size.w/2-sizes[0].w/2, shim::screen_size.h/3-sizes[0].h/2));
	std::vector< util::Point<int> > positions_2;
	if (sizes.size() > 1) {
		positions_2.push_back(util::Point<int>(shim::screen_size.w/2-sizes[0].w/2-25+(20-sizes[0].w/2), shim::screen_size.h/3-sizes[0].h/2));
		positions_2.push_back(util::Point<int>(shim::screen_size.w/2-sizes[0].w/2+25-(20-sizes[1].w/2), shim::screen_size.h/3-sizes[0].h/2));
	}

	positions = enemies.size() == 1 ? positions_1 : positions_2;

	for (size_t i = 0; i < enemies.size(); i++) {
		enemies[i]->set_position(positions[i]);
		entities.push_back(enemies[i]);
		gold += enemies[i]->get_gold();
	}

	return true;
}

wedge::Object Battle_TTH::get_found_object()
{
	wedge::Object o;
	o.type = wedge::OBJECT_NONE;
	return o;
}
