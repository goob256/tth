//#define DUMP_LEVEL_13

#include <wedge3/achieve.h>
#include <wedge3/area_game.h>
#include <wedge3/a_star.h>
#include <wedge3/change_angle.h>
#include <wedge3/check_positions.h>
#include <wedge3/chest.h>
#include <wedge3/delay.h>
#include <wedge3/delete_map_entity.h>
#include <wedge3/general.h>
#include <wedge3/generic_immediate_callback.h>
#include <wedge3/give_object.h>
#include <wedge3/input.h>
#include <wedge3/map_entity.h>
#include <wedge3/npc.h>
#include <wedge3/offset_arc.h>
#include <wedge3/omnipresent.h>
#include <wedge3/pause_presses.h>
#include <wedge3/play_animation.h>
#include <wedge3/play_music.h>
#include <wedge3/play_sound.h>
#include <wedge3/rumble.h>
#include <wedge3/set_animation.h>
#include <wedge3/set_direction.h>
#include <wedge3/set_integer.h>
#include <wedge3/set_milestone_complete.h>
#include <wedge3/set_solid.h>
#include <wedge3/set_visible.h>
#include <wedge3/shop_step.h>
#include <wedge3/slide_entity.h>
#include <wedge3/slide_entity_offset.h>
#include <wedge3/stop_sound.h>
#include <wedge3/tile_movement.h>
#include <wedge3/wait.h>
#include <wedge3/wait_for_integer.h>

#include "achievements.h"
#include "area_game.h"
#include "battle_transition_in.h"
#include "battle_transition_in2.h"
#include "battle_transition_out.h"
#include "battle_transition_out2.h"
#include "autosave.h"
#include "battles.h"
#include "dialogue.h"
#include "enemies.h"
#include "general.h"
#include "globals.h"
#include "inventory.h"
#include "menu.h"
#include "milestones.h"
#include "pan_camera.h"
#include "start_battle.h"
#include "stats.h"
#include "tth.h"

#include <shim3/internal/audio.h>

class Area_Hooks : public wedge::Area_Hooks
{
public:
	Area_Hooks(wedge::Area *area) :
		wedge::Area_Hooks(area),
		level(-1)
	{
	}
	
	void started()
	{
		maybe_autosave();
	
		wedge::Area_Hooks::started();
	}

	void maybe_autosave()
	{
		if (level == INSTANCE->get_num_levels()-1 || level < 0) {
			return;
		}

		wedge::Area_Hooks::maybe_autosave();

		autosave(true);
	}

	// generate walls, chests, exits
	void generate(bool dry_run = false)
	{
		gfx::Tilemap *tilemap = area->get_tilemap();

		enter_loc = level % 2 == 1 ? 1 : 5;
		exit_loc = level % 2 == 1 ? 5 : 1;

		bool special = level == 12 || level == 51 || level == 403 || level == 776;
		bool last = level == INSTANCE->get_num_levels()-1 || ((level+1) % 10) == 0;

		if (level == INSTANCE->get_num_levels()-1) {
			enter_door = 9;
			enter_loc = 9;
			if (dry_run == false) {
				tilemap->set_tile(0, {enter_door, enter_loc}, {last ? 11 : special ? 10 : 3, 0}, false);
			}
		}
		else if (level > 0) {
			util::srand(level-1);
			enter_door = util::rand(0, 4)+1;
			if (dry_run == false) {
				tilemap->set_tile(0, {enter_door, enter_loc}, {last ? 11 : special ? 10 : 3, 0}, false);
			}
		}
		else {
			enter_door = -1;
			if (dry_run == false) {
				tilemap->set_tile(2, {3, 6}, {-1, -1}, false);
				Fade_Zone z;
				z.zone = util::Rectangle<int>(util::Point<int>(3, 6), util::Size<int>(1, 1));
				z.area_name = "start";
				z.player_positions.push_back(util::Point<int>(8, 5));
				z.directions.push_back(wedge::DIR_S);
				fade_zones.push_back(z);
			}
		}

		util::srand(level);
		
		if (level == INSTANCE->get_num_levels()-1) {
			exit_door = -1;
		}
		else if (level == INSTANCE->get_num_levels()-2) {
			exit_door = 3;
			exit_loc = 3;
			if (dry_run == false) {
				tilemap->set_tile(1, {exit_door, exit_loc}, {2, 0}, false);
			}
		}
		else {
			exit_door = util::rand(0, 4)+1;
			if (dry_run == false) {
				tilemap->set_tile(1, {exit_door, exit_loc}, {2, 0}, false);
			}
		}

		if (level > 0) {
			int ed = enter_door;
			int el = enter_loc;
			// last level and other levels are different sizes, this adjusts
			if (level == INSTANCE->get_num_levels()-1) {
				ed -= 6;
				el -= 6;
			}
			if (dry_run == false) {
				if (GLOBALS->speed_run && level != INSTANCE->get_num_levels()-1) {
					tilemap->set_tile(1, {enter_door, enter_loc}, {-1, -1}, false);
					tilemap->set_tile(0, {enter_door, enter_loc}, {9, 0}, false);
				}
				else {
					Fade_Zone z;
					z.zone = util::Rectangle<int>(util::Point<int>(enter_door, enter_loc), util::Size<int>(1, 1));
					z.area_name = util::itos(level-1);
					z.player_positions.push_back(util::Point<int>(ed, el));
					z.directions.push_back(level % 2 == 1 ? wedge::DIR_S : wedge::DIR_N);
					fade_zones.push_back(z);
				}
			}
		}
	
		int ed = exit_door;
		int el = exit_loc;
		// last level and other levels are different sizes, this adjusts
		if (level == INSTANCE->get_num_levels()-2) {
			ed += 6;
			el += 6;
		}
		if (dry_run == false) {
			Fade_Zone z;
			z.zone = util::Rectangle<int>(util::Point<int>(exit_door, exit_loc), util::Size<int>(1, 1));
			z.area_name = util::itos(level+1);
			z.player_positions.push_back(util::Point<int>(ed, el));
			if (level == INSTANCE->get_num_levels()-2) {
				z.directions.push_back(wedge::DIR_N);
			}
			else {
				z.directions.push_back(level % 2 == 1 ? wedge::DIR_N : wedge::DIR_S);
			}
			fade_zones.push_back(z);
		}

		util::srand(time(0));
	}

