#include <wedge3/globals.h>
#include <wedge3/inventory.h>
#include <wedge3/stats.h>

#include "inventory.h"
#include "stats.h"
#include "tth.h"

Object_Interface::~Object_Interface()
{
}

wedge::Object Object_Interface::make_object(wedge::Object_Type type, int id, int quantity)
{
	wedge::Object o;

	o.type = type;
	o.id = id;
	o.quantity = quantity;
	o.name = "";
	o.sell_price = 0;
	o.description = "Unknown...";

	if (type == wedge::OBJECT_ITEM) {
		if (id == ITEM_DOUGHNUT) {
			o.name = GLOBALS->game_t->translate(579)/* Originally: Doughnut */;
		}
		else if (id == ITEM_DIE) {
			o.name = GLOBALS->game_t->translate(606)/* Originally: Die */;
		}
	}
	else if (type == wedge::OBJECT_ARMOUR) {
	}
	else if (type == wedge::OBJECT_SPECIAL) {
	}
	else if (type == wedge::OBJECT_WEAPON) {
		if (id == WEAPON_SAMURAI_SWORD) {
			o.name = GLOBALS->game_t->translate(610)/* Originally: Samurai Sword */;
		}
	}

	return o;
}

int Object_Interface::use(wedge::Object object, wedge::Base_Stats *target)
{
	int amount = 0;

	if (object.type == wedge::OBJECT_ITEM) {
		if (object.id == ITEM_DOUGHNUT) {
			if (target->hp > 0) {
				target->hp = MIN(target->fixed.max_hp, target->hp + DOUGHNUT_HP);
				amount = DOUGHNUT_HP;
			}
		}
	}

	return amount;
}

wedge::Fixed_Stats Object_Interface::get_armour_stats(int id)
{
	wedge::Fixed_Stats stats;
	return stats;
}
