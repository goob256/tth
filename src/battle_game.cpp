#include <wedge3/area.h>
#include <wedge3/area_game.h>
#include <wedge3/battle_enemy.h>
#include <wedge3/general.h>
#include <wedge3/generic_callback.h>
#include <wedge3/globals.h>
#include <wedge3/input.h>
#include <wedge3/map_entity.h>
#include <wedge3/pause_task.h>

#include "battle_combo_drawer.h"
#include "battle_game.h"
#include "battle_player.h"
#include "battle_transition_in.h"
#include "battle_transition_out.h"
#include "enemies.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "hit.h"
#include "inventory.h"
#include "milestones.h"
#include "widgets.h"

static int get_y(wedge::Battle_Entity *e)
{
	wedge::Battle_Enemy *enemy;

	int y;

	if ((enemy = dynamic_cast<wedge::Battle_Enemy *>(e)) != nullptr) {
		y = enemy->get_position().y;
	}
	else {
		Battle_Player *player = dynamic_cast<Battle_Player *>(e);
		y = player->get_draw_pos().y;
	}
	
	util::Point<int> topleft, bottomright;
	e->get_sprite()->get_bounds(topleft, bottomright);
	y += topleft.y + (bottomright.y-topleft.y);
	
	return y;
}

static bool entity_y_compare(wedge::Battle_Entity *a, wedge::Battle_Entity *b)
{
	int y1 = get_y(a);
	int y2 = get_y(b);

	return y1 < y2;
}

static bool enemy_x_compare(wedge::Battle_Entity *a, wedge::Battle_Entity *b)
{
	wedge::Battle_Enemy *a_enemy = static_cast<wedge::Battle_Enemy *>(a);
	wedge::Battle_Enemy *b_enemy = static_cast<wedge::Battle_Enemy *>(b);
	util::Point<int> a_pos = a_enemy->get_position();
	util::Point<int> b_pos = b_enemy->get_position();
	gfx::Sprite *a_sprite = a->get_sprite();
	gfx::Sprite *b_sprite = b->get_sprite();
	gfx::Image *a_img;
	gfx::Image *b_img;
	gfx::Sprite::Animation *a_idle = a_sprite->get_animation("idle");
	gfx::Sprite::Animation *b_idle = b_sprite->get_animation("idle");
	if (a_idle == nullptr) {
		a_img = a_sprite->get_current_image();
	}
	else {
		a_img = a_idle->images[0];
	}
	if (b_idle == nullptr) {
		b_img = b_sprite->get_current_image();
	}
	else {
		b_img = b_idle->images[0];
	}
	util::Point<int> a_topleft, a_bottomright;
	util::Point<int> b_topleft, b_bottomright;
	a_img->get_bounds(a_topleft, a_bottomright);
	b_img->get_bounds(b_topleft, b_bottomright);
	a_pos.x += a_topleft.x + (a_bottomright.x - a_topleft.x);
	b_pos.x += b_topleft.x + (b_bottomright.x - b_topleft.x);
	return a_pos.x > b_pos.x;
}

static void turn_callback(void *data)
{
	auto d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Battle_Game *b = static_cast<Battle_Game *>(d->userdata);
	b->set_turn(d->choice);
}

static void item_callback(void *data)
{
	auto d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Battle_Game *b = static_cast<Battle_Game *>(d->userdata);
	b->set_item(d->choice);
}

static void easy_callback(void *data)
{
	auto d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Battle_Game *b = static_cast<Battle_Game *>(d->userdata);
	b->set_easy(d->choice);
}

static void turn_sprite_ended(void *data)
{
	gfx::Sprite *sprite = static_cast<gfx::Sprite *>(data);
	static_cast<Battle_Game *>(BATTLE)->turn_sprite_ended(sprite);
}

Battle_Game::Battle_Game(std::string bg, int bg_delay) :
	wedge::Battle_Game(bg, bg_delay),
	window_shown(true),
	player_stats_shown(true),
	enemy_stats_shown(true),
	active_player(0),
	getting_turn(false),
	turn_type(wedge::Battle_Game::NONE),
	turn_gui(nullptr),
	selecting(false),
	selecting_enemy(false),
	selecting_multi(false),
	selected(nullptr),
	acting_player(nullptr),
	next_turn(nullptr),
	detecting_combo(false),
	detector(nullptr),
	good_combo(-1),
	item_gui(nullptr),
	easy_gui(nullptr),
	mc_gui(nullptr),
	item_index(-1),
	next_turn_pos(0),
	next_turn_top(0),
	turn_gui_gone(true),
	just_exited_item_gui(false),
	just_exited_easy_gui(false),
	just_exited_multi_gui(false),
	has_darkness(false),
	osc_enabled(false)
{
	osc_enabled = GLOBALS->onscreen_controller_was_enabled;

	NEW_SYSTEM_AND_TASK(this)
	combo_drawer = new Battle_Combo_Drawer_Step(new_task);
	ADD_STEP(combo_drawer)
	ADD_TASK(new_task)
	FINISH_SYSTEM(this)

	try {
		wall = new gfx::Tilemap("battle_" + bg + ".wm2");
	}
	catch (util::Error &) {
		wall = nullptr;
	}
}

Battle_Game::~Battle_Game()
{
	if (getting_turn) {
		if (turn_gui_gone == false) {
			turn_gui->exit();
		}
	}
	if (item_gui) {
		item_gui->exit();
	}
	if (easy_gui) {
		easy_gui->exit();
	}
	if (mc_gui) {
		mc_gui->exit();
	}

	delete wall;
	
	for (auto it = started_turns.begin(); it != started_turns.end(); it++) {
		auto p = *it;
		delete p.second;
	}
			
	// Make sure any items that were used but canceled before being removed get reset
	INSTANCE->inventory.unuse_all();
}

bool Battle_Game::start()
{
	if (wedge::Battle_Game::start() == false) {
		return false;
	}

	sneak_attack = false;
	#if 0
	if (boss_battle) {
		sneak_attack = false;
	}
	else {
		sneak_attack = util::rand(0, 11) == 11;
		if (sneak_attack) {
			gfx::add_notification(GLOBALS->game_t->translate(374)/* Originally: Sneak attack! */);
		}
	}
	#endif

	// FIXME:
	//sneak_attack = true;

	if (sneak_attack) {
		for (auto e : entities) {
			if (dynamic_cast<Battle_Player *>(e)) {
				e->set_inverse_x(true);
			}
		}
	}

	return true;
}

void Battle_Game::start_transition_in()
{
	wedge::Battle_Game::start_transition_in();

	NEW_SYSTEM_AND_TASK(AREA)
	wedge::Map_Entity *player = AREA->get_player(0);
	wedge::Pause_Task_Step *pause1 = new wedge::Pause_Task_Step(player->get_input_step()->get_task(), true, new_task);
	Battle_Transition_In_Step *battle_step = new Battle_Transition_In_Step(this, new_task);
	wedge::Pause_Task_Step *pause2 = new wedge::Pause_Task_Step(player->get_input_step()->get_task(), false, new_task);
	ADD_STEP(pause1)
	ADD_STEP(battle_step)
	ADD_STEP(pause2)
	ADD_TASK(new_task)
	FINISH_SYSTEM(AREA)
}

void Battle_Game::start_transition_out()
{
	NEW_SYSTEM_AND_TASK(this)
	Battle_Transition_Out_Step *step = new Battle_Transition_Out_Step(new_task);
	ADD_STEP(step)
	ADD_TASK(new_task)
	FINISH_SYSTEM(this)
}

wedge::Battle_Player *Battle_Game::create_player(int index)
{
	return new Battle_Player(index);
}

