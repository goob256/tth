#ifndef WEDGE3_NPC_H
#define WEDGE3_NPC_H

#include "wedge3/main.h"
#include "wedge3/globals.h"
#include "wedge3/map_entity.h"

namespace wedge {

class WEDGE3_EXPORT NPC : public Map_Entity
{
public:
	NPC(std::string name, std::string tag, std::string sprite_name, std::string dialogue_name);
	NPC(util::JSON::Node *json);
	~NPC();

	bool activate(Map_Entity *activator);

	std::string save();

	Direction get_previous_direction();
	bool get_was_moving();

	bool changed_anim();

protected:
	struct Text {
		std::vector<int> milestones;
		int id;
	};

	void load_dialogue();

	std::string tag;
	std::string dialogue_name;
	std::vector<Text> dialogue;
	Direction previous_direction;
	bool was_moving;
	bool anim_changed;
};

}

#endif // WEDGE3_NPC_H
