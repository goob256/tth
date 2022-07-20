#ifndef ENEMIES_H
#define ENEMIES_H

#include <wedge3/battle_enemy.h>
#include <wedge3/battle_player.h>

class Battle_Enemy : public wedge::Battle_Enemy
{
public:
	Battle_Enemy(std::string name, int min_delay, int max_delay);
	virtual ~Battle_Enemy();

	void set_attack_done(bool done);

	bool take_hit(wedge::Battle_Entity *actor, int damage);

	virtual util::Point<int> get_turn_order_display_offset();

protected:
	int start_attack(wedge::Battle_Game::Turn *turn);
	bool turn_attack(wedge::Battle_Game::Turn *turn);
	
	void start_dialogue(wedge::Battle_Game::Turn *turn);
	bool turn_dialogue(wedge::Battle_Game::Turn *turn);

	bool turn_started;
	bool attack_done;
	Uint32 next_move;
	int min_delay;
	int max_delay;
	std::string attack_name;
	Uint32 attack_start;

	std::vector<std::string> attack_anims;
	std::vector<std::string> attack_names;
	std::vector<audio::Sound *> attack_sounds;
	std::vector<float> attack_mults;
	int attack_num;
};

class Enemy_Beholder : public Battle_Enemy
{
public:
	Enemy_Beholder();
	virtual ~Enemy_Beholder();

	bool start();
	
	wedge::Battle_Game::Turn *get_turn();
	bool take_turn(wedge::Battle_Game::Turn *turn);
};

class Enemy_Werewolf : public Battle_Enemy
{
public:
	Enemy_Werewolf();
	virtual ~Enemy_Werewolf();

	bool start();
	
	wedge::Battle_Game::Turn *get_turn();
	bool take_turn(wedge::Battle_Game::Turn *turn);
};

class Enemy_Ooze : public Battle_Enemy
{
public:
	Enemy_Ooze();
	virtual ~Enemy_Ooze();

	bool start();
	
	wedge::Battle_Game::Turn *get_turn();
	bool take_turn(wedge::Battle_Game::Turn *turn);
};

class Enemy_Mermaid : public Battle_Enemy
{
public:
	Enemy_Mermaid();
	virtual ~Enemy_Mermaid();

	bool start();
	
	wedge::Battle_Game::Turn *get_turn();
	bool take_turn(wedge::Battle_Game::Turn *turn);
};

class Enemy_Skeleton : public Battle_Enemy
{
public:
	Enemy_Skeleton();
	virtual ~Enemy_Skeleton();

	bool start();
	
	wedge::Battle_Game::Turn *get_turn();
	bool take_turn(wedge::Battle_Game::Turn *turn);
};

class Enemy_Phoenix : public Battle_Enemy
{
public:
	Enemy_Phoenix();
	virtual ~Enemy_Phoenix();

	bool start();
	
	wedge::Battle_Game::Turn *get_turn();
	bool take_turn(wedge::Battle_Game::Turn *turn);
};

class Enemy_Alraune : public Battle_Enemy
{
public:
	Enemy_Alraune();
	virtual ~Enemy_Alraune();

	bool start();
	
	wedge::Battle_Game::Turn *get_turn();
	bool take_turn(wedge::Battle_Game::Turn *turn);
};

#endif // ENEMIES_H