void Battle_Game::draw()
{
	auto players = get_players();

	int frame;
	Uint32 div = (int)backgrounds.size() * bg_delay;
	if (div != 0) {
		Uint32 num = GET_TICKS() % div;
		frame = num / bg_delay;
	}
	else {
		frame = 0;
	}
	backgrounds[frame]->draw(util::Point<int>(shim::screen_size.w/2-backgrounds[0]->size.w/2, shim::screen_size.h-backgrounds[0]->size.h+3));

	std::string area_name = AREA->get_current_area()->get_name();
	if (atoi(area_name.c_str()) == 12) { // 13
		TTH_GLOBALS->level_13->draw({0, 0});
	}
	else if (atoi(area_name.c_str()) == 51) { // 52
		TTH_GLOBALS->joker->draw({float(shim::screen_size.w/2-TTH_GLOBALS->joker->size.w/2), float(shim::screen_size.h/2-TTH_GLOBALS->joker->size.h/2)});
	}
	else if (atoi(area_name.c_str()) == 776) { // 777
		TTH_GLOBALS->big_heart->draw({float(shim::screen_size.w/2-TTH_GLOBALS->big_heart->size.w/2), float(shim::screen_size.h/2-TTH_GLOBALS->big_heart->size.h/2)});
	}

	if (wall) {
		wall->draw(0, wall->get_num_layers()-1, util::Point<int>(-3, -18), false);
	}

	for (size_t i = 0; i < entities.size(); i++) {
		wedge::Battle_Entity *entity = entities[i];
		entity->draw_back();
	}

	for (size_t i = 0; i < entities.size(); i++) {
		wedge::Battle_Entity *entity = entities[i];
		entity->draw();
	}

	bool all_dead = true;
	auto v = get_enemies();
	for (auto &e : v) {
		if (e->get_stats()->hp > 0) {
			all_dead = false;
		}
	}
	if (all_dead == false) {
		all_dead = true;
		for (auto &e : players) {
			if (e->get_stats()->hp > 0) {
				all_dead = false;
			}
		}
	}

	if (GET_TICKS() < startup_time || all_dead) {
		Game::draw();

		if (all_dead) {
			do_all_dead(); // run it here too so turn_gui doesn't show up with no window (9patch below)
			for (size_t i = 0; i < entities.size(); i++) {
				wedge::Battle_Entity *entity = entities[i];
				entity->draw_fore();
			}
		}

		if (has_darkness) {
			real_draw_darkness(darkness_image1, darkness_image2, darkness_offset1, darkness_offset2, {0.0f, 0.0f}, 0.5f);
		}

		return;
	}
	
	bool running = false;
	for (auto p : players) {
		if (static_cast<Battle_Player *>(p)->is_running()) {
			running = true;
			break;
		}
	}

	if (running) {
		return;
	}

	if (selecting) {
		util::Point<float> pos;
		gfx::Image *cursor = TTH_GLOBALS->cursor->get_current_image();
		//gfx::Image *multi_image = TTH_GLOBALS->multi_check_checked->get_current_image();
		std::vector< std::pair<util::Point<float>, gfx::Sprite *> > v2;
		int flag;
		#if 0
		if (selecting_multi) {
			fixup_targets(players[active_player], multi_targets, selecting_enemy, false);
			for (auto e : multi_targets) {
				gfx::Sprite *anim;
				if (selecting_enemy) {
					auto enemy = static_cast<Battle_Enemy *>(e);
					if (enemy->get_draw_as_player()) {
						pos = enemy->get_sprite_pos();
					}
					else {
						pos = enemy->get_position();
					}
					gfx::Sprite *sprite = e->get_sprite();
					gfx::Image *img = sprite->get_current_image();
					util::Point<int> topleft, bottomright;
					img->get_bounds(topleft, bottomright);
					//sprite->get_bounds(topleft, bottomright);
					pos += enemy->get_draw_as_player() ? util::Point<int>(img->size.w-topleft.x-(bottomright.x-topleft.x), topleft.y) : topleft;
					pos.x += (bottomright.x - topleft.x);
					pos.x += shim::tile_size / 4.0f;
					pos.y -= 6;
				}
				else {
					pos = static_cast<Battle_Player *>(e)->get_draw_pos();
					pos.x -= multi_image->size.w;
					pos.y -= 6;
					gfx::Sprite *sprite = selected->get_sprite();
					gfx::Image *img = sprite->get_current_image();
					util::Point<int> topleft, bottomright;
					img->get_bounds(topleft, bottomright);
					pos.x += topleft.x;
					pos.x -= shim::tile_size/4.0f;
				}
				if (e == selected) {
					anim = TTH_GLOBALS->multi_check_checked_selected;
				}
				else {
					anim = TTH_GLOBALS->multi_check_checked;
				}
				if (sneak_attack) {
					pos.x = shim::screen_size.w - pos.x - multi_image->size.w;
				}
				v2.push_back({pos, anim});
			}
			std::vector<wedge::Battle_Entity *> all_targets;
			if (selecting_enemy) {
				all_targets = get_enemies();
			}
			else {
				all_targets = get_players();
			}
			for (auto ee : all_targets) {
				if (std::find(multi_targets.begin(), multi_targets.end(), ee) == multi_targets.end() && ee->get_stats()->hp < 1000000) {
					if (selecting_enemy) {
						auto e = static_cast<wedge::Battle_Enemy *>(ee);
						pos = e->get_position();
						gfx::Sprite *sprite = e->get_sprite();
						gfx::Image *img = sprite->get_current_image();
						util::Point<int> topleft, bottomright;
						img->get_bounds(topleft, bottomright);
						//sprite->get_bounds(topleft, bottomright);
						pos += topleft;
						pos.x += (bottomright.x - topleft.x);
						pos.x += shim::tile_size / 4.0f;
						pos.y -= 6;
					}
					else {
						auto p = static_cast<Battle_Player *>(ee);
						pos = p->get_draw_pos();
						pos.x -= multi_image->size.w;
						pos.y -= 6;
						gfx::Sprite *sprite = ee->get_sprite();
						gfx::Image *img = sprite->get_current_image();
						util::Point<int> topleft, bottomright;
						img->get_bounds(topleft, bottomright);
						pos.x += topleft.x;
						pos.x -= shim::tile_size/4.0f;
					}
					if (sneak_attack) {
						pos.x = shim::screen_size.w - pos.x - multi_image->size.w;
					}

					if (ee == selected) {
						v2.push_back({pos, TTH_GLOBALS->multi_check_unchecked_selected});
					}
					else {
						v2.push_back({pos, TTH_GLOBALS->multi_check_unchecked});
					}
				}
			}

			combo_drawer->set_multi_checkboxes(v2);
		}
		else 
		#endif
		if (shim::guis.size() == 0) {
			if (selecting_enemy) {
				auto e = static_cast<wedge::Battle_Enemy *>(selected);
				if (e->get_draw_as_player()) {
					pos = e->get_sprite_pos();
				}
				else {
					pos = e->get_position();
				}
				gfx::Sprite *sprite = e->get_sprite();
				gfx::Image *img = sprite->get_current_image();
				util::Point<int> topleft, bottomright;
				img->get_bounds(topleft, bottomright);
				//sprite->get_bounds(topleft, bottomright);
				pos += e->get_draw_as_player() ? util::Point<int>(img->size.w-topleft.x-(bottomright.x-topleft.x), topleft.y) : topleft;
				pos.x += (bottomright.x - topleft.x);
				pos.x += shim::tile_size / 4.0f;
				pos.y -= shim::tile_size/4.0f;
				flag = gfx::Image::FLIP_H;
			}
			else {
				auto p = static_cast<Battle_Player *>(selected);
				pos = p->get_draw_pos();
				pos.x -= cursor->size.w;
				pos.y -= shim::tile_size/4.0f;
				gfx::Sprite *sprite = selected->get_sprite();
				gfx::Image *img = sprite->get_current_image();
				util::Point<int> topleft, bottomright;
				img->get_bounds(topleft, bottomright);
				pos.x += topleft.x;
				pos.x -= shim::tile_size/4.0f;
				flag = 0;
			}

			if (sneak_attack) {
				pos.x = shim::screen_size.w - pos.x - cursor->size.w;
				if (flag == 0) {
					flag = gfx::Image::FLIP_H;
				}
				else {
					flag = 0;
				}
			}
			std::vector< std::pair<util::Point<float>, int> > v;
			v.push_back({pos, flag});
			combo_drawer->set_cursors(v);
		}

		return;
	}

#if 0
	if (simple_turn_display == false && GLOBALS->dialogue_active(BATTLE) == false) {
		std::vector<wedge::Battle_Game::Turn *> upcoming_turns = active_turns;
		upcoming_turns.insert(upcoming_turns.end(), turns.begin(), turns.end());

		for (auto it = upcoming_turns.begin(); it != upcoming_turns.end();) {
			auto t = *it;
			if (std::find(entities.begin(), entities.end(), t->actor) == entities.end()) {
				it = upcoming_turns.erase(it);
			}
			else {
				it++;
			}
		}

		if (upcoming_turns.size() > 0) {
			int i = 1;
			int max = int(get_players().size() + get_enemies().size());
			std::map< wedge::Battle_Entity *, std::vector<int> > m;
			for (auto t : upcoming_turns) {
				/*
				if (t->turn_type == wedge::Battle_Game::DIALOGUE) {
					continue;
				}
				*/
				auto e = t->actor;
				auto it = m.find(e);
				if (it == m.end()) {
					std::vector<int> v;
					v.push_back(i);
					m[e] = v;
				}
				else {
					// if using numbers (1, 2, 3) uncomment below. if using dots, keep this commented
					auto &v = *it;
					v.second.push_back(i);
				}
				if (i == max) {
					break;
				}
				i++;
			}
			for (auto &p : m) {
				auto e = p.first;
				auto &v = p.second;
				std::string text;
				for (size_t i = 0; i < v.size(); i++) {
					int n = v[i];
					/*
					for (int i = 0; i < n; i++) {
						text += ".";
					}
					*/
					// if using numbers (1, 2, 3) uncomment below. if using dots, keep this commented
					text += util::itos(n);
					if (i+1 < v.size()) {
						text += ",";
					}
				}
				int tw = shim::font->get_text_width(text);
				int th = shim::font->get_height();
				auto player = dynamic_cast<Battle_Player *>(e);
				util::Point<float> pos;
				bool skip = false;
				if (player) {
					pos = player->get_sprite_pos();
					pos.x -= tw;
					gfx::Sprite *sprite = player->get_sprite();
					gfx::Image *img = sprite->get_current_image();
					util::Point<int> topleft, bottomright;
					img->get_bounds(topleft, bottomright);
					pos.x += topleft.x;
					pos.x -= shim::tile_size/4.0f;
					sprite->get_bounds(topleft, bottomright);
					pos.y += (bottomright.y-topleft.y)/2.0f-th/2.0f;
				}
				else {
					auto enemy = dynamic_cast<Battle_Enemy *>(e);
					gfx::Sprite *sprite = e->get_sprite();
					auto img = sprite->get_current_image();
					util::Point<int> topleft, bottomright;
					img->get_bounds(topleft, bottomright);
					// check for blank frames and skip turn order display if it's blank
					if (topleft.x >= bottomright.x || topleft.y >= bottomright.y) {
						skip = true;
					}
					if (enemy->get_draw_as_player()) {
						pos = enemy->get_sprite_pos();
					}
					else {
						pos = enemy->get_position();
					}
					pos += enemy->get_draw_as_player() ? util::Point<int>(img->size.w-topleft.x-(bottomright.x-topleft.x), topleft.y): topleft;
					pos += util::Point<int>((bottomright.x-topleft.x)/2.0f, (bottomright.y-topleft.y)/2.0f);
					pos -= util::Point<int>(tw/2, th/2);
					pos.y += 4;
					pos += enemy->get_turn_order_display_offset();
					/*
					pos.x += (bottomright.x - topleft.x);
					pos.x += shim::tile_size / 4;
					pos.y += (bottomright.y-topleft.y)/2-th/4;
					*/
				}
				if (skip == false) {
					if (sneak_attack) {
						pos.x = shim::screen_size.w - pos.x - tw;
					}
					shim::font->enable_shadow(shim::black, gfx::Font::FULL_SHADOW);
					shim::font->draw(shim::palette[36], text, pos);
					shim::font->disable_shadow();
				}
			}
		}

		for (auto p : started_turns) {
			gfx::Image *arrow = p.second->get_current_image();
			auto player = dynamic_cast<Battle_Player *>(p.first);
			util::Point<float> pos;
			if (player) {
				pos = player->get_sprite_pos();
				gfx::Sprite *sprite = player->get_sprite();
				gfx::Image *img = sprite->get_current_image();
				util::Point<int> topleft, bottomright;
				img->get_bounds(topleft, bottomright);
				//pos.x += topleft.x;
				pos += topleft;
				pos.x += (bottomright.x-topleft.x)/2.0f;
				pos.x -= arrow->size.w/2.0f;
				pos.y -= (arrow->size.h+1);
			}
			else {
				auto enemy = dynamic_cast<Battle_Enemy *>(p.first);
				if (enemy->get_draw_as_player()) {
					pos = enemy->get_sprite_pos();
				}
				else {
					pos = enemy->get_position();
				}
				gfx::Sprite *sprite = enemy->get_sprite();
				gfx::Image *img = sprite->get_current_image();
				util::Point<int> topleft, bottomright;
				img->get_bounds(topleft, bottomright);
				pos.x += enemy->get_draw_as_player() ? img->size.w-topleft.x-(bottomright.x-topleft.x) : topleft.x;
				pos.y += topleft.y;
				pos.x += (bottomright.x-topleft.x)/2.0f;
				pos.x -= arrow->size.w/2.0f;
				pos.y -= (arrow->size.h+1);
			}
			if (sneak_attack) {
				pos.x = shim::screen_size.w - pos.x - arrow->size.w;
			}
			pos.x += 0.5f; // arrow sprite frames are 20px even though only fills 19 - this centres it better
			arrow->draw(pos);
		}
	}
#endif

	gfx::Font::end_batches();

	const int EDGE_X = shim::screen_size.w * 0.05f;
	const int EDGE_Y = shim::screen_size.h * 0.05f;

	int w = (shim::screen_size.w - EDGE_X*2) / 2;
	int h = shim::font->get_height() * 3 + 6; // 6 = border
	int y = shim::screen_size.h-EDGE_Y-h;

	for (size_t i = 0; i < entities.size(); i++) {
		wedge::Battle_Entity *entity = entities[i];
		entity->draw_fore();
	}

	if (has_darkness) {
		real_draw_darkness(darkness_image1, darkness_image2, darkness_offset1, darkness_offset2, {0.0f, 0.0f}, 0.5f);
	}

	if (window_shown) {
		gfx::draw_9patch(TTH_GLOBALS->battle_window, util::Point<int>(EDGE_X, y), util::Size<int>(w*2, h));
		
		gfx::draw_line(shim::palette[186], util::Point<int>(EDGE_X+w, y+1), util::Point<int>(EDGE_X+w, y+h-3));

		std::map<std::string, int> enemies;

		std::vector<wedge::Battle_Entity *> players = get_players();

		SDL_Colour c;
		c.r = 192;
		c.g = 192;
		c.b = 192;
		c.a = 255;

		int index = 0;
		int o = 4 + ((h - 6) / 3) / 2 - shim::font->get_height() / 2;
		if (player_stats_shown) {
			for (auto &p : players) {
				auto s = p->get_stats();
				TTH_GLOBALS->heart->get_current_image()->draw({float(EDGE_X+w+WIN_BORDER), float(y+o-(16-shim::font->get_height())/2+2)});
				shim::font->draw(c, util::itos(s->hp) + "/" + util::itos(s->fixed.max_hp), util::Point<int>(EDGE_X+w+WIN_BORDER+20, y+o));
				index++;
				o += (h - 6) / 3 + 3;
			}
		}
		
		TTH_GLOBALS->doughnut->get_current_image()->draw({float(EDGE_X+w+WIN_BORDER), float(y+o-(16-shim::font->get_height())/2+2)});
		shim::font->draw(c, util::itos(INSTANCE->get_gold()), util::Point<int>(EDGE_X+w+WIN_BORDER+20, y+o));
	}

	Game::draw();

	std::vector<wedge::Battle_Game::Turn *> t = active_turns;
	t.insert(t.end(), turns.begin(), turns.end());

	for (auto it = t.begin(); it != t.end();) {
		wedge::Battle_Game::Turn *turn = *it;
		// Enemy could have been deleted/killed
		if (std::find(entities.begin(), entities.end(), turn->actor) == entities.end()) {
			it = t.erase(it);
		}
		else {
			it++;
		}
	}

	if (detecting_combo) {
		Combo c;
		for (auto d : detector->get_detected()) {
			Combo_Event e;
			e.button = d.button;
			c.push_back(e);
		}

		combo_drawer->set(c);
	}
}

