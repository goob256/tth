#include <wedge3/battle_enemy.h>
#include <wedge3/delay.h>
#include <wedge3/general.h>
#include <wedge3/generic_callback.h>
#include <wedge3/generic_immediate_callback.h>
#include <wedge3/play_animation.h>
#include <wedge3/play_sound.h>
#include <wedge3/rumble.h>
#include <wedge3/special_number.h>
#include <wedge3/stats.h>

#include "battle_game.h"
#include "battle_player.h"
#include "battles.h"
#include "coin.h"
#include "enemies.h"
#include "general.h"
#include "globals.h"
#include "inventory.h"
#include "stats.h"

static void attack_callback(void *data)
{
	if (BATTLE == nullptr) {
		return;
	}

	Battle_Enemy *e = static_cast<Battle_Enemy *>(data);
	e->get_sprite()->set_animation("idle");
	e->set_attack_done(true);
}

static void hit_callback(void *data)
{
	auto e = static_cast<Battle_Enemy *>(data);
	gfx::Sprite *sprite = e->get_sprite();
	if (sprite->get_animation() == "hit") {
		sprite->set_animation(sprite->get_previous_animation());
	}
}

Battle_Enemy::Battle_Enemy(std::string name, int min_delay, int max_delay) :
	wedge::Battle_Enemy(name),
	turn_started(false),
	attack_done(false),
	min_delay(min_delay),
	max_delay(max_delay)
{
	next_move = GET_TICKS() + util::rand(min_delay, max_delay);

	attack_anims.push_back("attack");
	//attack_sounds.push_back(TTH_GLOBALS->enemy_attack);
	attack_mults.push_back(1.0f);

	float m = enemy_speed_mul();
	min_delay *= m;
	max_delay *= m;
	
	draw_shadow = true;
	shadow_size = {24, 12};
	auto_shadow_pos = true;
}

Battle_Enemy::~Battle_Enemy()
{
	if (stats->hp <= 0) {
		TTH_INSTANCE->enemies_killed++;
		if (TTH_INSTANCE->enemies_killed >= 100) {
			//util::achieve((void *)ACH_WAYDOWN);
		}
	}
}

void Battle_Enemy::set_attack_done(bool done)
{
	attack_done = done;
}

int Battle_Enemy::start_attack(wedge::Battle_Game::Turn *turn)
{
	attack_done = false;
	attack_num = util::rand(0, int(attack_anims.size()-1));
	if (attack_num < (int)attack_sounds.size()) {
		if (attack_sounds[attack_num] != nullptr) {
			attack_sounds[attack_num]->play(false);
		}
	}
	else if (attack_sounds.size() > 0) {
		attack_sounds[0]->play(false);
	}
	sprite->set_animation(attack_anims[attack_num], attack_callback, this);
	if (attack_num < (int)attack_names.size() && attack_names[attack_num] != "") {
		auto b = dynamic_cast<Battle_Game *>(BATTLE);
		b->add_notification(Battle_Game::LEFT, attack_names[attack_num]);
	}

	attack_start = GET_TICKS();

	return attack_num;
}

bool Battle_Enemy::turn_attack(wedge::Battle_Game::Turn *turn)
{
	if (GET_TICKS()-attack_start < 500) {
		return true;
	}
	if (attack_done) {
		auto p = turn->targets[0];
		float mult;
		if ((int)attack_mults.size() > attack_num) {
			mult = attack_mults[attack_num];
		}
		else if (attack_mults.size() > 0) {
			mult = attack_mults[0];
		}
		else {
			mult = 1.0f;
		}
		int damage = MAX(1, stats->fixed.attack*mult-p->get_stats()->fixed.defence);
		damage += util::rand(0, stats->fixed.attack*0.25f);
		add_special_number(this, p, damage, true, true);
		turn_started = false;
		next_move = GET_TICKS() + util::rand(min_delay, max_delay);
		return false;
	}
	return true;
}

