#ifndef INVENTORY_H
#define INVENTORY_H

#include <wedge3/inventory.h>

// 0 is none, defined in Wedge

enum Item_Types
{
	ITEM_DOUGHNUT = 1,
	ITEM_DIE,
	ITEM_SIZE
};

enum Special_Types
{
	SPECIAL_SIZE
};

enum Armour_Types
{
	ARMOUR_SIZE = 1
};

enum Weapon_Types
{
	WEAPON_SAMURAI_SWORD = 1,
	WEAPON_SIZE
};

class Object_Interface : public wedge::Object_Interface
{
public:
	virtual ~Object_Interface();

	wedge::Object make_object(wedge::Object_Type type, int id, int quantity);
	int use(wedge::Object object, wedge::Base_Stats *target);
	wedge::Fixed_Stats get_armour_stats(int id);
};

#endif // INVENTORY_H