wedge::Battle_Game::Turn *Battle_Game::get_turn(Battle_Player *player)
{
	/*
	if (player->get_sprite()->get_animation() == "attack_defend") {
		return nullptr;
	}
	*/

	if (GLOBALS->is_mini_paused()) {
		// can be here if sneak attack notification comes up
		return nullptr;
	}

	auto v = get_players();
	bool all_dead = true;
	for (auto p : v) {
		if (p->get_stats()->hp > 0) {
			all_dead = false;
			break;
		}
	}
	if (all_dead) {
		if (getting_turn) {
			getting_turn = false;
			if (turn_gui_gone == false) {
				turn_gui->exit();
				turn_gui_gone = true;
			}
			enemy_stats_shown = true;
			player_stats_shown = true;
			if (item_gui) {
				item_gui->exit();
				item_gui = nullptr;
			}
			if (easy_gui) {
				easy_gui->exit();
				easy_gui = nullptr;
			}
			if (mc_gui) {
				mc_gui->exit();
				mc_gui = nullptr;
			}
			end_combo();
			good_combo = -1;
			remove_notification(RIGHT);
		}
		return nullptr;
	}
		
	int index = get_player_index(player);

	auto p = dynamic_cast<Battle_Player *>(v[active_player]);
	// If active player has died...
	if (p->get_stats()->hp <= 0) {
		next_player(-1, true);
		getting_turn = false;
		if (turn_gui_gone == false) {
			turn_gui->exit();
			turn_gui_gone = true;
		}
		enemy_stats_shown = true;
		player_stats_shown = true;
		if (item_gui) {
			item_gui->exit();
			item_gui = nullptr;
		}
		if (easy_gui) {
			easy_gui->exit();
			easy_gui = nullptr;
		}
		if (mc_gui) {
			mc_gui->exit();
			mc_gui = nullptr;
		}
		end_combo();
		good_combo = -1;
		remove_notification(RIGHT);
		// this return moved into if
		return nullptr;
	}

	if (index != active_player) {
		return nullptr;
	}

	if (next_turn != nullptr) {
		// Need to set next_turn to nullptr
		wedge::Battle_Game::Turn *t = next_turn;
		next_turn = nullptr;
		getting_turn = false;
		return t;
	}
		
	const int EDGE_X = shim::screen_size.w * 0.05f;
	const int EDGE_Y = shim::screen_size.h * 0.05f;

	if (getting_turn == false) {
		getting_turn = true;
		window_shown = true;
		enemy_stats_shown = false;
		turn_type = wedge::Battle_Game::NONE;
		std::vector<std::string> choices;
		if (have_samurai_sword()) {
			choices.push_back(GLOBALS->game_t->translate(611)/* Originally: Slice */);
		}
		else {
			choices.push_back(GLOBALS->game_t->translate(592)/* Originally: Bonk */);
		}
		choices.push_back(GLOBALS->game_t->translate(579)/* Originally: Doughnut */);
		choices.push_back(GLOBALS->game_t->translate(593)/* Originally: Run */);
		turn_gui = new Positioned_Multiple_Choice_GUI(false, "", choices, -1, -1, 1, EDGE_X, 0, 0, EDGE_Y, 0, 0, turn_callback, this, 3, 1, true,  0.571f, NULL, false, 3, false);
		turn_gui->set_transition(false);
		Widget_List *list = turn_gui->get_list();
		//list->set_text_shadow_colour(shim::palette[23]);
		list->set_row_h(shim::font->get_height());
		//list->set_text_offset(util::Point<int>(0, 4));
		list->set_arrow_colour(shim::palette[8]);
		//list->set_text_shadow_colour(shim::palette[23]);
		list->set_selected(next_turn_pos);
		list->set_top(next_turn_top);
		next_turn_pos = 0;
		next_turn_top = 0;
		turn_gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
		shim::guis.push_back(turn_gui);
		turn_gui_gone = false;
	}
	else if (turn_type == wedge::Battle_Game::ATTACK) {
	#if 0
		if (good_combo < 0) {
			if (easy_combos) {
				if (easy_gui == nullptr) {
					enemy_stats_shown = false;
					player_stats_shown = false;

					#if 0
					std::vector<int> quantities;
					std::vector<std::string> names;
					std::vector<std::string> descriptions; // keep this empty, we don't do descriptions in battle

					for (auto cmbo : TTH_INSTANCE->combos[active_player]) {
						int cost = get_combo_cost(cmbo);
						quantities.push_back(cost);
						names.push_back(GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(cmbo)));
					}

					if (names.size() == 0) {
						getting_turn = false;
						gfx::add_notification(GLOBALS->game_t->translate(375)/* Originally: No combos... */);
						TTH_GLOBALS->error_sfx->play(false);
						enemy_stats_shown = true;
						player_stats_shown = true;
					}
					else {
						Widget_Combo_List *list = new Widget_Combo_List(1.0f, 3 * shim::font->get_height());
						//list->set_text_shadow_colour(shim::palette[23]);

						list->set_items_extra(quantities, names, descriptions);
						//list->set_text_shadow_colour(shim::palette[23]);
						list->set_row_h(shim::font->get_height());
						//list->set_text_offset(util::Point<int>(0, 4));
						list->set_arrow_colour(shim::palette[8]);
						list->set_arrow_shadow_colour(shim::black);
						//list->set_text_shadow_colour(shim::palette[23]);
						list->set_arrow_offsets(util::Point<int>(0, -5), util::Point<int>(0, 0));
						#if 0
						if (easy_combos && INSTANCE->is_milestone_complete(MS_DID_AN_ATTACK) == false) {
							std::vector<std::string> tips;
							tips.push_back(GLOBALS->game_t->translate(408)/* Originally: Pick a move! */);
							list->set_tips(tips);
						}
						#endif

						int width = shim::screen_size.w - shim::screen_size.w * 0.1f - 3*2/*battle window border aka win_border */ - WIN_BORDER*2;

						std::vector<std::string> choices;
						choices.insert(choices.end(), names.begin(), names.end());
						easy_gui = new Positioned_Multiple_Choice_GUI(false, "", choices, -3, -1, 1, EDGE_X, 0, 0, EDGE_Y, 0, 0, easy_callback, this, 3, width, false,  0.75f, list, false, 3);
						easy_gui->set_transition(false);
						easy_gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
						shim::guis.push_back(easy_gui);
					}
					#endif
				}
			}
			else {
				if (detecting_combo == false) {
					detecting_combo = true;
					GLOBALS->allow_global_mini_pause = false;
					GLOBALS->getting_combo = true;
					shim::convert_directions_to_focus_events = false;
					osc_enabled = GLOBALS->onscreen_controller_was_enabled;

					if (input::is_joystick_connected() == false && input::system_has_keyboard() == false) {
						GLOBALS->onscreen_controller_was_enabled = true;
						started_with_osc = true;
					}
					else {
						GLOBALS->onscreen_controller_was_enabled = false;
						started_with_osc = false;
					}
					wedge::set_onscreen_controller_generates_b1_b2(true);
					wedge::set_onscreen_controller_b2_enabled(true);

					combos.clear();

					for (size_t i = 0; i < TTH_INSTANCE->combos[active_player].size(); i++) {
						std::string name = TTH_INSTANCE->combos[active_player][i];
						Combo c = get_combo(name);
						combos.push_back(c);
					}

					detector = new Combo_Detector(combos);
				}
			}
		}
		else if (selecting == false) {
		#endif
		if (selecting == false) {
			end_combo();
			
			acting_player = player;

			int player_index = get_player_index(acting_player);
			#if 0
			std::string cmbo = TTH_INSTANCE->combos[player_index][good_combo];
			std::string lower = util::lowercase(cmbo);

			if (lower == "defend") {
				getting_turn = false;

				if (get_num_turns(acting_player) > 0) {
					TTH_GLOBALS->error_sfx->play(false);
					gfx::add_notification(GLOBALS->game_t->translate(573)/* Originally: Can't defend while in action... */);
				}
				else {
					//acting_player->get_sprite()->set_animation("attack_defend");
					acting_player->get_stats()->mp -= 100;
				}

				good_combo = -1;

				return nullptr;
			}
			else {
				if (get_combo_friendly(cmbo)) {
					selecting_enemy = false;
					selected = get_players()[0];
				}
				else {
			#endif
					selecting_enemy = true;
					auto enemies = get_enemies();
					std::sort(enemies.begin(), enemies.end(), enemy_x_compare);
					if (enemies.size() > 1) {
						for (auto it = enemies.begin(); it != enemies.end();) {
							auto e = *it;
							if (e->get_stats()->hp >= 1000000) {
								it = enemies.erase(it);
							}
							else {
								it++;
							}
						}
					}
					selected = enemies[0];
			//	}

				selecting = true;

				selecting_multi = false;//get_combo_multi(cmbo);

				if (selecting_multi) {
					/*
					mc_gui = new Battle_Multi_Confirm_GUI(multi_callback, this);
					mc_gui->resize(shim::screen_size);
					shim::guis.push_back(mc_gui);
					*/
				}
			//}
		//}
		}
		return nullptr;
	}
	else if (turn_type == wedge::Battle_Game::ITEM) {
		if (item_gui == nullptr && item_index < 0) {
			enemy_stats_shown = false;
			player_stats_shown = false;

			inventory_indices.clear();

			std::vector<int> quantities;
			std::vector<std::string> names;
			std::vector<std::string> descriptions; // keep this empty, we don't do descriptions in battle

			wedge::Object *inv = INSTANCE->inventory.get_all();

			for (int i = 0; i < wedge::Inventory::MAX_OBJECTS; i++) {
				if (inv[i].type == wedge::OBJECT_ITEM) {
					if (inv[i].quantity > inv[i].used) {
						inventory_indices.push_back(i);
						quantities.push_back(inv[i].quantity - inv[i].used);
						names.push_back(inv[i].name);
					}
				}
			}

			if (INSTANCE->get_gold() <= 0) {
				getting_turn = false;
				gfx::add_notification(GLOBALS->game_t->translate(174)/* Originally: No doughnuts! */);
				TTH_GLOBALS->error_sfx->play(false);
				enemy_stats_shown = true;
				player_stats_shown = true;
			}
			else {
			#if 0
				Widget_Quantity_List *list = new Widget_Quantity_List(1.0f, 3 * shim::font->get_height());
				//list->set_text_shadow_colour(shim::palette[23]);

				list->set_items_extra(quantities, names, descriptions);
				//list->set_text_shadow_colour(shim::palette[23]);
				list->set_row_h(shim::font->get_height());
				//list->set_text_offset(util::Point<int>(0, 4));
				list->set_arrow_colour(shim::palette[8]);
				list->set_arrow_shadow_colour(shim::black);
				//list->set_text_shadow_colour(shim::palette[23]);
				list->set_arrow_offsets(util::Point<int>(0, -5), util::Point<int>(0, 0));
		
				int width = shim::screen_size.w - shim::screen_size.w * 0.1f - 3*2/*battle window border aka win_border */ - WIN_BORDER*2;

				std::vector<std::string> choices;
				choices.insert(choices.end(), names.begin(), names.end());
				item_gui = new Positioned_Multiple_Choice_GUI(false, "", choices, -3, -1, 1, EDGE_X, 0, 0, EDGE_Y, 0, 0, item_callback, this, 3, width, false,  0.571f, list, false, 3);
				item_gui->set_transition(false);
				item_gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
				shim::guis.push_back(item_gui);
			#endif
				auto t = new wedge::Battle_Game::Turn;
				t->actor = player;
				t->turn_type = wedge::Battle_Game::ITEM;
				t->turn_name = "item";
				//wedge::Object *inv = INSTANCE->inventory.get_all();
				//add_notification(RIGHT, inv[i].name);
				t->targets.push_back(get_players()[0]);
				t->started = false;

				next_turn = t;

				selecting = false;
				
				enemy_stats_shown = true;
				player_stats_shown = true;
			}
		}
		/*
		else if (item_index >= 0 && selecting == false) {
			wedge::Object *inv = INSTANCE->inventory.get_all();
			int i = inventory_indices[item_index];
			selecting = true;
			selecting_enemy = false;
			selecting_multi = false;
			selected = get_players()[0];
			acting_player = player;
		}
		*/
	}
	else if (turn_type == wedge::Battle_Game::RUN) {
		getting_turn = false;

		auto t = new wedge::Battle_Game::Turn;

		t->actor = player;
		t->turn_type = wedge::Battle_Game::RUN;
		t->turn_name = "";
		t->started = false;

		return t;
	}

	return nullptr;
}

void Battle_Game::set_turn(int type)
{
	turn_gui_gone = true;

	enemy_stats_shown = true;
	player_stats_shown = true;

	auto v = get_players();
	bool all_dead = true;
	for (auto p : v) {
		if (p->get_stats()->hp > 0) {
			all_dead = false;
			break;
		}
	}
	if (all_dead) {
		getting_turn = false;
		return;
	}

	if (type < 0) {
		getting_turn = false;
	}
	else {
		turn_type = (wedge::Battle_Game::Turn_Type)(type+1);
		if (turn_type == wedge::Battle_Game::RUN) {
			window_shown = false;
		}
	}
}

void Battle_Game::set_item(int item)
{
	item_gui = nullptr;

	enemy_stats_shown = true;
	player_stats_shown = true;

	auto v = get_players();
	bool all_dead = true;
	for (auto p : v) {
		if (p->get_stats()->hp > 0) {
			all_dead = false;
			break;
		}
	}
	if (all_dead) {
		getting_turn = false;
		return;
	}

	if (item < 0) {
		just_exited_item_gui = true;

		getting_turn = false;
		next_turn_pos = 1;
	}
	else {
		item_index = item;
		wedge::Object *inv = INSTANCE->inventory.get_all();
		add_notification(RIGHT, inv[item_index].name, 1000000000);
	}
}

void Battle_Game::set_easy(int easy)
{
	easy_gui = nullptr;

	enemy_stats_shown = true;
	player_stats_shown = true;

	auto v = get_players();
	bool all_dead = true;
	for (auto p : v) {
		if (p->get_stats()->hp > 0) {
			all_dead = false;
			break;
		}
	}
	if (all_dead) {
		getting_turn = false;
		return;
	}

	/*
	if (easy < 0) {
		just_exited_easy_gui = true;

		getting_turn = false;
		next_turn_pos = 0;
	}
	else {
	*/
		good_combo = easy;
	#if 0
		std::string cmbo = TTH_INSTANCE->combos[active_player][good_combo];
		int cost = get_combo_cost(cmbo);
		if (cost > INSTANCE->stats[active_player].base.mp) {
			auto v = get_players();
			auto p = dynamic_cast<Battle_Player *>(v[active_player]);
			p->reset_mult_level();
			TTH_GLOBALS->error_sfx->play(false);
			gfx::add_notification(util::string_printf(GLOBALS->game_t->translate(230)/* Originally: Not enough SP! Need %d! */.c_str(), cost));
			close_guis();
		}
		else {
	#endif
			TTH_GLOBALS->button->play(false);
			/*
			cmbo = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(cmbo));
			std::string lower = util::lowercase(cmbo);
			if (lower == "defend") {
				add_notification(RIGHT, cmbo, NOTIFICATION_LIFETIME);
			}
			else {
				add_notification(RIGHT, cmbo, 1000000000);
			}
		}
	}
	*/
}

void Battle_Game::handle_event(TGUI_Event *event)
{
	bool used = false;
	
	const int EDGE_X = shim::screen_size.w * 0.05f;
	const int EDGE_Y = shim::screen_size.h * 0.05f;
	int w = (shim::screen_size.w - EDGE_X*2) / 2;
	int h = shim::font->get_height() * 3 + 6; // 6 = border
	int y = shim::screen_size.h-EDGE_Y-h;
	int xx = EDGE_X + w;
	int yy = y + 3;

	if (done == false) {
		auto enemies = get_enemies();
		bool go = false;
		if (getting_turn && event->type == TGUI_MOUSE_DOWN) {
			for (int i = (int)enemies.size()-1; i >= 0; i--) {
				wedge::Battle_Enemy *enemy = static_cast<wedge::Battle_Enemy *>(enemies[i]);
				util::Point<float> pos = enemy->get_position();
				gfx::Sprite *sprite = enemy->get_sprite();
				gfx::Image *img = sprite->get_current_image();
				util::Point<int> topleft, bottomright;
				img->get_bounds(topleft, bottomright);
				pos += topleft;
				util::Size<int> sz(bottomright.x-topleft.x, bottomright.y-topleft.y);
				int click_zone_inc = enemy->get_click_zone_inc();
				pos.x -= click_zone_inc;
				pos.y -= click_zone_inc;
				sz.w += click_zone_inc*2;
				sz.h += click_zone_inc*2;
				if (sneak_attack) {
					pos.x = shim::screen_size.w - pos.x - sz.w;
				}
				util::Point<int> m(event->mouse.x, event->mouse.y);
				if (m.x >= pos.x && m.y >= pos.y && m.x < pos.x+sz.w && m.y < pos.y+sz.h) {
					if (enemies[i]->get_stats()->hp < 1000000) {
						selected = enemies[i];
						go = true;
						break;
					}
				}
			}
			if (go) {
				if (turn_gui_gone == false) {
					turn_gui->exit();
				}
				set_turn(0);
				get_turn((Battle_Player *)get_players()[0]);
			}
		}
		if (selecting) {
			fixup_selection();
			if (selecting) {
				if (event->type == TGUI_MOUSE_DOWN) {
					std::sort(enemies.begin(), enemies.end(), entity_y_compare);
				}
				else {
					std::sort(enemies.begin(), enemies.end(), enemy_x_compare);
				}
				if (event->type == TGUI_MOUSE_DOWN) {
					if (go) {
						// Fake an event so the if below runs
						event->type = TGUI_KEY_DOWN;
						event->keyboard.code = GLOBALS->key_action;
						event->keyboard.simulated = true;
					}
				}
				if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_action) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_action)) {
					if (selecting_multi) {
						std::vector<wedge::Battle_Entity *>::iterator it;
						if ((it = std::find(multi_targets.begin(), multi_targets.end(), selected)) != multi_targets.end()) {
							multi_targets.erase(it);
						}
						else {
							multi_targets.push_back(selected);
						}
						TTH_GLOBALS->button->play(false);
					}
					else {
						if (turn_type == wedge::Battle_Game::ATTACK) {
							#if 0
							if (INSTANCE->is_milestone_complete(MS_DID_AN_ATTACK) == false) {
								gfx::add_notification(GLOBALS->game_t->translate(409)/* Originally: Good job! */);
								INSTANCE->set_milestone_complete(MS_DID_AN_ATTACK, true);
							}
							#endif

							auto t = new wedge::Battle_Game::Turn;

							t->actor = acting_player;
							t->turn_type = wedge::Battle_Game::ATTACK;
							int player_index = get_player_index(acting_player);
							/*
							std::string cmbo = TTH_INSTANCE->combos[player_index][good_combo];
							std::string bak = cmbo;
							int pos;
							if ((pos = cmbo.find('\'')) != std::string::npos) {
								cmbo = cmbo.substr(0, pos) + cmbo.substr(pos+1);
							}
							int cost = get_combo_cost(cmbo);
							INSTANCE->stats[player_index].base.mp -= cost;
							t->turn_name = std::string("attack_") + util::lowercase(cmbo);
							for (size_t i = 0; i < t->turn_name.length(); i++) {
								if (t->turn_name[i] == ' ') {
									t->turn_name[i] = '_';
								}
							}
							add_notification(RIGHT, GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(bak)));
							*/
							t->turn_name = "attack";
							good_combo = -1;
							t->targets.push_back(selected);
							t->started = false;

							next_turn = t;

							selecting = false;
							
							GLOBALS->button->play(false);
						}
						else if (turn_type == wedge::Battle_Game::ITEM) {
							auto t = new wedge::Battle_Game::Turn;
					
							t->actor = acting_player;
							t->turn_type = wedge::Battle_Game::ITEM;
							int i = inventory_indices[item_index];
							t->turn_name = util::itos(i);
							wedge::Object *inv = INSTANCE->inventory.get_all();
							inv[i].used++;
							add_notification(RIGHT, inv[i].name);
							t->targets.push_back(selected);
							t->started = false;

							next_turn = t;

							selecting = false;
							
							GLOBALS->button->play(false);

							item_index = -1;
						}
					}
					used = true;
				}
				else if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
					selecting = false;
					getting_turn = false;
					GLOBALS->button->play(false);
					item_gui = nullptr;
					easy_gui = nullptr;
					mc_gui = nullptr;
					if (turn_type == wedge::Battle_Game::ATTACK) {
						next_turn_pos = 0;
					}
					else {
						next_turn_pos = 1;
					}
					item_index = -1;
					good_combo = -1;
					used = true;
					remove_notification(RIGHT);
				}
				else if (event->type == TGUI_FOCUS) {
					auto players = get_players();
					std::vector<wedge::Battle_Entity *> v;
					if (selecting_enemy) {
						v = enemies;
						if (v.size() > 1) {
							for (auto it = v.begin(); it != v.end();) {
								auto e = *it;
								if (e->get_stats()->hp >= 1000000) {
									it = v.erase(it);
								}
								else {
									it++;
								}
							}
						}
					}
					else {
						v = players;
					}
					int index = 0;
					for (size_t i = 0; i < v.size(); i++) {
						if (v[i] == selected) {
							index = (int)i;
							break;
						}
					}
					if (sneak_attack) {
						if (event->focus.type == TGUI_FOCUS_RIGHT || event->focus.type == TGUI_FOCUS_UP) {
							index += selecting_enemy ? 1 : -1;
						}
						else {
							index -= selecting_enemy ? 1 : -1;
						}
					}
					else {
						if (event->focus.type == TGUI_FOCUS_LEFT || event->focus.type == TGUI_FOCUS_UP) {
							index += selecting_enemy ? 1 : -1;
						}
						else {
							index -= selecting_enemy ? 1 : -1;
						}
					}
					if (index >= (int)v.size()) {
						index = 0;
					}
					if (index < 0) {
						index = int(v.size()-1);
					}
					selected = v[index];
					shim::widget_sfx->play(false);
					used = true;
				}
			}
		}
	}

	if (detecting_combo) {
		if (!(input::is_joystick_connected() && event->type == TGUI_KEY_DOWN && get_key_name(event->keyboard.code) == "")) {
			int nok = detector->num_ok();
			detector->handle_event(event);
			detector->check();
			int nok2 = detector->num_ok();
			if (nok2 > nok) {
				/*
				auto v = get_players();
				auto p = dynamic_cast<Battle_Player *>(v[active_player]);
				p->inc_mult_level();
				*/
			}
			if (detector->error()) {
				auto v = get_players();
				auto p = dynamic_cast<Battle_Player *>(v[active_player]);
				p->reset_mult_level();
				good_combo = -1;
				Uint32 now = GET_TICKS();
				TTH_GLOBALS->error_sfx->play(false);
				detector->reset();
				combo_drawer->next_caption();
			}
			else if (detector->good() != -1) {
				good_combo = detector->good();
				std::string cmbo = TTH_INSTANCE->combos[active_player][good_combo];
				int cost = get_combo_cost(cmbo);
				if (cost > INSTANCE->stats[active_player].base.mp) {
					auto v = get_players();
					auto p = dynamic_cast<Battle_Player *>(v[active_player]);
					p->reset_mult_level();
					detector->reset();
					TTH_GLOBALS->error_sfx->play(false);
					gfx::add_notification(util::string_printf(GLOBALS->game_t->translate(230)/* Originally: Not enough SP! Need %d! */.c_str(), cost));
					close_guis();
				}
				else {
					//TTH_GLOBALS->combo->play(false);
					detector->reset();
					end_combo();
					TTH_GLOBALS->button->play(false);
					std::string cmbo_t = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(cmbo));
					combo_drawer->set(cmbo);
					add_notification(RIGHT, cmbo_t, 1000000000);
				}
			}
		}
	}
	else if (used == false) {
		Game::handle_event(event);
	}

	just_exited_item_gui = false;
	just_exited_easy_gui = false;
	just_exited_multi_gui = false;
}

