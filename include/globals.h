#ifndef GLOBALS_H
#define GLOBALS_H

#include <wedge3/globals.h>

#define TTH_GLOBALS static_cast<Globals *>(wedge::globals)
#define TTH_INSTANCE static_cast<Globals::Instance *>(TTH_GLOBALS->instance)
#define TTH_BATTLE static_cast<Monster_RPG_3_Battle_Game *>(wedge::globals->battle_game)

const int PLEASANT = 0;
const int TOM = 1;
const int TIK = 2;
const int WIT = 3;
const int NUM_PLAYERS = 4;

const int WIN_BORDER = 5;

enum Difficulty
{
	DIFFICULTY_EASY = 0,
	DIFFICULTY_NORMAL,
	DIFFICULTY_HARD
};

class Globals : public wedge::Globals
{
public:
	Globals();
	virtual ~Globals();

	void gen_level_13();

	bool add_title_gui(bool transition);
	void do_dialogue(std::string tag, std::string text, wedge::Dialogue_Type type, wedge::Dialogue_Position position, wedge::Step *monitor);
	bool dialogue_active(wedge::Game *game, bool only_if_initialised = false, bool wait_for_fade = true);
	void add_notification_gui(std::string text, util::Callback callback = 0, void *callback_data = 0);
	void add_yes_no_gui(std::string text, bool escape_cancels, bool selected, util::Callback callback = 0, void *callback_data = 0);
	bool can_walk();
	bool title_gui_is_top();
	util::Point<float> get_onscreen_button_position(wedge::Onscreen_Button button);
	void run();
	void lost_device();
	void found_device();
	virtual void get_joy_xy(TGUI_Event *event, float joy_axis0, float joy_axis1, int *x, int *y);
	bool should_show_back_arrow();

	class Instance : public wedge::Globals::Instance
	{
	public:
		Instance(util::JSON::Node *root);
		virtual ~Instance();

		int num_milestones();

		std::string save();
	
		Difficulty difficulty;

		bool saw_sneak;

		std::vector< std::vector<std::string> > combos;

		int sp_replenish_count; // when this reaches 1000000, inc. each player's SP by 1. Battle_Game::run and Area_Game::run increment this at different speeds.

		int enemies_killed;
	};

	gfx::Image *up_arrow;
	gfx::Image *down_arrow;
	gfx::Image *speech_window;
	gfx::Image *gui_window;
	gfx::Image *menu_window;
	gfx::Image *battle_window;
	gfx::Image *xb_a;
	gfx::Image *xb_b;
	gfx::Image *xb_x;
	gfx::Image *xb_y;
	gfx::Image *ps_x;
	gfx::Image *ps_circle;
	gfx::Image *ps_square;
	gfx::Image *ps_triangle;
	gfx::Image *b1stable_logo;
	gfx::Image *doughnut_img;
	gfx::Image *heart_img;
	gfx::Image *dice[6];
	gfx::Image *numfont;
	gfx::Image *level_13;
	gfx::Image *big_heart;
	gfx::Image *joker;
	
	gfx::Sprite *heart;
	gfx::Sprite *doughnut;
	gfx::Sprite *mug;

	audio::Sound *doughnut_sfx;
	audio::Sound *error_sfx;
	audio::Sound *battle_start;
	audio::Sound *hit_sfx;
	audio::Sound *mug_sfx;
	audio::Sound *jump_sfx;
	audio::Sound *big_jump_sfx;
	audio::Sound *doughnut_jump_sfx;
	audio::Sound *wow;
	audio::Sound *a_thousand;
	audio::Sound *die_sfx;
	audio::Sound *block_door_sfx;

	int save_slot;
	bool loaded_autosave;
};

extern util::JSON *cfg;

#endif // GLOBALS_H
