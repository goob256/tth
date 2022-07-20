#include "shim3/shaders/glsl/default_vertex.h"
#include "shim3/shaders/glsl/default_fragment.h"

#ifdef _WIN32
#include "shim3/shaders/hlsl/default_vertex.h"
#include "shim3/shaders/hlsl/default_fragment.h"
#endif

#include "wedge3/area.h"
#include "wedge3/area_game.h"
#include "wedge3/battle_game.h"
#include "wedge3/general.h"
#include "wedge3/globals.h"
#include "wedge3/input.h"
#include "wedge3/map_entity.h"
#include "wedge3/omnipresent.h"
#include "wedge3/spells.h"

using namespace wedge;

namespace wedge {

static Uint32 mini_pause_start;
static Uint32 mini_pause_total;
static bool mini_paused;
static Uint32 next_mini_pause_can_start;
static bool old_convert_directions_to_focus_events;

static void quit_callback(void *data)
{
	Yes_No_GUI_Callback_Data *d = (Yes_No_GUI_Callback_Data *)data;
	bool quit = d->cancelled == false && d->choice;
	GLOBALS->quit(quit);
}

Globals *globals;

Globals::Globals()
{
	game_t = NULL;
	english_game_t = NULL;

	spell_interface = NULL;
	object_interface = NULL;
	area_game = NULL;
	battle_game = NULL;
	menu_game = NULL;
	omnipresent_game = NULL;
	shop_game = NULL;
	instance = NULL;

#if defined ANDROID || defined IOS
	rumble_enabled = false;
#else
	rumble_enabled = true;
#endif

	try {
		dpad = new gfx::Sprite("dpad");
	}
	catch (util::Error &e) {
		dpad = NULL;
	}
	cursor = new gfx::Sprite("cursor");
	red_triangle = new gfx::Sprite("red_triangle");
	red_x = new gfx::Sprite("red_x");

	chest = new audio::Sample("chest.flac");
	enemy_die = new audio::Sample("enemy_die.flac");
	victory = new audio::Sample("victory.flac");
	footsteps = new audio::Sample("footsteps.flac");

	button = new audio::MML("sfx/button.mml");
	button->set_pause_with_sfx(false);

	boss_press = DIR_NONE;
	retry_boss = false;
	retried_boss = false;

	create_work_image();
	noise_data = NULL;
	create_noise();

	onscreen_controller_was_enabled = false;
	onscreen_controller_temporarily_disabled = false;
	
	util::JSON::Node *root = shim::shim_json->get_root();

	max_gold = root->get_nested_int("wedge>globals>max_gold", &max_gold, 9999);
	gameover_timeout = root->get_nested_int("wedge>globals>gameover_timeout", &gameover_timeout, 10000);
	gameover_fade_time = root->get_nested_int("wedge>globals>gameover_fade_time", &gameover_fade_time, 2500);

#if (defined IOS && !defined TVOS)
	no_red_triangle = false;
#else
	no_red_triangle = input::system_has_touchscreen() ? false : true;
#endif

	no_red_triangle = root->get_nested_bool("wedge>globals>no_red_triangle", &no_red_triangle, no_red_triangle);

	onscreen_controls_alpha = root->get_nested_float("wedge>globals>onscreen_controls_alpha", &onscreen_controls_alpha, 0.333f);

	red_triangle_colour.r = root->get_nested_byte("wedge>globals>colours>red_triangle_colour.r", &red_triangle_colour.r, shim::palette[13].r);
	red_triangle_colour.g = root->get_nested_byte("wedge>globals>colours>red_triangle_colour.g", &red_triangle_colour.g, shim::palette[13].g);
	red_triangle_colour.b = root->get_nested_byte("wedge>globals>colours>red_triangle_colour.b", &red_triangle_colour.b, shim::palette[13].b);
	red_triangle_colour.a = root->get_nested_byte("wedge>globals>colours>red_triangle_colour.a", &red_triangle_colour.a, shim::palette[13].a);
	
	gameover_fade_colour.r = root->get_nested_byte("wedge>globals>colours>gameover_fade_colour.r", &gameover_fade_colour.r, shim::palette[13].r);
	gameover_fade_colour.g = root->get_nested_byte("wedge>globals>colours>gameover_fade_colour.g", &gameover_fade_colour.g, shim::palette[13].g);
	gameover_fade_colour.b = root->get_nested_byte("wedge>globals>colours>gameover_fade_colour.b", &gameover_fade_colour.b, shim::palette[13].b);
	gameover_fade_colour.a = root->get_nested_byte("wedge>globals>colours>gameover_fade_colour.a", &gameover_fade_colour.a, shim::palette[13].a);
	
	enemy_death_tint.r = root->get_nested_byte("wedge>globals>colours>enemy_death_tint.r", &enemy_death_tint.r, shim::palette[0].r);
	enemy_death_tint.g = root->get_nested_byte("wedge>globals>colours>enemy_death_tint.g", &enemy_death_tint.g, shim::palette[0].g);
	enemy_death_tint.b = root->get_nested_byte("wedge>globals>colours>enemy_death_tint.b", &enemy_death_tint.b, shim::palette[0].b);
	enemy_death_tint.a = root->get_nested_byte("wedge>globals>colours>enemy_death_tint.a", &enemy_death_tint.a, shim::palette[0].a);
	
	helpful_attack_colour.r = root->get_nested_byte("wedge>globals>colours>helpful_attack_colour.r", &helpful_attack_colour.r, shim::white.r);
	helpful_attack_colour.g = root->get_nested_byte("wedge>globals>colours>helpful_attack_colour.g", &helpful_attack_colour.g, shim::white.g);
	helpful_attack_colour.b = root->get_nested_byte("wedge>globals>colours>helpful_attack_colour.b", &helpful_attack_colour.b, shim::white.b);
	helpful_attack_colour.a = root->get_nested_byte("wedge>globals>colours>helpful_attack_colour.a", &helpful_attack_colour.a, shim::white.a);
	
	helpful_attack_shadow.r = root->get_nested_byte("wedge>globals>colours>helpful_attack_shadow.r", &helpful_attack_shadow.r, shim::black.r);
	helpful_attack_shadow.g = root->get_nested_byte("wedge>globals>colours>helpful_attack_shadow.g", &helpful_attack_shadow.g, shim::black.g);
	helpful_attack_shadow.b = root->get_nested_byte("wedge>globals>colours>helpful_attack_shadow.b", &helpful_attack_shadow.b, shim::black.b);
	helpful_attack_shadow.a = root->get_nested_byte("wedge>globals>colours>helpful_attack_shadow.a", &helpful_attack_shadow.a, shim::black.a);
	
	regular_attack_colour.r = root->get_nested_byte("wedge>globals>colours>regular_attack_colour.r", &regular_attack_colour.r, shim::white.r);
	regular_attack_colour.g = root->get_nested_byte("wedge>globals>colours>regular_attack_colour.g", &regular_attack_colour.g, shim::white.g);
	regular_attack_colour.b = root->get_nested_byte("wedge>globals>colours>regular_attack_colour.b", &regular_attack_colour.b, shim::white.b);
	regular_attack_colour.a = root->get_nested_byte("wedge>globals>colours>regular_attack_colour.a", &regular_attack_colour.a, shim::white.a);
	
	regular_attack_shadow.r = root->get_nested_byte("wedge>globals>colours>regular_attack_shadow.r", &regular_attack_shadow.r, shim::black.r);
	regular_attack_shadow.g = root->get_nested_byte("wedge>globals>colours>regular_attack_shadow.g", &regular_attack_shadow.g, shim::black.g);
	regular_attack_shadow.b = root->get_nested_byte("wedge>globals>colours>regular_attack_shadow.b", &regular_attack_shadow.b, shim::black.b);
	regular_attack_shadow.a = root->get_nested_byte("wedge>globals>colours>regular_attack_shadow.a", &regular_attack_shadow.a, shim::black.a);

	strong_attack_colour.r = root->get_nested_byte("wedge>globals>colours>strong_attack_colour.r", &strong_attack_colour.r, shim::white.r);
	strong_attack_colour.g = root->get_nested_byte("wedge>globals>colours>strong_attack_colour.g", &strong_attack_colour.g, shim::white.g);
	strong_attack_colour.b = root->get_nested_byte("wedge>globals>colours>strong_attack_colour.b", &strong_attack_colour.b, shim::white.b);
	strong_attack_colour.a = root->get_nested_byte("wedge>globals>colours>strong_attack_colour.a", &strong_attack_colour.a, shim::white.a);
	
	strong_attack_shadow.r = root->get_nested_byte("wedge>globals>colours>strong_attack_shadow.r", &strong_attack_shadow.r, shim::black.r);
	strong_attack_shadow.g = root->get_nested_byte("wedge>globals>colours>strong_attack_shadow.g", &strong_attack_shadow.g, shim::black.g);
	strong_attack_shadow.b = root->get_nested_byte("wedge>globals>colours>strong_attack_shadow.b", &strong_attack_shadow.b, shim::black.b);
	strong_attack_shadow.a = root->get_nested_byte("wedge>globals>colours>strong_attack_shadow.a", &strong_attack_shadow.a, shim::black.a);

	key_action = TGUIK_RETURN;
	key_back = TGUIK_ESCAPE;
	key_doughnut = TGUIK_x;
	key_die = TGUIK_z;
	key_l = TGUIK_LEFT;
	key_r = TGUIK_RIGHT;
	key_u = TGUIK_UP;
	key_d = TGUIK_DOWN;
	joy_action = TGUI_B_A;
	joy_back = TGUI_B_B;
	joy_doughnut = TGUI_B_Y;
	joy_die = TGUI_B_X;

	language = "English";

	terminate = false;

	allow_global_mini_pause = true;
	getting_combo = false;
	assigning_controls = false;

	prerendered_music = true;

	speed_run = false;
}

Globals::~Globals()
{
	delete dpad;
	delete cursor;
	delete red_triangle;
	delete red_x;

	//delete red_triangle;
	//delete red_x;

	delete chest;
	delete button;
	delete enemy_die;
	delete victory;
	delete footsteps;

	//delete gameover;

	delete spell_interface;
	delete object_interface;

	delete instance;

	delete work_image;

	delete noise;
	delete[] noise_data;

	delete game_t;
	delete english_game_t;
	
	shim::shim_json->remove("wedge>globals>max_gold", true);
	shim::shim_json->remove("wedge>globals>gameover_timeout", true);
	shim::shim_json->remove("wedge>globals>gameover_fade_time", true);

	shim::shim_json->remove("wedge>globals>key_b1", true);
	shim::shim_json->remove("wedge>globals>key_b2", true);
	shim::shim_json->remove("wedge>globals>key_b3", true);
	shim::shim_json->remove("wedge>globals>key_b4", true);
	shim::shim_json->remove("wedge>globals>key_back", true);
	shim::shim_json->remove("wedge>globals>key_switch", true);
	shim::shim_json->remove("wedge>globals>key_l", true);
	shim::shim_json->remove("wedge>globals>key_r", true);
	shim::shim_json->remove("wedge>globals>key_u", true);
	shim::shim_json->remove("wedge>globals>key_d", true);
	shim::shim_json->remove("wedge>globals>joy_action", true);
	shim::shim_json->remove("wedge>globals>joy_b1", true);
	shim::shim_json->remove("wedge>globals>joy_b2", true);
	shim::shim_json->remove("wedge>globals>joy_b3", true);
	shim::shim_json->remove("wedge>globals>joy_b4", true);
	shim::shim_json->remove("wedge>globals>joy_back", true);

	shim::shim_json->remove("wedge>globals>onscreen_controls_alpha", true);

	shim::shim_json->remove("wedge>globals>enemy_death_tint.r", true);
	shim::shim_json->remove("wedge>globals>enemy_death_tint.g", true);
	shim::shim_json->remove("wedge>globals>enemy_death_tint.b", true);
	shim::shim_json->remove("wedge>globals>enemy_death_tint.a", true);
	
	shim::shim_json->remove("wedge>globals>poison_attack_colour.r", true);
	shim::shim_json->remove("wedge>globals>poison_attack_colour.g", true);
	shim::shim_json->remove("wedge>globals>poison_attack_colour.b", true);
	shim::shim_json->remove("wedge>globals>poison_attack_colour.a", true);

	shim::shim_json->remove("wedge>globals>poison_attack_shadow.r", true);
	shim::shim_json->remove("wedge>globals>poison_attack_shadow.g", true);
	shim::shim_json->remove("wedge>globals>poison_attack_shadow.b", true);
	shim::shim_json->remove("wedge>globals>poison_attack_shadow.a", true);
	
	shim::shim_json->remove("wedge>globals>helpful_attack_colour.r", true);
	shim::shim_json->remove("wedge>globals>helpful_attack_colour.g", true);
	shim::shim_json->remove("wedge>globals>helpful_attack_colour.b", true);
	shim::shim_json->remove("wedge>globals>helpful_attack_colour.a", true);
	
	shim::shim_json->remove("wedge>globals>helpful_attack_shadow.r", true);
	shim::shim_json->remove("wedge>globals>helpful_attack_shadow.g", true);
	shim::shim_json->remove("wedge>globals>helpful_attack_shadow.b", true);
	shim::shim_json->remove("wedge>globals>helpful_attack_shadow.a", true);
	
	shim::shim_json->remove("wedge>globals>regular_attack_colour.r", true);
	shim::shim_json->remove("wedge>globals>regular_attack_colour.g", true);
	shim::shim_json->remove("wedge>globals>regular_attack_colour.b", true);
	shim::shim_json->remove("wedge>globals>regular_attack_colour.a", true);
	
	shim::shim_json->remove("wedge>globals>regular_attack_shadow.r", true);
	shim::shim_json->remove("wedge>globals>regular_attack_shadow.g", true);
	shim::shim_json->remove("wedge>globals>regular_attack_shadow.b", true);
	shim::shim_json->remove("wedge>globals>regular_attack_shadow.a", true);

	shim::shim_json->remove("English", true);
}

void Globals::add_next_dialogue_monitor(wedge::Step *monitor)
{
	next_dialogue_monitors.push_back(monitor);
}

void Globals::create_work_image()
{
	bool old_create_depth_buffer = gfx::Image::create_depth_buffer;
	gfx::Image::create_depth_buffer = true;
	work_image = new gfx::Image(shim::real_screen_size);
	gfx::Image::create_depth_buffer = old_create_depth_buffer;
}

void Globals::create_noise()
{
	const int size = 256;

	if (noise_data == NULL) {
		noise_data = new Uint8[size*size*4];

		util::srand(0); // make it the same each time

		for (int i = 0; i < size*size*4; i += 4) {
			Uint8 *p = noise_data + i;
			p[0] = util::rand(0, 255);
			p[1] = util::rand(0, 255);
			p[2] = util::rand(0, 255);
			p[3] = util::rand(0, 255);
		}

		util::srand((uint32_t)time(NULL));
	}

	noise = new gfx::Image(noise_data, util::Size<int>(size, size));
}

util::Point<float> Globals::get_onscreen_button_position(Onscreen_Button button)
{
	return util::Point<float>(0.0f, 0.0f);
}

void Globals::mini_pause(bool is_joystick_disconnect_notification, bool is_sneak_attack_notification)
{
	if (instance == NULL || mini_paused || SDL_GetTicks() < next_mini_pause_can_start) {
		return;
	}

	button->play(false);

	mini_paused = true;
	mini_pause_start = SDL_GetTicks();
	old_convert_directions_to_focus_events = shim::convert_directions_to_focus_events;
	shim::convert_directions_to_focus_events = true;

	audio::pause_sfx(true);

	if (is_joystick_disconnect_notification) {
		add_notification_gui(GLOBALS->game_t->translate(192)/* Originally: Joystick disconnected! */, quit_callback);
	}
	else if (is_sneak_attack_notification) {
		add_notification_gui(GLOBALS->game_t->translate(392)/* Originally: You've been attacked from behind! For combos, left is now right and right is left! */, quit_callback);
	}
	else {
		add_yes_no_gui(GLOBALS->game_t->translate(10)/* Originally: Quit to title? */, true, false, quit_callback);
	}
}

bool Globals::is_mini_paused()
{
	if (instance == NULL) {
		return false;
	}
	return mini_paused;
}

void Globals::quit(bool yesno)
{
	if (yesno) {
		quit_all();
	}
	
	mini_paused = false;
	mini_pause_total += SDL_GetTicks() - mini_pause_start;
	next_mini_pause_can_start = SDL_GetTicks() + 100;
	shim::convert_directions_to_focus_events = yesno ? true : old_convert_directions_to_focus_events;
	
	audio::pause_sfx(false);
}

Uint32 Globals::get_ticks()
{
	if (instance == NULL) {
		return SDL_GetTicks();
	}
	else if (mini_paused) {
		return mini_pause_start - mini_pause_total;
	}
	else {
		return SDL_GetTicks() - mini_pause_total;
	}
}

void Globals::run()
{
}

//--

Globals::Instance::Instance(util::JSON::Node *root) :
	milestones(nullptr)
{
	play_time = 0;

	num_levels = 100;

	doughnuts_used = 0;
	
	mini_paused = false;
	mini_pause_total = 0;
	next_mini_pause_can_start = 0;

	version = VERSION;
	
	util::JSON::Node *shim_json = shim::shim_json->get_root();

	if (root) {
		util::JSON::Node *cfg = root->find("game");
		util::JSON::Node *n;

		n = cfg->find("version");
		if (n == NULL) {
			version = 0;
		}
		else {
			version = n->as_int();
		}

		n = cfg->find("gold");
		if (n != NULL) {
			gold = shim_json->get_nested_int("wedge>instance>gold", &gold, n->as_int());
		}
		n = cfg->find("num_levels");
		if (n != NULL) {
			num_levels = n->as_int();
		}
		n = cfg->find("doughnuts_used");
		if (n != NULL) {
			doughnuts_used = n->as_int();
		}
		n = cfg->find("step_count");
		if (n != NULL) {
			step_count = n->as_int();
		}
		n = cfg->find("play_time");
		if (n != NULL) {
			play_time = n->as_int();
		}
		n = cfg->find("party_following_player");
		if (n != NULL) {
			party_following_player = n->as_bool();
		}
		n = cfg->find("chests_opened");
		if (n != NULL) {
			chests_opened = n->as_int();
		}

		n = root->find("players");

		for (size_t i = 0; i < n->children.size(); i++) {
			stats.push_back(Player_Stats(n->children[i]));
		}

		n = root->find("inventory");
		if (n != NULL) {
			inventory = Inventory(n);
			inventory.sort();
		}
	}
	else {
		gold = shim_json->get_nested_int("wedge>instance>gold", &gold, 0);
		step_count = 0;

		party_following_player = false;

		chests_opened = 0;
	}
	play_start = GLOBALS->get_ticks();
}

Globals::Instance::~Instance()
{
	for (size_t i = 0; i < stats.size(); i++) {
		delete stats[i].sprite;
	}
	
	shim::shim_json->remove("wedge>instance>gold", true);
}	

bool Globals::Instance::is_milestone_complete(int milestone)
{
	return milestones[milestone];
}

void Globals::Instance::set_milestone_complete(int milestone, bool complete)
{
	milestones[milestone] = complete;
}

int Globals::Instance::get_gold()
{
	return gold;
}

void Globals::Instance::add_gold(int amount)
{
	gold += amount;
	if (amount < 0) {
		use_doughnut();
	}
	if (gold < 0) {
		gold = 0;
	}
	if (gold > globals->max_gold) {
		gold = globals->max_gold;
	}
}

int Globals::Instance::get_num_levels()
{
	return num_levels;
}

void Globals::Instance::set_num_levels(int num_levels)
{
	this->num_levels = num_levels;
}

int Globals::Instance::get_doughnuts_used()
{
	return doughnuts_used;
}

void Globals::Instance::use_doughnut()
{
	doughnuts_used++;
}

std::string Globals::Instance::save()
{
	std::string s;
	s += "\"game\": {";
	s += "\"version\": " + util::itos(VERSION) + ",";
	s += "\"gold\": " + util::itos(get_gold()) + ",";
	s += "\"num_levels\": " + util::itos(get_num_levels()) + ",";
	s += "\"doughnuts_used\": " + util::itos(get_doughnuts_used()) + ",";
	s += "\"step_count\": " + util::itos(step_count) + ",";
	s += "\"party_following_player\": " + bool_to_string(party_following_player) + ",";
	s += "\"chests_opened\": " + util::itos(chests_opened) + ",";
	s += "\"play_time\": " + util::itos(play_time);
	s += "},";
	s += "\"players\": [";
	for (size_t i = 0; i < stats.size(); i++) {
		s += stats[i].save();
		if (i < stats.size()-1) {
			s += ",";
		}
	}
	s += "],";
	s += "\"inventory\": [";
	s += inventory.save();
	s += "],";
	s += "\"milestones\": [";
	for (int i = 0; i < num_milestones(); i++) {
		if (is_milestone_complete(i)) {
			s += "true";
		}
		else {
			s += "false";
		}
		if (i < num_milestones()-1) {
			s += ",";
		}
	}
	s += "]";
	return s;
}

int Globals::Instance::get_version()
{
	return version;
}

void Globals::draw_custom_status(Map_Entity *entity, int status, util::Point<float> draw_pos)
{
}

void Globals::load_translation()
{
	delete game_t;
	delete english_game_t;

	std::string game_t_text = util::load_text(std::string("text/") + language + std::string(".utf8"));
	std::string english_game_t_text = util::load_text("text/English.utf8");

	game_t = new util::Translation(game_t_text);
	english_game_t = new util::Translation(english_game_t_text);

	/*
	// Do this if using a TTF
	entire_translation = game_t->get_entire_translation();

	std::vector<std::string> filenames = shim::cpa->get_all_filenames();

	for (size_t i = 0; i < filenames.size(); i++) {
		util::Tokenizer t(filenames[i], '|');
		std::string s = t.next();
		s = t.next();
		entire_translation += s;
	}
	*/
}

std::string Globals::tag_end()
{
	if (language == "French") {
		return std::string(" : ");
	}
	else {
		return std::string(": ");
	}
}

void Globals::lost_device()
{
	delete globals->work_image;
	globals->work_image = NULL;
	delete globals->noise;
	globals->noise = NULL;
}

void Globals::found_device()
{
	globals->create_work_image();
	globals->create_noise();

}

}
