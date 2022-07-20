#include <wedge3/area_game.h>
#include <wedge3/battle_game.h>
#include <wedge3/general.h>

#include "battle_game.h"
#include "dialogue.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "gui_drawing_hook.h"
#include "inventory.h"
#include "menu.h"
#include "milestones.h"
#include "stats.h"

util::JSON *cfg;

void Globals::gen_level_13()
{
	int l13_sz;
	SDL_RWops *l13 = util::open_file("gfx/images/misc/level_13.raw", &l13_sz);
	unsigned char *pixels = new unsigned char[l13_sz];
	for (int i = 0; i < l13_sz; i++) {
		pixels[i] = util::SDL_fgetc(l13);
	}
	util::close_file(l13);
	level_13 = new gfx::Image(util::Size<int>(SCR_W, SCR_H), pixels);
	delete[] pixels;
}

Globals::Globals() :
	wedge::Globals()
{
	gen_level_13();

	loaded_autosave = false;

	player_start_positions.push_back(util::Point<int>(6, 12));
	player_start_directions.push_back(wedge::DIR_N);
	player_sprite_names.push_back("mim");

	spell_interface = NULL; // FIXME, you can implement this
	object_interface = new Object_Interface();

	bold_font = new gfx::Pixel_Font("bold");
	bold_font->set_vertex_cache_id(2);
	bold_font->start_batch();

	up_arrow = new gfx::Image("ui/up_arrow.tga");
	down_arrow = new gfx::Image("ui/down_arrow.tga");
	gui_window = new gfx::Image("ui/gui_window.tga");
	menu_window = new gfx::Image("ui/menu_window.tga");
	battle_window = new gfx::Image("ui/battle_window.tga");
	b1stable_logo = new gfx::Image("misc/b1stable.tga");
	doughnut_img = new gfx::Image("misc/doughnut.tga");
	heart_img = new gfx::Image("misc/heart.tga");
	for (int i = 0; i < 6; i++) {
		dice[i] = new gfx::Image("misc/" + util::itos(i+1) + ".tga");
	}
	numfont = new gfx::Image("misc/numfont.tga");
	big_heart = new gfx::Image("misc/big_heart.tga");
	joker = new gfx::Image("misc/joker.tga");

	xb_a = new gfx::Image("buttons/xb_a.tga");
	xb_b = new gfx::Image("buttons/xb_b.tga");
	xb_x = new gfx::Image("buttons/xb_x.tga");
	xb_y = new gfx::Image("buttons/xb_y.tga");
	ps_x = new gfx::Image("buttons/ps_x.tga");
	ps_circle = new gfx::Image("buttons/ps_circle.tga");
	ps_square = new gfx::Image("buttons/ps_square.tga");
	ps_triangle = new gfx::Image("buttons/ps_triangle.tga");

	shim::font->set_extra_glyph_offset(util::Point<float>(0, 3));
	bold_font->set_extra_glyph_offset(util::Point<float>(0, 3));

	shim::font->add_extra_glyph(0xf0, xb_a);
	shim::font->add_extra_glyph(0xf1, xb_b);
	shim::font->add_extra_glyph(0xf2, xb_x);
	shim::font->add_extra_glyph(0xf3, xb_y);
	shim::font->add_extra_glyph(0xf4, ps_x);
	shim::font->add_extra_glyph(0xf5, ps_circle);
	shim::font->add_extra_glyph(0xf6, ps_square);
	shim::font->add_extra_glyph(0xf7, ps_triangle);
	bold_font->add_extra_glyph(0xf0, xb_a);
	bold_font->add_extra_glyph(0xf1, xb_b);
	bold_font->add_extra_glyph(0xf2, xb_x);
	bold_font->add_extra_glyph(0xf3, xb_y);
	bold_font->add_extra_glyph(0xf4, ps_x);
	bold_font->add_extra_glyph(0xf5, ps_circle);
	bold_font->add_extra_glyph(0xf6, ps_square);
	bold_font->add_extra_glyph(0xf7, ps_triangle);

	shadow = new gfx::Image("misc/shadow.tga");
	player_shadow = new gfx::Image("misc/player_shadow.tga");

	heart = new gfx::Sprite("heart");
	heart->set_animation("heart");
	doughnut = new gfx::Sprite("doughnut");
	doughnut->set_animation("doughnut");
	mug = new gfx::Sprite("mug");

	doughnut_sfx = new audio::Sample("doughnut.flac");
	error_sfx = new audio::Sample("error.flac");
	battle_start = new audio::Sample("battle_start.flac");
	hit_sfx = new audio::Sample("hit.flac");
	mug_sfx = new audio::Sample("mug.flac");
	jump_sfx = new audio::Sample("jump.flac");
	big_jump_sfx = new audio::Sample("big_jump.flac");
	doughnut_jump_sfx = new audio::Sample("doughnut_jump.flac");
	wow = new audio::Sample("wow.flac");
	a_thousand = new audio::Sample("a_thousand.flac");
	die_sfx = new audio::Sample("die.flac");
	block_door_sfx = new audio::Sample("block_door.flac");
	
	util::JSON::Node *root = shim::shim_json->get_root();
	
	max_battle_steps = root->get_nested_int("wedge>globals>max_battle_steps", &max_battle_steps, 21);
	min_battle_steps = root->get_nested_int("wedge>globals>min_battle_steps", &min_battle_steps, 7);
}