bool Battle_Game::run()
{
	if (detecting_combo) {
		detector->check();
		if (detector->error()) {
			auto v = get_players();
			auto p = dynamic_cast<Battle_Player *>(v[active_player]);
			p->reset_mult_level();
			good_combo = -1;
			Uint32 now = GET_TICKS();
			TTH_GLOBALS->error_sfx->play(false);
			detector->reset();
			combo_drawer->next_caption();
		}
		else if (detector->good() != -1) {
			good_combo = detector->good();
			std::string cmbo = TTH_INSTANCE->combos[active_player][good_combo];
			int cost = get_combo_cost(cmbo);
			if (cost > INSTANCE->stats[active_player].base.mp) {
				auto v = get_players();
				auto p = dynamic_cast<Battle_Player *>(v[active_player]);
				p->reset_mult_level();
				detector->reset();
				TTH_GLOBALS->error_sfx->play(false);
				gfx::add_notification(util::string_printf(GLOBALS->game_t->translate(230)/* Originally: Not enough SP! Need %d! */.c_str(), cost));
				close_guis();
			}
			else {
				//TTH_GLOBALS->combo->play(false);
				detector->reset();
				end_combo();
				TTH_GLOBALS->button->play(false);
				cmbo = GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(cmbo));
				combo_drawer->set(cmbo);
				add_notification(RIGHT, cmbo, 1000000000);
			}
		}
	}

	if (done == false) {
		int max = 0;
		for (auto &s : INSTANCE->stats) {
			if (s.base.fixed.max_mp > max) {
				max = s.base.fixed.max_mp;
			}
		}
		// Fill all SP after 1 minute
		int inc = 1000000 / 60 / 60 * max;
		TTH_INSTANCE->sp_replenish_count += inc;
	}

	bool running = false;
	for (auto p : get_players()) {
		if (static_cast<Battle_Player *>(p)->is_running()) {
			running = true;
			break;
		}
	}
	if (running) {
		if (getting_turn) {
			if (turn_gui_gone == false) {
				turn_gui->exit();
				turn_gui_gone = true;
			}
			getting_turn = false;
		}
		if (item_gui) {
			item_gui->exit();
			item_gui = nullptr;
		}
		if (easy_gui) {
			easy_gui->exit();
			easy_gui = nullptr;
		}
		if (mc_gui) {
			mc_gui->exit();
			mc_gui = nullptr;
		}
		end_combo();
		good_combo = -1;
		remove_notification(LEFT);
		remove_notification(RIGHT);
	}

	bool all_dead = true;
	auto v = get_enemies();
	for (auto &e : v) {
		if (e->get_stats()->hp > 0) {
			all_dead = false;
		}
	}
	if (all_dead == false) {
		all_dead = true;
		auto v = get_players();
		for (auto &e : v) {
			if (e->get_stats()->hp > 0) {
				all_dead = false;
			}
		}
	}

	if (all_dead) {
		do_all_dead();
	}

	bool ret = wedge::Battle_Game::run();
	if (ret == false) {
		save_play_time();
		return ret;
	}

	if (selecting) {
		fixup_selection();
	}
	else if (getting_turn && (turn_type == wedge::Battle_Game::NONE || selecting || detecting_combo || item_gui != nullptr || easy_gui != nullptr || mc_gui != nullptr)) {
		auto v = get_players();
		auto p = dynamic_cast<Battle_Player *>(v[active_player]);
		// If active player has died...
		if (p->get_stats()->hp <= 0) {
			next_player(-1, true);
			getting_turn = false;
			if (turn_gui_gone == false) {
				turn_gui->exit();
				turn_gui_gone = true;
			}
			enemy_stats_shown = true;
			player_stats_shown = true;
			if (item_gui) {
				item_gui->exit();
				item_gui = nullptr;
			}
			if (easy_gui) {
				easy_gui->exit();
				easy_gui = nullptr;
			}
			if (mc_gui) {
				mc_gui->exit();
				mc_gui = nullptr;
			}
			end_combo();
			good_combo = -1;
			remove_notification(RIGHT);
		}
		else {
			auto v2 = get_enemies();
			bool all_enemies_dead = v2.size() == 0;
			if (all_enemies_dead == false) {
				all_enemies_dead = true;
				for (auto e : v2) {
					if (e->get_stats()->hp > 0) {
						all_enemies_dead = false;
						break;
					}
				}
			}
			// not else
			if (all_enemies_dead) {
				getting_turn = false;
				if (turn_gui_gone == false) {
					turn_gui->exit();
					turn_gui_gone = true;
				}
				enemy_stats_shown = true;
				player_stats_shown = true;
				if (item_gui) {
					item_gui->exit();
					item_gui = nullptr;
				}
				if (easy_gui) {
					easy_gui->exit();
					easy_gui = nullptr;
				}
				if (mc_gui) {
					mc_gui->exit();
					mc_gui = nullptr;
				}
				end_combo();
				good_combo = -1;
			}
		}
	}

	int st_count = 0;
	while (st_count < (int)started_turns_to_delete.size()) {
		gfx::Sprite *s = started_turns_to_delete[st_count];
		bool found = false;
		for (auto it = started_turns.begin(); it != started_turns.end(); it++) {
			auto p = *it;
			if (p.second == s) {
				started_turns.erase(it);
				delete s;
				found = true;
				break;
			}
		}
		if (found == false) {
			st_count++;
		}
	}

	started_turns_to_delete.clear();

	return true;
}