void Battle_Enemy::start_dialogue(wedge::Battle_Game::Turn *turn)
{
	static_cast<Battle_Game *>(BATTLE)->close_guis();
	Battle_Game *b = static_cast<Battle_Game *>(BATTLE);
	b->set_window_shown(false);
	GLOBALS->do_dialogue(turn->dialogue.tag, turn->dialogue.text, turn->dialogue.type, turn->dialogue.position, turn->dialogue.monitor);
}

bool Battle_Enemy::turn_dialogue(wedge::Battle_Game::Turn *turn)
{
	if (GLOBALS->dialogue_active(BATTLE) == false) {
		Battle_Game *b = static_cast<Battle_Game *>(BATTLE);
		b->set_window_shown(true);
		turn_started = false;
		return false;
	}
	return true;
}

bool Battle_Enemy::take_hit(wedge::Battle_Entity *actor, int damage)
{
	if (damage > 0) {
		sprite->set_animation("hit", hit_callback, this);
	}
	return wedge::Battle_Enemy::take_hit(actor, damage);
}

util::Point<int> Battle_Enemy::get_turn_order_display_offset()
{
	return util::Point<int>(0, 0);
}

//--

Enemy_Beholder::Enemy_Beholder() :
	Battle_Enemy(GLOBALS->game_t->translate(591)/* Originally: Beholder */, 3000, 6000)
{
	floating = true;
}

Enemy_Beholder::~Enemy_Beholder()
{
}

bool Enemy_Beholder::start()
{
	gold = 0;
	sprite = new gfx::Sprite("beholder");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 90;
	stats->fixed.attack = 5 * enemy_attack_mul();
	stats->fixed.defence = 5 * enemy_defence_mul();

	return true;
}

wedge::Battle_Game::Turn *Enemy_Beholder::get_turn()
{
	/*
	if (GET_TICKS() < next_move) {
		return nullptr;
	}
	*/

	auto t = new wedge::Battle_Game::Turn;

	t->actor = this;
	t->turn_type = wedge::Battle_Game::ATTACK;
	t->turn_name = "attack";
	t->targets.push_back(BATTLE->get_random_player());
	t->started = false;

	return t;
}

bool Enemy_Beholder::take_turn(wedge::Battle_Game::Turn *turn)
{
	if (turn_started == false) {
		if (turn->targets.size() == 0) {
			next_move = GET_TICKS() + util::rand(min_delay, max_delay);
			return false; // do nothing, we were attacking dead player or something and there are no more living players
		}
		turn_started = true;
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			start_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			start_dialogue(turn);
		}
	}
	else {
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			return turn_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			if (turn_dialogue(turn) == false) {
				return false;
			}
		}
	}
	return true;
}

//--

Enemy_Werewolf::Enemy_Werewolf() :
	Battle_Enemy(GLOBALS->game_t->translate(594)/* Originally: Werewolf */, 3000, 6000)
{
}

Enemy_Werewolf::~Enemy_Werewolf()
{
}

bool Enemy_Werewolf::start()
{
	gold = 0;
	sprite = new gfx::Sprite("werewolf");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 120;
	stats->fixed.attack = 7 * enemy_attack_mul();
	stats->fixed.defence = 0 * enemy_defence_mul();

	return true;
}

wedge::Battle_Game::Turn *Enemy_Werewolf::get_turn()
{
	/*
	if (GET_TICKS() < next_move) {
		return nullptr;
	}
	*/

	auto t = new wedge::Battle_Game::Turn;

	t->actor = this;
	t->turn_type = wedge::Battle_Game::ATTACK;
	t->turn_name = "attack";
	t->targets.push_back(BATTLE->get_random_player());
	t->started = false;

	return t;
}

bool Enemy_Werewolf::take_turn(wedge::Battle_Game::Turn *turn)
{
	if (turn_started == false) {
		if (turn->targets.size() == 0) {
			next_move = GET_TICKS() + util::rand(min_delay, max_delay);
			return false; // do nothing, we were attacking dead player or something and there are no more living players
		}
		turn_started = true;
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			start_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			start_dialogue(turn);
		}
	}
	else {
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			return turn_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			if (turn_dialogue(turn) == false) {
				return false;
			}
		}
	}
	return true;
}

