#ifndef WEDGE3_CHEST_H
#define WEDGE3_CHEST_H

#include "wedge3/main.h"
#include "wedge3/globals.h"
#include "wedge3/inventory.h"
#include "wedge3/map_entity.h"

namespace wedge {

class WEDGE3_EXPORT Chest : public Map_Entity
{
public:
	Chest(std::string entity_name, std::string sprite_name, Object contents, int milestone = -1, Dialogue_Position dialogue_position = DIALOGUE_AUTO);
	Chest(std::string entity_name, std::string sprite_name, int gold, int milestone = -1, Dialogue_Position dialogue_position = DIALOGUE_AUTO);
	Chest(util::JSON::Node *json);
	virtual ~Chest();

	bool activate(Map_Entity *activator);

	std::string save();

	void set_remove_when_activated(bool remove_when_activated);

	void set_achievement(void *id);

protected:
	bool open;
	Object contents;
	int gold;
	int milestone;
	Dialogue_Position dialogue_position;
	bool remove_when_activated;
	void *achievement;
	bool delete_achievement;
};

}

#endif // WEDGE3_CHEST_H