void Battle_Game::fixup_selection()
{
	if (selecting) {
		auto players = get_players();
		auto p = dynamic_cast<Battle_Player *>(players[active_player]);
		if (p->get_stats()->hp <= 0) {
			next_player();
			if (selecting) {
				selecting = false;
				getting_turn = false;
			}
			if (item_gui) {
				item_gui->exit();
				item_gui = nullptr;
				getting_turn = false;
				selecting = false;
			}
			if (easy_gui) {
				easy_gui->exit();
				easy_gui = nullptr;
				getting_turn = false;
				selecting = false;
			}
			if (mc_gui) {
				mc_gui->exit();
				mc_gui = nullptr;
				getting_turn = false;
				selecting = false;
			}
			end_combo();
			good_combo = -1;
			remove_notification(RIGHT);
		}
		else if (selecting_enemy) {
			auto enemies = get_enemies();
			std::sort(enemies.begin(), enemies.end(), enemy_x_compare);
			if (enemies.size() > 1) {
				for (auto it = enemies.begin(); it != enemies.end();) {
					auto e = *it;
					if (e->get_stats()->hp >= 1000000) {
						it = enemies.erase(it);
					}
					else {
						it++;
					}
				}
			}
			if (enemies.size() == 0) {
				selecting = false;
				turn_type = wedge::Battle_Game::NONE;
			}
			else {
				if (std::find(enemies.begin(), enemies.end(), selected) == enemies.end()) {
					selected = enemies[0];
				}
				else {
					if (selected->get_stats()->hp <= 0) {
						bool found = false;
						for (auto e : enemies) {
							if (e != selected) {
								if (e->get_stats()->hp > 0 && e->get_stats()->hp < 1000000) {
									found = true;
									selected = e;
									break;
								}
							}
						}
						if (found == false) {
							selecting = false;
							turn_type = wedge::Battle_Game::NONE;
						}
					}
				}
			}
		}
	}
}