Globals::~Globals()
{
	delete bold_font;

	delete up_arrow;
	delete down_arrow;
	delete gui_window;
	delete menu_window;
	delete battle_window;
	
	delete xb_a;
	delete xb_b;
	delete xb_x;
	delete xb_y;
	delete ps_x;
	delete ps_circle;
	delete ps_square;
	delete ps_triangle;
	delete b1stable_logo;
	delete doughnut_img;
	delete heart_img;
	for (int i = 0; i < 6; i++) {
		delete dice[i];
	}
	delete numfont;
	delete level_13;
	delete big_heart;
	delete joker;
	delete shadow;
	delete player_shadow;
	delete heart;
	delete doughnut;
	delete mug;

	delete doughnut_sfx;
	delete error_sfx;
	delete battle_start;
	delete hit_sfx;
	delete mug_sfx;
	delete jump_sfx;
	delete big_jump_sfx;
	delete doughnut_jump_sfx;
	delete wow;
	delete a_thousand;
	delete die_sfx;
	delete block_door_sfx;

	shim::shim_json->remove("max_battle_steps", true);
	shim::shim_json->remove("min_battle_steps", true);

	shim::shim_json->remove("red_triangle_colour.r", true);
	shim::shim_json->remove("red_triangle_colour.g", true);
	shim::shim_json->remove("red_triangle_colour.b", true);
	shim::shim_json->remove("red_triangle_colour.a", true);
		
	shim::shim_json->remove("gameover_fade_colour.r", true);
	shim::shim_json->remove("gameover_fade_colour.g", true);
	shim::shim_json->remove("gameover_fade_colour.b", true);
	shim::shim_json->remove("gameover_fade_colour.a", true);
}

void Globals::do_dialogue(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position, wedge::Step *monitor)
{
	wedge::Game *g;
	if (BATTLE) {
		g = BATTLE;
	}
	else {
		g = AREA;
	}
	NEW_SYSTEM_AND_TASK(g)
	Dialogue_Step *d = new Dialogue_Step(tag, text, type, position, new_task);
	if (monitor) {
		d->add_monitor(monitor);
	}
	for (size_t i = 0; i < next_dialogue_monitors.size(); i++) {
		d->add_monitor(next_dialogue_monitors[i]);
	}
	next_dialogue_monitors.clear();
	ADD_STEP(d)
	ADD_TASK(new_task)
	FINISH_SYSTEM(g)
}

bool Globals::add_title_gui(bool transition)
{
	save_slot = -1;

	Title_GUI *title = new Title_GUI(transition);
	shim::guis.push_back(title);

	return true;
}

bool Globals::dialogue_active(wedge::Game *game, bool only_if_initialised, bool wait_for_fade)
{
	std::vector<Dialogue_Step *> v = active_dialogues(game);

	for (size_t i = 0; i < v.size(); i++) {
		if ((only_if_initialised == false || v[i]->is_initialised()) && (wait_for_fade == true || v[i]->is_done() == false)) {
			return true;
		}
	}

	return false;
}

void Globals::add_notification_gui(std::string text, util::Callback callback, void *callback_data)
{
	Notification_GUI *gui = new Notification_GUI(text, callback, callback_data);
	gui->hook_omnipresent(true, true);
	shim::guis.push_back(gui);
}

void Globals::add_yes_no_gui(std::string text, bool escape_cancels, bool selected, util::Callback callback, void *callback_data)
{
	Yes_No_GUI *gui = new Yes_No_GUI(text, escape_cancels, callback, callback_data, true);
	gui->set_selected(selected);
	gui->hook_omnipresent(true, true);
	shim::guis.push_back(gui);
}