	void generate_chests()
	{
		if (level >= INSTANCE->get_num_levels()-2) {
			return;
		}

		if (level == 12) {
			wedge::Chest *chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_WEAPON, WEAPON_SAMURAI_SWORD, 1));
			chest->start(area);
			chest->set_position(util::Point<int>(3, 3));
			chest->set_layer(1);
			area->add_entity(chest);
			return;
		}
		else if (level == 51) {
			wedge::Chest *chest;
			if (GLOBALS->speed_run) {
				chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_DIE, 100));
			}
			else {
				chest = new wedge::Chest("chest", "chest", 100);
			}
			chest->start(area);
			chest->set_position(util::Point<int>(3, 3));
			chest->set_layer(1);
			area->add_entity(chest);
			return;
		}

		int nchests;

		if (level > 0 && ((level+1) % 10) == 0) {
			nchests = util::rand(3, 5);
		}
		else if (level == 0) {
			nchests = util::rand(0, 3);
		}
		else {
			nchests = util::rand(0, 2);
		}

		std::vector< util::Point<int> > added;
		int num_tries = 0;

		while (added.size() < nchests && num_tries < 100) {
			int x, y;
			int loc = util::rand(0, 3);
			if (loc == 0) {
				x = 1;
			}
			else if (loc == 1) {
				y = 1;
			}
			else if (loc == 2) {
				x = 5;
			}
			else {
				y = 5;
			}
			if (loc == 0) {
				y = util::rand(1, 5);
			}
			else if (loc == 1) {
				x = util::rand(1, 5);
			}
			else if (loc == 2) {
				y = util::rand(1, 5);
			}
			else {
				x = util::rand(1, 5);
			}
			bool found = true;
			if ((std::abs(x-enter_door) > 1 || std::abs(y-enter_loc) > 1) && (std::abs(x-exit_door) > 1 || std::abs(y-exit_loc) > 1) && (level != 0 || ((x != 3 || y != 5) && (x != 3 && y != 6))) && (std::find(added.begin(), added.end(), util::Point<int>(x, y)) == added.end())) {
				bool found2 = false;
				for (size_t i = 0; i < added.size(); i++) {
					util::Point<int> &p = added[i];
					if (std::abs(p.x-x) <= 1 && std::abs(p.y-y) <= 1) {
						found2 = true;
						break;
					}
				}
				if (found2) {
					found = false;
				}
				else {
					found = true;
				}
			}
			else {
				found = false;
			}
			if (found == false) {
				num_tries++;
				continue;
			}
			added.push_back(util::Point<int>(x, y));
			int num_doughnuts;
			if (level > 0 && ((level+1) % 10) == 0) {
				num_doughnuts = util::rand(1, 3);
			}
			else {
				num_doughnuts = 1;
			}
			wedge::Chest *chest;
			
			if (GLOBALS->speed_run && util::rand(0, 1) == 0) {
				chest = new wedge::Chest("chest", "chest", OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_DIE, num_doughnuts));
			}
			else {
				chest = new wedge::Chest("chest", "chest", num_doughnuts);
			}
			chest->start(area);
			chest->set_position(util::Point<int>(x, y));
			chest->set_layer(1);
			area->add_entity(chest);
		}
	}

protected:
	int level;
	int enter_door;
	int exit_door;
	int enter_loc;
	int exit_loc;
};

class Area_Hooks_Start : public Area_Hooks
{
public:
	Area_Hooks_Start(wedge::Area *area) :
		Area_Hooks(area)
	{
		Fade_Zone z;
		z.zone = util::Rectangle<int>(util::Point<int>(8, 5), util::Size<int>(1, 1));
		z.area_name = "0";
		z.player_positions.push_back(util::Point<int>(3, 6));
		z.directions.push_back(wedge::DIR_N);
		fade_zones.push_back(z);
	}
	
	virtual ~Area_Hooks_Start()
	{
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		gfx::add_notification(GLOBALS->game_t->translate(586)/* Originally: Long ways up... */);

		if (GLOBALS->prerendered_music) {
			audio::play_music("music/start_prerendered.mml");
		}
		else {
			audio::play_music("music/start.mml");
		}

		if (new_game) {
		}

		return true;
	}
};

static void add_mug(void *data);

class Area_Hooks_Level : public Area_Hooks
{
public:
	Area_Hooks_Level(wedge::Area *area, int level) :
		Area_Hooks(area),
		doughnut(false),
		dice(false)
	{
		this->level = level;
	}

	virtual ~Area_Hooks_Level()
	{
	}

	void gen_stars(bool selective, int w, int h, int num_stars)
	{
		util::srand(selective ? 0 : 13);
		for (int i = 0; i < num_stars; i++) {
			Star s;
			s.pos.x = util::rand(5, w-6);
			s.pos.y = util::rand(5, h-6);
			if (selective && s.pos.x >= 7*shim::tile_size-6 && s.pos.x < 12*shim::tile_size+5 && s.pos.y >= 7 *shim::tile_size-6 && s.pos.y < 12*shim::tile_size+5) {
				continue;
			}
			bool skip = false;
			for (size_t i = 0; i < stars.size(); i++) {
				Star &s2 = stars[i];
				if (std::abs(s.pos.x-s2.pos.x) <= 2 && std::abs(s.pos.y-s2.pos.y) <= 2) {
					skip = true;
					break;
				}
			}
			if (skip) {
				continue;
			}
			s.p = util::rand(0, 1000) / 1000.0f;
			s.inc = util::rand(0, 1) == 0 ? 1 : -1;
			const float min_step = 0.01f;
			const float max_step = 0.02f;
			s.step = min_step + (max_step-min_step)*(util::rand(0, 1000)/1000.0f);
			int r = util::rand(0, 7);
			SDL_Colour c1;
			SDL_Colour c2;
			if (r == 0) {
				c2.r = 45;
				c2.g = 79;
				c2.b = 175;
				c2.a = 255;
				c1.r = c2.r/2;
				c1.g = c2.g/2;
				c1.b = c2.b/2;
				c1.a = 255;
			}
			else if (r == 1) {
				c2.r = 254;
				c2.g = 237;
				c2.b = 186;
				c2.a = 255;
				c1.r = c2.r/2;
				c1.g = c2.g/2;
				c1.b = c2.b/2;
				c1.a = 255;
			}
			else if (r == 2) {
				c2.r = 175;
				c2.g = 45;
				c2.b = 47;
				c2.a = 255;
				c1.r = c2.r/2;
				c1.g = c2.g/2;
				c1.b = c2.b/2;
				c1.a = 255;
			}
			else {
				c1.r = 128;
				c1.g = 128;
				c1.b = 128;
				c1.a = 255;
				c2 = shim::white;
			}
			float p = 0.5f + (util::rand(0, 1000)/1000.0f/2.0f);
			c1.r *= p;
			c1.g *= p;
			c1.b *= p;
			c2.r *= p;
			c2.g *= p;
			c2.b *= p;
			s.c1 = c1;
			s.c2 = c2;
			stars.push_back(s);
		}
		util::srand(time(NULL));
	}