//--

Enemy_Ooze::Enemy_Ooze() :
	Battle_Enemy(GLOBALS->game_t->translate(600)/* Originally: Ooze */, 3000, 6000)
{
}

Enemy_Ooze::~Enemy_Ooze()
{
}

bool Enemy_Ooze::start()
{
	gold = 0;
	sprite = new gfx::Sprite("ooze");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 100;
	stats->fixed.attack = 6 * enemy_attack_mul();
	stats->fixed.defence = 0 * enemy_defence_mul();

	return true;
}

wedge::Battle_Game::Turn *Enemy_Ooze::get_turn()
{
	/*
	if (GET_TICKS() < next_move) {
		return nullptr;
	}
	*/

	auto t = new wedge::Battle_Game::Turn;

	t->actor = this;
	t->turn_type = wedge::Battle_Game::ATTACK;
	t->turn_name = "attack";
	t->targets.push_back(BATTLE->get_random_player());
	t->started = false;

	return t;
}

bool Enemy_Ooze::take_turn(wedge::Battle_Game::Turn *turn)
{
	if (turn_started == false) {
		if (turn->targets.size() == 0) {
			next_move = GET_TICKS() + util::rand(min_delay, max_delay);
			return false; // do nothing, we were attacking dead player or something and there are no more living players
		}
		turn_started = true;
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			start_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			start_dialogue(turn);
		}
	}
	else {
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			return turn_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			if (turn_dialogue(turn) == false) {
				return false;
			}
		}
	}
	return true;
}

//--

Enemy_Mermaid::Enemy_Mermaid() :
	Battle_Enemy(GLOBALS->game_t->translate(601)/* Originally: Mermaid */, 3000, 6000)
{
}

Enemy_Mermaid::~Enemy_Mermaid()
{
}

bool Enemy_Mermaid::start()
{
	gold = 0;
	sprite = new gfx::Sprite("mermaid");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 110;
	stats->fixed.attack = 8 * enemy_attack_mul();
	stats->fixed.defence = 0 * enemy_defence_mul();

	return true;
}

wedge::Battle_Game::Turn *Enemy_Mermaid::get_turn()
{
	/*
	if (GET_TICKS() < next_move) {
		return nullptr;
	}
	*/

	auto t = new wedge::Battle_Game::Turn;

	t->actor = this;
	t->turn_type = wedge::Battle_Game::ATTACK;
	t->turn_name = "attack";
	t->targets.push_back(BATTLE->get_random_player());
	t->started = false;

	return t;
}

bool Enemy_Mermaid::take_turn(wedge::Battle_Game::Turn *turn)
{
	if (turn_started == false) {
		if (turn->targets.size() == 0) {
			next_move = GET_TICKS() + util::rand(min_delay, max_delay);
			return false; // do nothing, we were attacking dead player or something and there are no more living players
		}
		turn_started = true;
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			start_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			start_dialogue(turn);
		}
	}
	else {
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			return turn_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			if (turn_dialogue(turn) == false) {
				return false;
			}
		}
	}
	return true;
}

//--

Enemy_Skeleton::Enemy_Skeleton() :
	Battle_Enemy(GLOBALS->game_t->translate(602)/* Originally: Skeleton */, 3000, 6000)
{
}

Enemy_Skeleton::~Enemy_Skeleton()
{
}

bool Enemy_Skeleton::start()
{
	gold = 0;
	sprite = new gfx::Sprite("skeleton");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 80;
	stats->fixed.attack = 6 * enemy_attack_mul();
	stats->fixed.defence = 5 * enemy_defence_mul();

	return true;
}

wedge::Battle_Game::Turn *Enemy_Skeleton::get_turn()
{
	/*
	if (GET_TICKS() < next_move) {
		return nullptr;
	}
	*/

	auto t = new wedge::Battle_Game::Turn;

	t->actor = this;
	t->turn_type = wedge::Battle_Game::ATTACK;
	t->turn_name = "attack";
	t->targets.push_back(BATTLE->get_random_player());
	t->started = false;

	return t;
}