bool Globals::can_walk()
{
	return can_show_settings(false, true, true, true) && shim::guis.size() == 0 && (BATTLE == NULL || dynamic_cast<Battle_Game *>(BATTLE)->is_detecting_combo() == true);
	/*
	if (BATTLE != NULL && dynamic_cast<Battle_Game *>(BATTLE)->is_detecting_combo()) {
		return true;
	}
	return can_show_settings(false, true, true, true) && shim::guis.size() == 0;
	*/
}

bool Globals::title_gui_is_top()
{
	// FIXME
	return false;
}

util::Point<float> Globals::get_onscreen_button_position(wedge::Onscreen_Button button)
{
	dpad->set_animation("dpad");
	util::Size<int> dpad_size = dpad->get_current_image()->size / 3;
	dpad->set_animation("button1");
	util::Size<int> button_size = dpad->get_current_image()->size;
	int offset = shim::tile_size/2;
	float b1_x = shim::screen_size.w-offset-button_size.w*2;

	switch (button) {
		case wedge::ONSCREEN_UP:
			return util::Point<float>(offset+dpad_size.w, shim::screen_size.h-offset-dpad_size.h*3);
		case wedge::ONSCREEN_RIGHT:
			return util::Point<float>(offset+dpad_size.w*2, shim::screen_size.h-offset-dpad_size.h*2);
		case wedge::ONSCREEN_DOWN:
			return util::Point<float>(offset+dpad_size.w, shim::screen_size.h-offset-dpad_size.h);
		case wedge::ONSCREEN_LEFT:
			return util::Point<float>(offset, shim::screen_size.h-offset-dpad_size.h*2);
		case wedge::ONSCREEN_B1:
			return util::Point<float>(b1_x, shim::screen_size.h-offset-button_size.h);
		case wedge::ONSCREEN_B2:
			return util::Point<float>(b1_x + button_size.w, shim::screen_size.h-offset-button_size.h*2);
		default:
			return util::Point<float>(0, 0);
	}
}

void Globals::run()
{
	while (instance != nullptr && TTH_INSTANCE->sp_replenish_count > 1000000) {
		TTH_INSTANCE->sp_replenish_count -= 1000000;
		if (MENU == nullptr) {
			for (auto &s : INSTANCE->stats) {
				if (s.base.hp > 0) {
					s.base.mp = MIN(s.base.fixed.max_mp, s.base.mp+1);
				}
			}
		}
	}
}

void Globals::lost_device()
{
	wedge::Globals::lost_device();
	delete level_13;
	level_13 = nullptr;
}

void Globals::found_device()
{
	wedge::Globals::found_device();

	gen_level_13();
}

void Globals::get_joy_xy(TGUI_Event *event, float joy_axis0, float joy_axis1, int *x, int *y)
{
	if (wedge::is_onscreen_controller_enabled()) {
		if (event->joystick.axis == 0) {
			float curr_y = 0.0f;
			*y = fabsf(event->joystick.value) >= shim::joystick_deactivate_threshold ? 0 : joy_axis1;
			*x = fabsf(curr_y) >= shim::joystick_deactivate_threshold ? 0 : wedge::JOYF_TO_I(event->joystick.value, joy_axis0);
		}
		else {
			float curr_x = 0.0f;
			*x = fabsf(event->joystick.value) >= shim::joystick_deactivate_threshold ? 0 : joy_axis0;
			*y = fabsf(curr_x) >= shim::joystick_deactivate_threshold ? 0 : wedge::JOYF_TO_I(event->joystick.value, joy_axis1);
		}
	}
	else {
		SDL_Joystick *joy = SDL_JoystickFromInstanceID(event->joystick.id);
		if (event->joystick.axis == 0) {
			float curr_y = TGUI5_NORMALISE_JOY_AXIS(SDL_JoystickGetAxis(joy, 1));
			*y = fabsf(event->joystick.value) >= shim::joystick_deactivate_threshold ? 0 : joy_axis1;
			*x = fabsf(curr_y) >= shim::joystick_deactivate_threshold ? 0 : wedge::JOYF_TO_I(event->joystick.value, joy_axis0);
		}
		else {
			float curr_x = TGUI5_NORMALISE_JOY_AXIS(SDL_JoystickGetAxis(joy, 0));
			*x = fabsf(event->joystick.value) >= shim::joystick_deactivate_threshold ? 0 : joy_axis0;
			*y = fabsf(curr_x) >= shim::joystick_deactivate_threshold ? 0 : wedge::JOYF_TO_I(event->joystick.value, joy_axis1);
		}
	}
}