int Battle_Game::get_player_index(Battle_Player *player)
{
	int index = 0;
	auto v = get_players();

	for (size_t i = 0; i < v.size(); i++) {
		if (v[i] == player) {
			index = (int)i;
			break;
		}
	}

	return index;
}

void Battle_Game::next_player(int direct, bool no_sound)
{
	return;
	#if 0
	auto v = get_players();
	auto start_p = dynamic_cast<Battle_Player *>(v[active_player]);
	if (direct >= 0) {
		active_player = direct;
	}
	else {
		active_player++;
	}
	active_player %= v.size();
	auto p = dynamic_cast<Battle_Player *>(v[active_player]);
	int count = 1;
	while (p->get_stats()->hp <= 0 || p->get_sprite()->get_animation() == "attack_defend") {
		if (count >= (int)v.size()) {
			break;
		}
		active_player++;
		active_player %= v.size();
		p = dynamic_cast<Battle_Player *>(v[active_player]);
		count++;
	}
	if (p->get_stats()->hp > 0 && p != start_p && no_sound == false) {
		shim::widget_sfx->play(false);
	}
	#endif
}

void Battle_Game::end_combo()
{
	detecting_combo = false;
	GLOBALS->allow_global_mini_pause = true;
	GLOBALS->getting_combo = false;
	shim::convert_directions_to_focus_events = true;
	GLOBALS->onscreen_controller_was_enabled = osc_enabled;
	/*
	wedge::set_onscreen_controller_generates_b1_b2(false);
	wedge::set_onscreen_controller_b2_enabled(false);
	*/
	delete detector;
	detector = nullptr;
}