bool Enemy_Skeleton::take_turn(wedge::Battle_Game::Turn *turn)
{
	if (turn_started == false) {
		if (turn->targets.size() == 0) {
			next_move = GET_TICKS() + util::rand(min_delay, max_delay);
			return false; // do nothing, we were attacking dead player or something and there are no more living players
		}
		turn_started = true;
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			start_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			start_dialogue(turn);
		}
	}
	else {
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			return turn_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			if (turn_dialogue(turn) == false) {
				return false;
			}
		}
	}
	return true;
}

//--

Enemy_Phoenix::Enemy_Phoenix() :
	Battle_Enemy(GLOBALS->game_t->translate(603)/* Originally: Phoenix */, 3000, 6000)
{
	floating = true;
}

Enemy_Phoenix::~Enemy_Phoenix()
{
}

bool Enemy_Phoenix::start()
{
	gold = 0;
	sprite = new gfx::Sprite("phoenix");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 120;
	stats->fixed.attack = 8 * enemy_attack_mul();
	stats->fixed.defence = 5 * enemy_defence_mul();

	return true;
}

wedge::Battle_Game::Turn *Enemy_Phoenix::get_turn()
{
	/*
	if (GET_TICKS() < next_move) {
		return nullptr;
	}
	*/

	auto t = new wedge::Battle_Game::Turn;

	t->actor = this;
	t->turn_type = wedge::Battle_Game::ATTACK;
	t->turn_name = "attack";
	t->targets.push_back(BATTLE->get_random_player());
	t->started = false;

	return t;
}

bool Enemy_Phoenix::take_turn(wedge::Battle_Game::Turn *turn)
{
	if (turn_started == false) {
		if (turn->targets.size() == 0) {
			next_move = GET_TICKS() + util::rand(min_delay, max_delay);
			return false; // do nothing, we were attacking dead player or something and there are no more living players
		}
		turn_started = true;
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			start_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			start_dialogue(turn);
		}
	}
	else {
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			return turn_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			if (turn_dialogue(turn) == false) {
				return false;
			}
		}
	}
	return true;
}

//--

Enemy_Alraune::Enemy_Alraune() :
	Battle_Enemy(GLOBALS->game_t->translate(604)/* Originally: Alraune */, 3000, 6000)
{
}

Enemy_Alraune::~Enemy_Alraune()
{
}

bool Enemy_Alraune::start()
{
	gold = 0;
	sprite = new gfx::Sprite("alraune");

	stats = new wedge::Base_Stats();

	stats->fixed.max_hp = stats->hp = 110;
	stats->fixed.attack = 7 * enemy_attack_mul();
	stats->fixed.defence = 10 * enemy_defence_mul();

	return true;
}

wedge::Battle_Game::Turn *Enemy_Alraune::get_turn()
{
	/*
	if (GET_TICKS() < next_move) {
		return nullptr;
	}
	*/

	auto t = new wedge::Battle_Game::Turn;

	t->actor = this;
	t->turn_type = wedge::Battle_Game::ATTACK;
	t->turn_name = "attack";
	t->targets.push_back(BATTLE->get_random_player());
	t->started = false;

	return t;
}

bool Enemy_Alraune::take_turn(wedge::Battle_Game::Turn *turn)
{
	if (turn_started == false) {
		if (turn->targets.size() == 0) {
			next_move = GET_TICKS() + util::rand(min_delay, max_delay);
			return false; // do nothing, we were attacking dead player or something and there are no more living players
		}
		turn_started = true;
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			start_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			start_dialogue(turn);
		}
	}
	else {
		if (turn->turn_type == wedge::Battle_Game::ATTACK) {
			return turn_attack(turn);
		}
		else if (turn->turn_type == wedge::Battle_Game::DIALOGUE) {
			if (turn_dialogue(turn) == false) {
				return false;
			}
		}
	}
	return true;
}