bool Globals::should_show_back_arrow()
{
	if (shim::guis.size() != 0) {
		auto last = shim::guis.back();
		Yes_No_GUI *yn = dynamic_cast<Yes_No_GUI *>(last);
		Multiple_Choice_GUI *m = dynamic_cast<Multiple_Choice_GUI *>(last);
		if (yn) {
			if (yn->get_escape_cancels() == false) {
				return false;
			}
		}
		if (m) {
			if (m->get_escape_choice() == -1) {
				return false;
			}
		}
		return true;
	}
	else if (BATTLE != nullptr) {
		if (GLOBALS->dialogue_active(BATTLE) == true) {
			return false;
		}
		else {
			return true;
		}
	}
	else if (wedge::are_presses_paused() || (AREA && GLOBALS->dialogue_active(AREA) == true)) {
		return false;
	}
	else {
		return true;
	}
}

//--

Globals::Instance::Instance(util::JSON::Node *root) :
	wedge::Globals::Instance(root)
{
	sp_replenish_count = 0;

	enemies_killed = 0;

	util::JSON::Node *shim_json = shim::shim_json->get_root();

	milestones = new bool[MS_SIZE];

	for (int i = 0; i < MS_SIZE; i++) {
		milestones[i] = shim_json->get_nested_bool("wedge>instance>milestones>" + util::itos(i), &milestones[i], false);
	}

	saw_sneak = false;

	if (root) {
		util::JSON::Node *n;
		
		util::JSON::Node *ms = root->find("milestones");
		if (ms != NULL) {
			for (size_t i = 0; i < ms->children.size(); i++) {
				util::JSON::Node *node = ms->children[i];
				milestones[i] = node->as_bool();
			}
		}

		n = root->find("difficulty");
		if (n != NULL) {
			difficulty = (Difficulty)n->as_int();
		}
		n = root->find("saw_sneak");
		if (n != NULL) {
			saw_sneak = n->as_bool();
		}
		n = root->find("enemies_killed");
		if (n != NULL) {
			enemies_killed = n->as_int();
		}
		n = root->find("combos");
		for (size_t i = 0; i < n->children.size(); i++) {
			std::vector<std::string> v;
			util::JSON::Node *n2 = n->children[i];
			for (size_t j = 0; j < n2->children.size(); j++) {
				v.push_back(n2->children[j]->as_string());
			}
			combos.push_back(v);
		}

		if (version < 2) {
			for (auto &s : stats) {
				if (version == 0) {
					s.base.fixed.set_extra(LUCK, 25);
				}
				else {
					int extra_turns = s.base.fixed.get_extra(0);
					s.base.fixed.set_extra(LUCK, 25);
				}
			}
		}
	}
	else {
		stats.push_back(wedge::Player_Stats());
		stats[0].sprite = NULL; // fix crash if line below throws exception
		stats[0].sprite = new gfx::Sprite("mim");
		
		stats[0].base.set_name("Mim");

		for (size_t i = 0; i < stats.size(); i++) {
			stats[i].level = 1;
			stats[i].experience = 0;
			stats[i].base.fixed.max_hp = 100;
			stats[i].base.fixed.attack = 30;
			stats[i].base.fixed.defence = 0;
			stats[i].base.hp = stats[i].base.fixed.max_hp;
			stats[i].base.mp = stats[i].base.fixed.max_mp;
		}

		party_following_player = true;

		std::vector<std::string> v;

		v.push_back("Punch");
		combos.push_back(v);

		v.clear();
		
		v.push_back("Kick");
		combos.push_back(v);
	}
}

Globals::Instance::~Instance()
{
	delete[] milestones;
	for (int i = 0; i < MS_SIZE; i++) {
		shim::shim_json->remove("wedge>instance>milestones>" + util::itos(i), true);
	}
}

int Globals::Instance::num_milestones()
{
	return MS_SIZE;
}

std::string Globals::Instance::save()
{
	std::string s = wedge::Globals::Instance::save();
	s += ",";
	s += "\"difficulty\": " + util::itos((int)difficulty) + ",\n";
	s += "\"saw_sneak\": " + std::string(saw_sneak ? "true" : "false") + ",\n";
	s += "\"enemies_killed\": " + util::itos(enemies_killed) + ",\n";
	s += "\"combos\": [\n";
	for (size_t i = 0; i < combos.size(); i++) {
		std::vector<std::string> &v = combos[i];
		s += "[\n";
		for (size_t j = 0; j < v.size(); j++) {
			s += util::string_printf("\"%s\"%s\n", v[j].c_str(), j == v.size()-1 ? "" : ",");
		}
		s += "]\n";
	}
	s += "]\n";
	return s;
}