	bool start(bool new_game, bool loaded, std::string save)
	{
		gfx::add_notification(util::string_printf(GLOBALS->game_t->translate(587)/* Originally: Level %d... */.c_str(),  level+1));

		if (level == INSTANCE->get_num_levels()-1) {
			if (GLOBALS->prerendered_music) {
				audio::play_music("music/end_prerendered.mml");
			}
			else {
				audio::play_music("music/end.mml");
			}
		}
		else if ((level+1) % 10 == 0) {
			if (GLOBALS->prerendered_music) {
				audio::play_music("music/bonus_prerendered.mml");
			}
			else {
				audio::play_music("music/bonus.mml");
			}
		}
		else if (level == 12 || level == 51 || level == 403 || level == 776) {
			if (GLOBALS->prerendered_music) {
				audio::play_music("music/special_prerendered.mml");
			}
			else {
				audio::play_music("music/special.mml");
			}
		}
		else {
			if (GLOBALS->prerendered_music) {
				audio::play_music("music/level_prerendered.mml");
			}
			else {
				audio::play_music("music/level.mml");
			}
		}

		generate();

		if (loaded == false) {
			generate_chests();
		}

		if (level == INSTANCE->get_num_levels()-1) {
			gfx::Tilemap *t = area->get_tilemap();
			util::Size<int> sz = t->get_size();
			int w = sz.w * shim::tile_size;
			int h = sz.h * shim::tile_size;
			gen_stars(true, w, h, 500);
		}
#ifdef DUMP_LEVEL_13
		else if (level == 12/* 13 */) {
			gen_stars(false, SCR_W, SCR_H, 175);
		}
#endif

		blanked = false;

		headings.push_back(GLOBALS->game_t->translate(597)/* Originally: GRAPHICS */);
		headings.push_back(GLOBALS->game_t->translate(598)/* Originally: MUSIC */);
		headings.push_back(GLOBALS->game_t->translate(599)/* Originally: CODE */);

		graphics.push_back("VIKTOR YANEV");
		graphics.push_back("GABRIEL G");
		graphics.push_back("GABRIEL CANELLAS");
		
		music.push_back("TRENT GAMBLIN");

		code.push_back("TRENT GAMBLIN");

		credits_count = 0;
		for (size_t i = 0; i < headings.size(); i++) {
			credits_count++;
		}
		for (size_t i = 0; i < graphics.size(); i++) {
			credits_count++;
		}
		for (size_t i = 0; i < music.size(); i++) {
			credits_count++;
		}
		for (size_t i = 0; i < code.size(); i++) {
			credits_count++;
		}

		credits_count++;

		credits_len = credits_count * 2000 + 5000;

		faded = false;

		return true;
	}

	bool has_battles()
	{
		if (level > 0 && ((level+1) % 10) == 0) {
			return false;
		}
		return level != INSTANCE->get_num_levels()-1;
	}

	wedge::Battle_Game *get_random_battle()
	{
		if (has_battles() == false) {
			return nullptr;
		}

		int num_enemies = util::rand(1, 2);
		std::vector<Battle_Enemy *> enemies;

		for (int i = 0; i < num_enemies; i++) {
			int r = util::rand(0, 6);
			if (r == 0) {
				enemies.push_back(new Enemy_Beholder());
			}
			else if (r == 1) {
				enemies.push_back(new Enemy_Werewolf());
			}
			else if (r == 2) {
				enemies.push_back(new Enemy_Ooze());
			}
			else if (r == 3) {
				enemies.push_back(new Enemy_Mermaid());
			}
			else if (r == 4) {
				enemies.push_back(new Enemy_Phoenix());
			}
			else if (r == 5) {
				enemies.push_back(new Enemy_Alraune());
			}
			else {
				enemies.push_back(new Enemy_Skeleton());
			}
		}

		return new Battle_TTH(enemies, level  == 51 || level == 403 || level == 776 ? "black" : "tower");
	}
	