int Battle_Game::get_num_turns(wedge::Battle_Entity *e)
{
	int turn_count = 0;
	for (auto t : active_turns) {
		if (t->actor == e) {
			turn_count++;
		}
	}
	for (auto t : turns) {
		if (t->actor == e) {
			turn_count++;
		}
	}
	return turn_count;
}

void Battle_Game::resize(util::Size<int> new_size)
{
	wedge::Battle_Game::resize(new_size);
	
	const int EDGE_X = shim::screen_size.w * 0.05f;
	const int EDGE_Y = shim::screen_size.h * 0.05f;

	if (item_gui) {
		item_gui->set_padding(EDGE_X, 0, 0, EDGE_Y);
		item_gui->resize(new_size);
		int width = shim::screen_size.w - shim::screen_size.w * 0.1f - 3*2/*battle window border aka win_border */ - WIN_BORDER*2;
		item_gui->get_list()->set_width(width);
	}
	else if (easy_gui) {
		easy_gui->set_padding(EDGE_X, 0, 0, EDGE_Y);
		easy_gui->resize(new_size);
		int width = shim::screen_size.w - shim::screen_size.w * 0.1f - 3*2/*battle window border aka win_border */ - WIN_BORDER*2;
		easy_gui->get_list()->set_width(width);
	}
	else if (turn_gui) {
		turn_gui->set_padding(EDGE_X, 0, 0, EDGE_Y);
		turn_gui->resize(new_size);
	}
}