	void pre_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 0) {
#ifdef DUMP_LEVEL_13
			gfx::Image *tex = new gfx::Image(util::Size<int>(SCR_W, SCR_H));
			gfx::set_target_image(tex);
			gfx::clear(shim::black);
#endif
			gfx::draw_primitives_start();
			for (size_t i = 0; i < stars.size(); i++) {
				Star &s = stars[i];
				SDL_Colour c;
				c.r = s.c1.r+(s.c2.r-s.c1.r)*s.p;
				c.g = s.c1.g+(s.c2.g-s.c1.g)*s.p;
				c.b = s.c1.b+(s.c2.b-s.c1.b)*s.p;
				c.a = 255;
#ifdef DUMP_LEVEL_13
				gfx::draw_filled_rectangle(c, s.pos, util::Size<float>(1, 1));
#else
				gfx::draw_filled_rectangle(c, s.pos+map_offset, util::Size<float>(1, 1));
#endif
			}
			gfx::draw_primitives_end();
#ifdef DUMP_LEVEL_13
			unsigned char *buf = gfx::Image::read_texture(tex);
			FILE *f = fopen("level_13.raw", "wb");
			for (int i = 0; i < SCR_W*SCR_H*4; i++) {
				fputc(buf[i], f);
			}
			exit(0);
#endif
			if (level == 12) { // 13
				TTH_GLOBALS->level_13->draw({0, 0});
			}
			else if (level == 51) { // 52
				TTH_GLOBALS->joker->draw({float(shim::screen_size.w/2-TTH_GLOBALS->joker->size.w/2), float(shim::screen_size.h/2-TTH_GLOBALS->joker->size.h/2)});
			}
			else if (level == 776) { // 777
				TTH_GLOBALS->big_heart->draw({float(shim::screen_size.w/2-TTH_GLOBALS->big_heart->size.w/2), float(shim::screen_size.h/2-TTH_GLOBALS->big_heart->size.h/2)});
			}
		}
	}
	
	void post_draw(int layer, util::Point<float> map_offset)
	{
		if (layer == 3) {
			for (size_t i = 0; i < doughnuts.size(); i++) {
				Doughnut &d = doughnuts[i];
				gfx::Image *img = TTH_GLOBALS->doughnut->get_current_image();
				img->draw({d.pos.x+map_offset.x-img->size.w/2, d.pos.y+map_offset.y-img->size.h});
			}


			int x = shim::tile_size+10;
			int y = shim::screen_size.h-shim::tile_size*2-1;

			SDL_Colour col;
			col.r = 253;
			col.g = 77;
			col.b = 79;
			col.a = 255;

			std::string str;
			std::string nd;

			// dice
			
			if (GLOBALS->speed_run == true && level != INSTANCE->get_num_levels()-1) {
			
				y = shim::screen_size.h-shim::tile_size*2-1;

				TTH_GLOBALS->dice[5]->draw({float(shim::screen_size.w-shim::tile_size-6), float(shim::screen_size.h-shim::tile_size*2-1)});
			
				str = "";
				if (input::is_joystick_connected()) {
					str = input::get_joystick_button_name(GLOBALS->joy_die);
				}
#ifdef ANDROID
#else
				else {
					str = get_key_name(GLOBALS->key_die);
				}
#endif

				x = shim::screen_size.w-(shim::tile_size+10)-1;

				nd = util::itos(get_num_dice());
				
				TTH_GLOBALS->numfont->start_batch();
				int totlen = 0;
				for (size_t i = 0; i < nd.length(); i++) {
					char c = nd[i];
					int index = c - '0';
					totlen += index == 1 ? 3 : 4;
				}
				int curlen = 0;
				for (size_t i = 0; i < nd.length(); i++) {
					char c = nd[i];
					int index = c - '0';
					int w = index == 1 ? 3 : 4;
					TTH_GLOBALS->numfont->draw_region_tinted(col, {float(index*4), 0.0f}, {w, 5}, {float(x+curlen-totlen), float(y+TTH_GLOBALS->bold_font->get_height()-7)});
					curlen += w;
				}
				TTH_GLOBALS->numfont->end_batch();

				gfx::Font::end_batches();
			}
		}

		if (blanked) {
			gfx::draw_filled_rectangle(shim::black, {0, 0}, shim::screen_size);

			if (credits_started) {
				Uint32 t = GET_TICKS() - credits_start;

				if (t > 2500 && t < credits_len-2500) {
					t -= 2500;
					int section = t / 2000;
					float p = t % 2000 / 2000.0f;
					std::vector<std::string> all;
					std::vector<bool> heading;
					all.push_back(headings[0]);
					heading.push_back(true);
					for (size_t i = 0; i < graphics.size(); i++) {
						all.push_back(graphics[i]);
						heading.push_back(false);
					}
					all.push_back(headings[1]);
					heading.push_back(true);
					for (size_t i = 0; i < music.size(); i++) {
						all.push_back(music[i]);
						heading.push_back(false);
					}
					all.push_back(headings[2]);
					heading.push_back(true);
					for (size_t i = 0; i < code.size(); i++) {
						all.push_back(code[i]);
						heading.push_back(false);
					}


					if (section < all.size()) {
						int num_items = 0;
						int sec = -1;
						for (int i = section; i >= 0; i--) {
							num_items++;
							if (heading[i]) {
								sec = i;
								break;
							}
						}
						int total_items = num_items;
						for (int i = section+1; i < all.size(); i++) {
							if (heading[i]) {
								break;
							}
							total_items++;
						}
						SDL_Colour c = shim::white;
						if (heading[section]) {
							p /= 1.0f;
							c.r *= p;
							c.g *= p;
							c.b *= p;
						}
						TTH_GLOBALS->bold_font->draw(c, all[sec], {float(shim::screen_size.w/2-TTH_GLOBALS->bold_font->get_text_width(all[sec])/2), float(shim::screen_size.h/2-(TTH_GLOBALS->bold_font->get_height()+(5+(total_items-1)*shim::font->get_height()))/2-5)});
						for (int i = sec+1; i <= section; i++) {
							float x = shim::screen_size.w/2-shim::font->get_text_width(all[i])/2;
							shim::font->draw(shim::white, all[i], {float(x), float(shim::screen_size.h/2-(TTH_GLOBALS->bold_font->get_height()+(5+(total_items-1)*shim::font->get_height()))/2-5+5+(i-(sec+1))*shim::font->get_height()+TTH_GLOBALS->bold_font->get_height())});
						}
					}
					else {
						if (p < 0.5f) {
							p /= 0.5f;
						}
						else {
							p = 1.0f - ((p-0.5f) / 0.5f);
						}
						SDL_Colour c = shim::white;
						c.r *= p;
						c.g *= p;
						c.b *= p;
						TTH_GLOBALS->b1stable_logo->draw_tinted(c, {float(shim::screen_size.w/2-TTH_GLOBALS->b1stable_logo->size.w/2), float(shim::screen_size.h/2-TTH_GLOBALS->b1stable_logo->size.h/2-shim::font->get_height()/2)});
					}
				}
				else if (t >= credits_len-2500) {
					if (faded == false) {
						faded = true;
						OMNIPRESENT->start_fade(shim::black, 0, 100);
					}
				}
			}
		}
	}
	
	std::vector<int> get_pre_draw_layers()
	{
		std::vector<int> v;
		v.push_back(0);
		return v;
	}
	
	std::vector<int> get_post_draw_layers()
	{
		std::vector<int> v;
		v.push_back(3);
		return v;
	}
	
	void run()
	{
		for (size_t i = 0; i < stars.size(); i++) {
			Star &s = stars[i];
			s.p += s.step * s.inc;
			if (s.p < 0.0f) {
				s.p = 0.0f;
				s.inc = -s.inc;
			}
			else if (s.p > 1.0f) {
				s.p = 1.0f;
				s.inc = -s.inc;
			}
			if (util::rand(0, 100) == 0) {
				s.p = util::rand(0, 1000)/1000.0f;
				s.inc = util::rand(0, 1) == 0 ? 1 : -1;
			}
		}

		for (size_t i = 0; i < doughnuts.size(); i++) {
			Doughnut &d = doughnuts[i];
			if (d.ticks <= 0) {
				continue;
			}
			d.pos += d.delta;
			d.delta.y += 0.05f;
			d.ticks--;
		}
	}

	void started()
	{
#if defined STEAMWORKS || defined GOOGLE_PLAY
		if (level == 12) {
			util::achieve((void *)ACH_13);
		}
		else if (level == 51) {
			util::achieve((void *)ACH_52);
		}
		else if (level == 403) {
			util::achieve((void *)ACH_404);
		}
		else if (level == 776) {
			util::achieve((void *)ACH_777);
		}
#endif

		if (GLOBALS->speed_run && level > 0 && level != INSTANCE->get_num_levels()-1) {
			TTH_GLOBALS->block_door_sfx->play(false);
		}

		if (level == INSTANCE->get_num_levels()-1) {
			wedge::Map_Entity *mim = AREA->get_player(0);

			wedge::pause_presses(true);

			NEW_SYSTEM_AND_TASK(AREA)
			ADD_STEP(new wedge::A_Star_Step(mim, util::Point<int>(9, 8), new_task))
			ADD_STEP(new wedge::Delay_Step(2500, new_task))
			ADD_STEP(new wedge::Generic_Immediate_Callback_Step(add_mug, this, new_task))
			ADD_STEP(new wedge::Delay_Step(2500, new_task))
			ADD_TASK(new_task)
			FINISH_SYSTEM(AREA)
		}

		Area_Hooks::started();
	}

	void add_doughnuts()
	{
		TTH_GLOBALS->doughnut->set_animation("static");

		util::srand(19);
		for (int i = 0; i < 7; i++) {
			Doughnut d;
			d.pos = util::Point<float>(9.5f*shim::tile_size, 8.25f*shim::tile_size);
			d.delta.x = (i % 2 == 0 ? 1 : -1) * (0.35f + util::rand(0, 1000)/1000.0f*0.666f);
			d.delta.y = -(0.35f + util::rand(0, 1000)/1000.0f*0.666f);
			d.ticks = util::rand(25, 50);
			doughnuts.push_back(d);
		}
		util::srand(time(NULL));
	}

	void blank()
	{
		if (GLOBALS->prerendered_music) {
			audio::play_music("music/credits_prerendered.mml");
		}
		else {
			audio::play_music("music/credits.mml");
		}
		blanked = true;
		start_credits();
		TTH_GLOBALS->doughnut->set_animation("doughnut");
	}

	void start_credits()
	{
#if defined STEAMWORKS || defined GOOGLE_PLAY
		if (INSTANCE->get_doughnuts_used() == 0) {
			util::achieve((void *)ACH_HARDCORE);
		}
		if (GLOBALS->speed_run) {
			if (INSTANCE->get_num_levels() == 100) {
				util::achieve((void *)ACH_100_SPEED);
			}
			else if (INSTANCE->get_num_levels() == 1000) {
				util::achieve((void *)ACH_1000_SPEED);
			}
		}
		else {
			if (INSTANCE->get_num_levels() == 100) {
				util::achieve((void *)ACH_100);
			}
			else if (INSTANCE->get_num_levels() == 1000) {
				util::achieve((void *)ACH_1000);
			}
		}
#endif
		credits_started = true;
		credits_start = GET_TICKS();
	}

	int get_credits_len()
	{
		return credits_len;
	}

	void handle_event(TGUI_Event *event)
	{
		int skip_to = -1;

		if (level == INSTANCE->get_num_levels()-1) {
			return;
		}
	
		if (AREA->get_player(0)->get_input_step()->get_task()->is_paused()) {
			return;
		}

		if (event->type == TGUI_JOY_DOWN) {
			if (GLOBALS->speed_run == true) {
				if (event->joystick.button == GLOBALS->joy_doughnut) {
					//doughnut = true;
				}
				else if (event->joystick.button == GLOBALS->joy_die) {
					dice = true;
				}
			}
		}
		else if (event->type == TGUI_KEY_DOWN) {
			if (GLOBALS->speed_run == true) {
				if (event->keyboard.code == GLOBALS->key_doughnut) {
					//doughnut = true;
				}
				else if (event->keyboard.code == GLOBALS->key_die) {
					dice = true;
				}
			}
#ifdef DEBUG
			if (event->type == TGUI_KEY_DOWN && event->keyboard.code == TGUIK_F2) {
#else
			if (event->type == TGUI_KEY_DOWN && event->keyboard.code == TGUIK_F2 && shim::debug) {
#endif
				if (level < 12) {
					skip_to = 12;
				}
				else if (level < 51) {
					skip_to = 51;
				}
				else if (INSTANCE->get_num_levels() == 100 && level < 99) {
					skip_to = 99;
				}
				else if (level < 403) {
					skip_to = 403;
				}
				else if (level < 776) {
					skip_to = 776;
				}
				else {
					skip_to = 999;
				}
				dice = true;
			}
#ifdef DEBUG
			if (event->type == TGUI_KEY_DOWN && event->keyboard.code == TGUIK_F3) {
#else
			if (event->type == TGUI_KEY_DOWN && event->keyboard.code == TGUIK_F3 && shim::debug) {
#endif
				skip_to = (level+10) - ((level+1) % 10);
				dice = true;
			}
		}
		else if (event->type == TGUI_MOUSE_DOWN) {
			if (event->mouse.x >= 5 && event->mouse.x < 5+shim::tile_size && event->mouse.y >= shim::screen_size.h-shim::tile_size*2-1 && event->mouse.y < shim::screen_size.h-shim::tile_size-1) {
				//doughnut = true;
			}
			else if (event->mouse.x >= shim::screen_size.w-shim::tile_size-5 && event->mouse.x < shim::screen_size.w-5 && event->mouse.y >= shim::screen_size.h-shim::tile_size*2-1 && event->mouse.y < shim::screen_size.h-shim::tile_size-1) {
				dice = true;
			}
		}

		// FIXME:
		doughnut = false;

		auto mim = AREA->get_player(0);
		if (mim->is_moving() == false) {
			if (doughnut == true) {
				doughnut = false;

				TTH_GLOBALS->doughnut_sfx->play(false);
				INSTANCE->add_gold(-1);
				INSTANCE->stats[0].base.hp = MIN(INSTANCE->stats[0].base.fixed.max_hp, INSTANCE->stats[0].base.hp+DOUGHNUT_HP);

				NEW_SYSTEM_AND_TASK(AREA)
				ADD_STEP(new wedge::Play_Animation_Step(mim->get_sprite(), "jump", new_task))
				ADD_STEP(new wedge::Set_Animation_Step(mim, "stand_s", new_task))
				ADD_TASK(new_task)
				FINISH_SYSTEM(AREA)
			}
			if (dice == true) {
				dice = false;
				if (get_num_dice() <= 0 && skip_to < 0) {
					TTH_GLOBALS->error_sfx->play(false);
				}
				else {
					TTH_GLOBALS->die_sfx->play(false);
					
					int index = INSTANCE->inventory.find(OBJECT->make_object(wedge::OBJECT_ITEM, ITEM_DIE, 1));
					
					if (index >= 0 || skip_to >= 0) {
						if (skip_to < 0) {
							INSTANCE->inventory.remove(index, 1);
						}

						static int last_three[3] = { 0, 0, 0 };
						int skips = util::rand(1, 6);

						last_three[2] = last_three[1];
						last_three[1] = last_three[0];
						last_three[0] = skips;

						if (skip_to >= 0) {
							skips = skip_to - level;
						}
						else if (last_three[0] == 6 && last_three[1] == 6 && last_three[2] == 6) {
							skips = (INSTANCE->get_num_levels()-1) - level;
							util::achieve((void *)ACH_666);
						}
						else if (level + skips > INSTANCE->get_num_levels()-1) {
							skips = (INSTANCE->get_num_levels()-1) - level;
						}
			
						gfx::add_notification(util::string_printf(GLOBALS->game_t->translate(609)/* Originally: Skipped %d Levels! */.c_str(), skips));

						int en_d = enter_door;
						int en_l = enter_loc;
						int ex_d = exit_door;
						int ex_l = exit_loc;
						int l = level;

						level = level + skips;

						generate(true);
						
						std::string level_name = util::itos(level);
						std::vector< util::Point<int> > pos;
						pos.push_back({enter_door, enter_loc});
						std::vector< wedge::Direction > dir;
						dir.push_back(enter_loc == 1 ? wedge::DIR_S : wedge::DIR_N);

						level = l;
						enter_door = en_d;
						enter_loc = en_l;
						exit_door = ex_d;
						exit_loc = ex_l;

						// Redrawing here avoids a little jerk as you step on a tile leading to another area
						if (AREA->get_pause_entity_movement_on_next_area_change()) {
							wedge::Map_Entity *player0 = AREA->get_player(0);
							wedge::Map_Entity_List &entities = area->get_entities();
							for (wedge::Map_Entity_List::iterator it = entities.begin(); it != entities.end(); it++) {
								wedge::Map_Entity *e = *it;
								if (e == player0) {
									continue;
								}
								wedge::Map_Entity_Input_Step *meis = e->get_input_step();
								if (meis) {
									wedge::Tile_Movement_Step *tms = meis->get_movement_step();
									if (tms) {
										tms->die();
									}
									meis->die();
									e->set_input_step(nullptr); // So die() isn't called again in destructor
								}
							}
							area->set_entities_standing();
						}
						shim::user_render();

						area->set_next_area(level_name, pos, dir);
					}
				}
			}
		}
	}

protected:
	struct Star {
		util::Point<int> pos;
		float p;
		int inc;
		float step;
		SDL_Colour c1;
		SDL_Colour c2;
	};

	std::vector<Star> stars;

	struct Doughnut {
		util::Point<float> pos;
		util::Point<float> delta;
		int ticks;
	};

	std::vector<Doughnut> doughnuts;

	bool blanked;

	bool credits_started;
	Uint32 credits_start;
	int credits_count;
	int credits_len;
				
	std::vector<std::string> headings;
	std::vector<std::string> graphics;
	std::vector<std::string> music;
	std::vector<std::string> code;

	bool faded;

	bool doughnut;
	bool dice;
};

static void add_doughnuts(void *data)
{
	Area_Hooks_Level *hooks = (Area_Hooks_Level *)data;
	hooks->add_doughnuts();
}

static void exit_to_menu(void *data)
{
	AREA->set_gameover(true);
	AREA->set_gameover_time(0);
}

static void start_fade(void *data)
{
	OMNIPRESENT->start_fade(shim::black, 0, 2500);
}

static void end_fade(void *data)
{
	Area_Hooks_Level *hooks = (Area_Hooks_Level *)data;
	hooks->blank();
	OMNIPRESENT->end_fade();
}

static void add_mug(void *data)
{
	Area_Hooks_Level *hooks = (Area_Hooks_Level *)data;
	wedge::Area *area = AREA->get_current_area();

	auto mim = area->find_entity("Mim");
			
	wedge::Map_Entity *mug = new wedge::Map_Entity("mug");
	mug->start(area);
	mug->set_position(util::Point<int>(9, 0));
	mug->set_sprite(new gfx::Sprite("mug"));
	area->add_entity(mug);
	mug->get_sprite()->set_animation("rotate");
		
	NEW_SYSTEM_AND_TASK(AREA)
	ADD_STEP(new wedge::Play_Sound_Step(TTH_GLOBALS->mug_sfx, false, false, new_task))
	ADD_STEP(new wedge::Slide_Entity_Step(mug, {9, 7}, 0.025f*shim::tile_size/*speed is in pixels per tick*/, new_task))
	ADD_STEP(new wedge::Set_Animation_Step(mug, "right", new_task))
	ADD_STEP(new wedge::Play_Sound_Step(TTH_GLOBALS->doughnut_jump_sfx, false, false, new_task))
	ADD_STEP(new wedge::Generic_Immediate_Callback_Step(add_doughnuts, data, new_task))
	ADD_STEP(new wedge::Delay_Step(500, new_task))
	ADD_STEP(new wedge::Play_Animation_Step(mim->get_sprite(), "jump_n", new_task))
	ADD_STEP(new wedge::Delay_Step(50, new_task))
	ADD_STEP(new wedge::Set_Animation_Step(mim, "stand_s", new_task))
	ADD_STEP(new wedge::Delay_Step(500, new_task))
	ADD_STEP(new wedge::Play_Sound_Step(TTH_GLOBALS->jump_sfx, false, false, new_task))
	ADD_STEP(new wedge::Play_Animation_Step(mim->get_sprite(), "jump", new_task))
	ADD_STEP(new wedge::Play_Sound_Step(TTH_GLOBALS->jump_sfx, false, false, new_task))
	ADD_STEP(new wedge::Play_Animation_Step(mim->get_sprite(), "jump", new_task))
	if (INSTANCE->get_doughnuts_used() == 0) {
		ADD_STEP(new wedge::Play_Sound_Step(TTH_GLOBALS->big_jump_sfx, false, false, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(mim->get_sprite(), "jump_part", new_task))
		ADD_STEP(new wedge::Set_Animation_Step(mim, "spin", new_task))
		ADD_STEP(new wedge::Slide_Entity_Step(mim, {9, -2}, 1.0f*shim::tile_size/*speed is in pixels per tick*/, new_task))
	}
	else {
		ADD_STEP(new wedge::Play_Sound_Step(TTH_GLOBALS->jump_sfx, false, false, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(mim->get_sprite(), "jump", new_task))
		ADD_STEP(new wedge::Play_Sound_Step(TTH_GLOBALS->wow, false, false, new_task))
		ADD_STEP(new wedge::Play_Animation_Step(mim->get_sprite(), "wow", new_task))
		if (INSTANCE->get_num_levels() == 1000) {
			ADD_STEP(new wedge::Set_Animation_Step(mim, "stand_s", new_task))
			ADD_STEP(new wedge::Delay_Step(50, new_task))
			ADD_STEP(new wedge::Play_Sound_Step(TTH_GLOBALS->a_thousand, false, false, new_task))
			ADD_STEP(new wedge::Play_Animation_Step(mim->get_sprite(), "wow", new_task))
			ADD_STEP(new wedge::Play_Animation_Step(mim->get_sprite(), "wow", new_task))
		}
		ADD_STEP(new wedge::Set_Animation_Step(mim, "stand_s", new_task))
		ADD_STEP(new wedge::Delay_Step(10000, new_task))
	}
	ADD_STEP(new wedge::Generic_Immediate_Callback_Step(start_fade, nullptr, new_task))
	ADD_STEP(new wedge::Delay_Step(2500, new_task))
	ADD_STEP(new wedge::Generic_Immediate_Callback_Step(end_fade, data, new_task))
	ADD_STEP(new wedge::Delay_Step(hooks->get_credits_len(), new_task))
	ADD_STEP(new wedge::Generic_Immediate_Callback_Step(exit_to_menu, nullptr, new_task))
	ADD_TASK(new_task)
	FINISH_SYSTEM(AREA)
}

//--

Area_Game::Area_Game() :
	area_create_count(0),
	use_camera(false)
{
	fadeout_colour = shim::black;
}

Area_Game::~Area_Game()
{
	delete plasma;
}

wedge::Area_Hooks *get_area_hooks(std::string area_name, wedge::Area *area)
{
	wedge::Area_Hooks *hooks = NULL;

	if (area_name == "start") {
		hooks = new Area_Hooks_Start(area);
	}
	else {
		int n = atoi(area_name.c_str());
		hooks = new Area_Hooks_Level(area, n);
	}

	return hooks;
}

wedge::Area_Hooks *Area_Game::get_area_hooks(std::string area_name, wedge::Area *area)
{
	return ::get_area_hooks(area_name, area);
}

void Area_Game::draw()
{
	if (scrolling_in) {
		for (int i = 0; i < 2; i++) {
			wedge::Area::Layer_Spec spec;

			if (i == 0) {
				spec = wedge::Area::BELOW;
			}
			else {
				spec = wedge::Area::ABOVE;
			}

			util::Size<int> tilemap_size = current_area->get_tilemap()->get_size() * shim::tile_size;
			util::Point<float> maximum(shim::screen_size.w, shim::screen_size.h);
			maximum.x = MIN(maximum.x, tilemap_size.w);
			maximum.y = MIN(maximum.y, tilemap_size.h);
			util::Point<float> scrolled = scroll_offset * maximum;
			util::Point<int> player_pos = players[0]->get_position();
			util::Point<float> player_offset = util::Point<float>(0.0f, 0.0f);
			util::Size<int> player_size = players[0]->get_size();
			util::Point<float> sz(player_size.w / 2.0f, 1.0f - player_size.h / 2.0f);
			wedge::add_tiles(player_pos, player_offset, sz);
			//util::Point<float> curr_offset = current_area->get_centred_offset(players[0]->get_position(), util::Point<float>(0.0f, 0.0f), true);
			util::Point<float> curr_offset = current_area->get_centred_offset(player_pos, player_offset, true);
			curr_offset -= scrolled;
			current_area->draw(curr_offset, spec);
			util::Point<int> next_pos = next_area_positions[0];
			util::Point<float> next_o(0.0f, 0.0f);
			wedge::add_tiles(next_pos, next_o, sz);
			//util::Point<float> next_offset = next_area->get_centred_offset(next_area_positions[0], util::Point<float>(0.0f, 0.0f), true);
			util::Point<float> next_offset = next_area->get_centred_offset(next_pos, next_o, true);
			util::Point<float> no;
			if (scroll_increment.x < 0 || scroll_increment.y < 0) {
				util::Point<float> o = curr_offset;
				if (tilemap_size.w < shim::screen_size.w) {
					o.x -= (shim::screen_size.w-tilemap_size.w)/2.0f;
				}
				if (tilemap_size.h < shim::screen_size.h) {
					o.y -= (shim::screen_size.h-tilemap_size.h)/2.0f;
				}
				if (scroll_increment.x == 0.0f) {
					maximum.x = o.x;
				}
				else {
					maximum.y = o.y;
				}
				no = next_offset - (maximum - o);
				next_area->draw(next_offset - (maximum - o), spec);
			}
			else {
				if (scroll_increment.x == 0.0f) {
					maximum.x = scrolled.x;
				}
				else {
					maximum.y = scrolled.y;
				}
				no = next_offset + (maximum - scrolled);
				next_area->draw(next_offset + (maximum - scrolled), spec);
			}
			if (i == 0) {
				std::vector<wedge::Map_Entity *> entities = players;
				std::sort(entities.begin(), entities.end(), wedge::entity_y_compare);
				for (size_t i = 0; i < entities.size(); i++) {
					wedge::Map_Entity *p = entities[i];
					p->draw(curr_offset + (scroll_offset * shim::tile_size));
				}
			}
		}
	}
	else {
		Uint32 now = GET_TICKS();

		/*
		if (pausing) {
			gfx::set_target_image(GLOBALS->work_image);
			gfx::clear(shim::black);
			gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
			gfx::update_projection();
		}
		//else if (int(now - pause_end_time) <= pause_fade_time) {
		else if (pause_ended) {
			gfx::set_target_image(GLOBALS->work_image);
			gfx::clear(shim::black);
			gfx::set_default_projection(shim::real_screen_size, shim::screen_offset, shim::scale);
			gfx::update_projection();
		}
		*/
		
		if (use_camera) {
			current_area->draw(camera);
		}
		else {
			current_area->draw();
		}

#if 0
		
		if (pausing) {
			Uint32 diff = now - pause_start_time;
			if ((int)diff > pause_fade_time) {
				diff = pause_fade_time;
			}
			float p = diff / (float)pause_fade_time;

			gfx::set_target_backbuffer();
			if (pause_is_for_menu == false) {
				gfx::clear(shim::black);
				gfx::draw_filled_rectangle(shim::palette[22], {0, 0}, shim::screen_size);
			}
			else {
				//gfx::clear(shim::transparent);
				//gfx::draw_filled_rectangle(shim::black, {0, 0}, shim::screen_size);
				gfx::clear(shim::black);
			}
			/*
			gfx::enable_depth_write(true);
			gfx::clear_depth_buffer(1.0f);
			gfx::enable_depth_write(false);
			*/
			glm::mat4 mv_save, proj_save, mv, proj;
			gfx::get_matrices(mv_save, proj_save);
			float x = 1.0f;
			float y = 1.0f;
			proj = glm::scale(proj, glm::vec3(10.0f, 10.0f, 1.0f));
			proj = glm::translate(proj, glm::vec3(0.0f, 0.0f, -10.0f));
			proj = glm::frustum(-x, x, y, -y, 1.0f, 1000.0f) * proj;
			proj = glm::translate(proj, glm::vec3(0.0f, y, 0.0f));
			proj = glm::rotate(proj, float(M_PI/2.0f*p), glm::vec3(1.0f, 0.0f, 0.0f));
			proj = glm::translate(proj, glm::vec3(0.0f, -y, 0.0f));
			gfx::set_matrices(mv, proj);
			gfx::update_projection();
			//gfx::set_cull_mode(gfx::NO_FACE);
			SDL_Colour tint = shim::white;
			float f = 1.0f - p;
			tint.r *= f;
			tint.g *= f;
			tint.b *= f;
			tint.a *= f;
			GLOBALS->work_image->stretch_region_tinted(tint, util::Point<float>(0.0f, 0.0f), shim::real_screen_size, util::Point<float>(-x, -y), util::Size<int>(x*2, y*2));
			gfx::set_matrices(mv_save, proj_save);
			gfx::update_projection();
			//gfx::set_cull_mode(gfx::BACK_FACE);
		}
		//else if (int(now - pause_end_time) <= pause_fade_time) {
		else if (pause_ended) {
			gfx::set_target_backbuffer();
		}
#endif

		Game::draw();
	
		if (gameover) {
			Uint32 now = GET_TICKS();
			Uint32 end = gameover_time + GLOBALS->gameover_timeout;
			Uint32 diff;
			if (now > end) {
				diff = 0;
			}
			else {
				diff = end-now;
			}
			if ((int)diff <= GLOBALS->gameover_fade_time) {
				float p = 1.0f - ((float)diff / GLOBALS->gameover_fade_time);
				p = p * p;
				SDL_Colour colour;
				if (gameover_time == 0) { // hack for credits
					colour = shim::palette[38];
				}
				else {
					colour.r = GLOBALS->gameover_fade_colour.r * p;
					colour.g = GLOBALS->gameover_fade_colour.g * p;
					colour.b = GLOBALS->gameover_fade_colour.b * p;
					colour.a = GLOBALS->gameover_fade_colour.a * p;
				}
				gfx::draw_filled_rectangle(colour, util::Point<int>(0, 0), shim::screen_size);
			}
		}
	}
	
	if (fading_in) {
		Uint32 now = GET_TICKS();
		Uint32 elapsed = now - fade_start_time;
		if ((int)elapsed > change_area_fade_duration) {
			elapsed = change_area_fade_duration;
		}

		float p;
		if ((int)elapsed >= change_area_fade_duration/2) {
			elapsed -= change_area_fade_duration/2;
			p = 1.0f - (elapsed / (change_area_fade_duration/2.0f));
		}
		else {
			p = elapsed / (change_area_fade_duration/2.0f);
		}
		p = MAX(0.0f, MIN(1.0f, p));
		SDL_Colour black;
		black.r = fadeout_colour.r * p;
		black.g = fadeout_colour.g * p;
		black.b = fadeout_colour.b * p;
		black.a = p * 255;
		gfx::draw_filled_rectangle(black, util::Point<int>(0, 0), shim::screen_size);
	}
}

void Area_Game::set_next_fadeout_colour(SDL_Colour colour)
{
	fadeout_colour = colour;
}

std::string get_friendly_name(std::string area_name)
{
	if (area_name == "start") {
		return GLOBALS->game_t->translate(53)/* Originally: Tower Base */;
	}
	else {
		return util::string_printf(GLOBALS->game_t->translate(590)/* Originally: Level %d */.c_str(), atoi(area_name.c_str())+1);
	}
}

int Area_Game::get_num_areas_created()
{
	return area_create_count;
}

wedge::Area *Area_Game::create_area(std::string name)
{
	area_create_count++;
	return new wedge::Area(name);
}

wedge::Area *Area_Game::create_area(util::JSON::Node *json)
{
	area_create_count++;
	return new wedge::Area(json);
}

void Area_Game::set_use_camera(bool use_camera)
{
	this->use_camera = use_camera;
}

void Area_Game::set_camera(util::Point<float> camera)
{
	this->camera = camera;
}

wedge::Game *Area_Game::create_menu()
{
	return new Menu_Game();
}

wedge::Game *Area_Game::create_shop(std::vector<wedge::Object> items)
{
	return nullptr;
}

bool Area_Game::run()
{
	int max = 0;
	for (auto &s : INSTANCE->stats) {
		if (s.base.fixed.max_mp > max) {
			max = s.base.fixed.max_mp;
		}
	}
	// Fill all SP after 2 minutes
	int inc = 1000000 / 120 / 60 * max;
	TTH_INSTANCE->sp_replenish_count += inc;

	return wedge::Area_Game::run();
}

void Area_Game::battle_ended(wedge::Battle_Game *battle)
{
	wedge::Area_Game::battle_ended(battle);

	/*
	if (dynamic_cast<Battle_BigTuna *>(battle) != NULL) {
	}

	auto cave2_4_hooks = dynamic_cast<Area_Hooks_Cave2_4 *>(current_area->get_hooks());
	if (cave2_4_hooks) {
		cave2_4_hooks->restart_rock_sfx();
	}
	*/
	
	autosave(true);
}

void Area_Game::handle_event(TGUI_Event *event)
{
	wedge::Area_Game::handle_event(event);

#ifdef DEBUG
	if (event->type == TGUI_KEY_DOWN && event->keyboard.code == TGUIK_b) {
#else
	if (event->type == TGUI_KEY_DOWN && event->keyboard.code == TGUIK_b && shim::debug) {
#endif
		int num_enemies = util::rand(1, 2);
		std::vector<Battle_Enemy *> enemies;

		for (int i = 0; i < num_enemies; i++) {
			int r = util::rand(0, 6);
			if (r == 0) {
				enemies.push_back(new Enemy_Beholder());
			}
			else if (r == 1) {
				enemies.push_back(new Enemy_Werewolf());
			}
			else if (r == 2) {
				enemies.push_back(new Enemy_Ooze());
			}
			else if (r == 3) {
				enemies.push_back(new Enemy_Mermaid());
			}
			else if (r == 4) {
				enemies.push_back(new Enemy_Phoenix());
			}
			else if (r == 5) {
				enemies.push_back(new Enemy_Alraune());
			}
			else {
				enemies.push_back(new Enemy_Skeleton());
			}
		}

		wedge::Battle_Game *battle_game = new Battle_TTH(enemies, "tower");

		battle_game->start_transition_in();
	}
}