void Battle_Game::add_notification(Notification_Position position, std::string text, int lifetime)
{
	if (sneak_attack) {
		if (position == LEFT) {
			position = RIGHT;
		}
		else {
			position = LEFT;
		}
	}
	Notification n;
	n.position = position;
	n.text = text;
	n.death_time = lifetime + GET_TICKS();
	// Delete any in the same position
	for (auto it = notifications.begin(); it != notifications.end();) {
		if ((*it).position == position) {
			it = notifications.erase(it);
		}
		else {
			it++;
		}
	}
	notifications.push_back(n);
}

bool Battle_Game::is_detecting_combo()
{
	return detecting_combo;
}
	
void Battle_Game::set_player_stats_shown(bool show)
{
	player_stats_shown = show;
}

void Battle_Game::set_enemy_stats_shown(bool show)
{
	enemy_stats_shown = show;
}

void Battle_Game::set_window_shown(bool show)
{
	window_shown = show;
}

void Battle_Game::close_guis()
{
	getting_turn = false;
	if (turn_gui_gone == false) {
		turn_gui->exit();
		turn_gui_gone = true;
	}
	enemy_stats_shown = true;
	player_stats_shown = true;
	if (item_gui) {
		item_gui->exit();
		item_gui = nullptr;
	}
	if (easy_gui) {
		easy_gui->exit();
		easy_gui = nullptr;
	}
	if (mc_gui) {
		mc_gui->exit();
		mc_gui = nullptr;
	}
	end_combo();
	good_combo = -1;
	item_index = -1;
	selecting = false;
	remove_notification(LEFT);
	remove_notification(RIGHT);
	multi_targets.clear();
}

void Battle_Game::remove_notification(Notification_Position pos)
{
	for (auto it = notifications.begin(); it != notifications.end();) {
		Notification &n = *it;
		if (n.position == pos) {
			it = notifications.erase(it);
		}
		else {
			it++;
		}
	}
}

void Battle_Game::do_all_dead()
{
	if (getting_turn) {
		getting_turn = false;
		if (turn_gui_gone == false) {
			turn_gui->exit();
			turn_gui_gone = true;
		}
		enemy_stats_shown = true;
		player_stats_shown = true;
		if (item_gui) {
			item_gui->exit();
			item_gui = nullptr;
		}
		if (easy_gui) {
			easy_gui->exit();
			easy_gui = nullptr;
		}
		if (mc_gui) {
			mc_gui->exit();
			mc_gui = nullptr;
		}
		end_combo();
		good_combo = -1;
		remove_notification(RIGHT);
	}
}

bool Battle_Game::is_sneak_attack()
{
	return sneak_attack;
}

void Battle_Game::startup()
{
	if (sneak_attack && TTH_INSTANCE->saw_sneak == false && easy_combos == false) {
		TTH_INSTANCE->saw_sneak = true;
		GLOBALS->mini_pause(false, true);
	}
}

int Battle_Game::get_active_player()
{
	return active_player;
}

void Battle_Game::add_gold(int gold)
{
	this->gold += gold;
}

void Battle_Game::multi_confirm(bool confirmed)
{
	auto players = get_players();

	fixup_targets(players[active_player], multi_targets, selecting_enemy, false);

	if (multi_targets.size() == 0) {
		confirmed = false;
	}

	if (confirmed) {
		if (turn_type == wedge::Battle_Game::ATTACK) {
			#if 0
			if (INSTANCE->is_milestone_complete(MS_DID_AN_ATTACK) == false) {
				gfx::add_notification(GLOBALS->game_t->translate(409)/* Originally: Good job! */);
				INSTANCE->set_milestone_complete(MS_DID_AN_ATTACK, true);
			}
			#endif

			auto t = new wedge::Battle_Game::Turn;

			t->actor = acting_player;
			t->turn_type = wedge::Battle_Game::ATTACK;
			int player_index = get_player_index(acting_player);
			std::string cmbo = TTH_INSTANCE->combos[player_index][good_combo];
			std::string bak = cmbo;
			int pos;
			if ((pos = cmbo.find('\'')) != std::string::npos) {
				cmbo = cmbo.substr(0, pos) + cmbo.substr(pos+1);
			}
			int cost = get_combo_cost(cmbo);
			INSTANCE->stats[player_index].base.mp -= cost;
			t->turn_name = std::string("attack_") + util::lowercase(cmbo);
			for (size_t i = 0; i < t->turn_name.length(); i++) {
				if (t->turn_name[i] == ' ') {
					t->turn_name[i] = '_';
				}
			}
			add_notification(RIGHT, GLOBALS->game_t->translate(GLOBALS->english_game_t->get_id(bak)));
			good_combo = -1;
			for (auto e : multi_targets) {
				t->targets.push_back(e);
			}
			t->started = false;

			next_turn = t;

			selecting = false;
			
			GLOBALS->button->play(false);
		}
		else if (turn_type == wedge::Battle_Game::ITEM) {
			auto t = new wedge::Battle_Game::Turn;

			t->actor = acting_player;
			t->turn_type = wedge::Battle_Game::ITEM;
			int i = inventory_indices[item_index];
			t->turn_name = util::itos(i);
			wedge::Object *inv = INSTANCE->inventory.get_all();
			inv[i].used++;
			add_notification(RIGHT, inv[i].name);
			for (auto e : multi_targets) {
				t->targets.push_back(e);
			}
			t->started = false;

			next_turn = t;

			selecting = false;
			
			GLOBALS->button->play(false);

			item_index = -1;
		}
	}
	else {
		selecting = false;
		getting_turn = false;
		GLOBALS->button->play(false);
		item_gui = nullptr;
		easy_gui = nullptr;
		if (turn_type == wedge::Battle_Game::ATTACK) {
			next_turn_pos = 0;
		}
		else {
			next_turn_pos = 1;
		}
		item_index = -1;
		good_combo = -1;
		remove_notification(RIGHT);
		just_exited_multi_gui = true;
	}
	
	mc_gui = nullptr;
	
	multi_targets.clear();
}

bool Battle_Game::get_osc_enabled()
{
	return osc_enabled;
}

void Battle_Game::start_turn(wedge::Battle_Entity *entity)
{
	return;

	/*
	if (simple_turn_display) {
		return;
	}

	for (auto it = started_turns.begin(); it != started_turns.end(); it++) {
		auto p = *it;
		if (p.first == entity) {
			started_turns.erase(it);
			break;
		}
	}

	gfx::Sprite *sprite = new gfx::Sprite("turn_arrow");
	sprite->set_animation("only", ::turn_sprite_ended, sprite);
	std::pair<wedge::Battle_Entity *, gfx::Sprite *> p;
	p.first = entity;
	p.second = sprite;
	started_turns.push_back(p);
	*/
}

void Battle_Game::turn_sprite_ended(gfx::Sprite *sprite)
{
	started_turns_to_delete.push_back(sprite);
}
