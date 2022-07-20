#ifdef ANDROID
#include <jni.h>
#endif

#include <wedge3/area_game.h>
#include <wedge3/general.h>
#include <wedge3/onscreen_controller.h>
#include <wedge3/omnipresent.h>
#include <wedge3/systems.h>

#include "area_game.h"
#include "battle_game.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "gui_drawing_hook.h"
#include "menu.h"
#include "widgets.h"

#ifdef IOS
#include <shim3/ios.h>
#include "ios.h"
#endif

#ifdef __APPLE__
#include "shim3/internal/gfx.h"
#endif

TTH_GUI::TTH_GUI()
{
	transition = true;
	transition_is_slide_vertical = true;
}

TTH_GUI::~TTH_GUI()
{
}

//--

static void speed_run_callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Title_GUI *gui = static_cast<Title_GUI *>(d->userdata);
	if (d->choice < 0) {
		return;
	}
	int levels = 100;
	if (d->choice == 0) {
		levels = 100;
	}
	else if (d->choice == 1) {
		levels = 1000;
	}
	else {
		levels = 1000000;
	}
	gui->do_speed_run(levels);
}

Title_GUI::Title_GUI(bool transition)
{
	OMNIPRESENT->end_fade();

	if (transition) {
		this->transition = true;
		transition_is_slide_vertical = true;
	}
	else {
		this->transition = false;
	}

	//if (wedge::globals->gameover->is_playing()) {
		//wedge::globals->gameover->stop();
	//}

	//TTH_GLOBALS->credits_music->stop();

	if (GLOBALS->prerendered_music) {
		audio::play_music("music/title_prerendered.mml");
	}
	else {
		audio::play_music("music/title.mml");
	}

	logo = new gfx::Image("misc/logo.tga");
	tower_left = new gfx::Image("misc/tower_left.tga");
	tower_right = new gfx::Image("misc/tower_right.tga");

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	TGUI_Widget *both = new TGUI_Widget(0.8f, 4*shim::font->get_height());
	both->set_centre_x(true);
	both->set_centre_y(true);
	both->set_parent(modal_main_widget);

	TGUI_Widget *left = new TGUI_Widget(0.49f, 10);
	left->set_parent(both);

	container = new TGUI_Widget(0.49f, 4*shim::font->get_height());
	//set_container_pad();
	container->set_padding_left(20);
	container->set_parent(both);

	play_button = new Widget_Text_Button(GLOBALS->game_t->translate(607)/* Originally: Casual */);
	play_button->set_parent(container);

	speed_run_button = new Widget_Text_Button(GLOBALS->game_t->translate(608)/* Originally: Speed-Run */);
	speed_run_button->set_break_line(true);
	speed_run_button->set_parent(container);

	settings_button = new Widget_Text_Button(GLOBALS->game_t->translate(50)/* Originally: Settings */);
	settings_button->set_break_line(true);
	settings_button->set_parent(container);

#ifndef IOS
	quit_button = new Widget_Text_Button(GLOBALS->game_t->translate(51)/* Originally: Quit */);
	quit_button->set_break_line(true);
	quit_button->set_parent(container);
#endif

	// Wrap cursor
#ifdef IOS
	play_button->set_up_widget(settings_button);
	settings_button->set_down_widget(play_button);
#else
	play_button->set_up_widget(quit_button);
	quit_button->set_down_widget(play_button);
#endif

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(play_button);

#ifdef IOS
	OMNIPRESENT->set_hide_red_triangle(true);
#endif
#ifdef TVOS
	shim::pass_menu_to_os = true;
#endif
}

Title_GUI::~Title_GUI()
{
	delete logo;
	delete tower_left;
	delete tower_right;
}

void Title_GUI::draw()
{
	gfx::draw_filled_rectangle(shim::black, {0, 0}, shim::screen_size);
	int h = shim::screen_size.h;
	int y = -(tower_left->size.h-(h % tower_left->size.h));
	//tower_left->stretch_region_tinted_repeat(shim::white, {0.0f, 0.0f}, tower_left->size, {0.0f, (float)y}, {tower_left->size.w, shim::screen_size.h+(-y)}, 0);
	//tower_right->stretch_region_tinted_repeat(shim::white, {0.0f, 0.0f}, tower_right->size, {(float)(shim::screen_size.w-tower_right->size.w), (float)y}, {tower_right->size.w, shim::screen_size.h+(-y)}, 0);
	std::string text = GLOBALS->game_t->translate(570)/* Originally: © 2021 b1stable */;
	SDL_Colour c;
	c.r = 64;
	c.g = 64;
	c.b = 64;
	c.a = 255;
	TTH_GLOBALS->bold_font->draw(c, text, util::Point<int>(shim::screen_size.w/2-TTH_GLOBALS->bold_font->get_text_width(text)/2, shim::screen_size.h-3-TTH_GLOBALS->bold_font->get_height()));
	util::Point<int> draw_pos(shim::screen_size.w*0.2f+shim::screen_size.w*0.4f/2.0f-logo->size.w+20, shim::screen_size.h/2-logo->size.h/2);

	logo->draw(draw_pos);

	GUI::draw();
}

void Title_GUI::update()
{
	TTH_GUI::update();

	if (transitioning_in == false) {
		transition = true;
	}

#ifndef IOS
	if (quit_button->pressed()) {
		GLOBALS->terminate = true;
		transition = false;
		exit();
	}
	else
#endif
	if (play_button->pressed()) {
		TTH_GLOBALS->speed_run = false;
#ifdef IOS
		OMNIPRESENT->set_hide_red_triangle(false);
#endif
#ifdef TVOS
		shim::pass_menu_to_os = false;
#endif
		Save_Slot_GUI *gui = new Save_Slot_GUI();
		shim::guis.push_back(gui);
		exit();
	}
	else if (speed_run_button->pressed()) {
		TTH_GLOBALS->save_slot = -1;
		TTH_GLOBALS->speed_run = true;
		std::vector<std::string> choices;
		choices.push_back(GLOBALS->game_t->translate(595)/* Originally: 100 Levels */);
		choices.push_back(GLOBALS->game_t->translate(596)/* Originally: 1000 Levels */);
		Positioned_Multiple_Choice_GUI *gui = new Positioned_Multiple_Choice_GUI(true, GLOBALS->game_t->translate(70)/* Originally: Select Difficulty */, choices, -2, 0, 0, 0, 0, 0, 0, 0, 0, speed_run_callback, this, 2, 125);
		gui->set_wrap(true);
		gui->set_selected(0);
		gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
		shim::guis.push_back(gui);
	}
	else if (settings_button->pressed()) {
#ifdef IOS
		OMNIPRESENT->set_hide_red_triangle(false);
#endif
#ifdef TVOS
		shim::pass_menu_to_os = false;
#endif
		Settings_GUI *gui = new Settings_GUI(false);
		shim::guis.push_back(gui);
		exit();
	}
}

void Title_GUI::do_speed_run(int levels)
{
	INSTANCE = new Globals::Instance(NULL);
	INSTANCE->set_num_levels(levels);;
	AREA = new Area_Game();
	AREA->start_area(NULL);
	exit();
}

void Title_GUI::resize(util::Size<int> new_size)
{
	//set_container_pad();
	gui->layout();
}

void Title_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

#ifndef IOS
	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		GLOBALS->button->play(false);
		transition = false;
		GLOBALS->terminate = true;
		exit();
	}
	else
#endif
	{
		gui::GUI::handle_event(event);
	}
}

void Title_GUI::focus_button(int n)
{
	switch (n) {
		case 0:
			gui->set_focus(play_button);
			break;
		case 1:
			gui->set_focus(settings_button);
			break;
#ifndef IOS
		case 2:
			gui->set_focus(quit_button);
			break;
#endif
	}
}

void Title_GUI::set_container_pad()
{
#ifdef IOS
	container->set_padding_top(logo->size.h+(shim::screen_size.h-logo->size.h)/2-shim::font->get_height()*3/2-5);
#else
	container->set_padding_top(logo->size.h+(shim::screen_size.h-logo->size.h)/2-shim::font->get_height()*4/2-5);
#endif
}

//--

static void erase_callback(void *data)
{
	wedge::Yes_No_GUI_Callback_Data *d = static_cast<wedge::Yes_No_GUI_Callback_Data *>(data);
	Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
	if (d->cancelled == false && d->choice == true) {
		gui->erase();
	}
	else {
		gui->show_sub_menu(1);
	}
}

static void load_callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
	if (d->choice == 0) {
		gui->load();
	}
	else if (d->choice == 1) {
		Yes_No_GUI *yn_gui = new Yes_No_GUI(GLOBALS->game_t->translate(66)/* Originally: Erase this game? */, true, erase_callback, gui);
		shim::guis.push_back(yn_gui);
	}
}

static void autosave_callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
	if (d->choice < 0) {
		gui->show_sub_menu(1);
		return;
	}
	gui->load_autosave(d->choice);
}

static void difficulty_callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
	if (d->choice < 0) {
		return;
	}
	gui->new_game(d->choice);
}

/*
static void battle_mode_callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
	if (d->choice < 0) {
		return;
	}
	gui->new_game(d->choice);
}
*/

static void show_autosaves_sub(void *data)
{
	Notification_GUI::Callback_Data *d = static_cast<Notification_GUI::Callback_Data *>(data);
	Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
	gui->show_sub_menu(1);
}

static void autosave_help_done(void *data)
{
	Notification_GUI::Callback_Data *d = static_cast<Notification_GUI::Callback_Data *>(data);
	Save_Slot_GUI *gui = static_cast<Save_Slot_GUI *>(d->userdata);
	gui->show_sub_menu(0);
}

static void saved_callback(void *data)
{
	auto ngd = static_cast<Notification_GUI::Callback_Data *>(data);
	auto ssg = static_cast<Save_Slot_GUI *>(ngd->userdata);
	ssg->saved();
}

static void save_failed_callback(void *data)
{
	auto ngd = static_cast<Notification_GUI::Callback_Data *>(data);
	auto ssg = static_cast<Save_Slot_GUI *>(ngd->userdata);
	ssg->save_failed();
}

static void save_callback(void *data)
{
	wedge::Yes_No_GUI_Callback_Data *d = static_cast<wedge::Yes_No_GUI_Callback_Data *>(data);

	if (d->cancelled == false && d->choice == true) {
		std::string msg;
		util::Callback cb;
		if (save()) {
			msg = GLOBALS->game_t->translate(158)/* Originally: Game saved! */;
			TTH_GLOBALS->loaded_autosave = false;
			cb = saved_callback;
			// Make the text appear behind the Notification
			auto ssg = static_cast<Save_Slot_GUI *>(d->userdata);
			ssg->set_text();
		}
		else {
			msg = GLOBALS->game_t->translate(159)/* Originally: Error saving game! */;
			cb = save_failed_callback;
		}
		Notification_GUI *gui = new Notification_GUI(msg, cb, d->userdata);
		shim::guis.push_back(gui);
	}
}

Save_Slot_GUI::Save_Slot_GUI(bool loading, int start_slot) :
	selected(-1),
	loading(loading)
{
	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	TGUI_Widget *container = new TGUI_Widget();
	container->set_centre_x(true);
	container->set_centre_y(true);
	container->set_padding_top(10);
	container->set_parent(modal_main_widget);

	for (int i = 0; i < NUM_SLOTS; i++) {
		slots[i] = new Widget_Save_Slot(i);
		if (i > 0) {
			slots[i]->set_break_line(true);
			slots[i]->set_padding_top(1);
		}
		slots[i]->set_parent(container);
	}

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(slots[start_slot]);
}

Save_Slot_GUI::~Save_Slot_GUI()
{
}

void Save_Slot_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}
	
	TTH_GUI::handle_event(event);

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		if (AREA) {
			auto gui = new Main_Menu_GUI(6);
			shim::guis.push_back(gui);
			exit();
		}
		else {
			GLOBALS->add_title_gui(true);
			static_cast<Title_GUI *>(shim::guis.back())->set_transition(true);
			GLOBALS->button->play(false);
			exit();
		}
	}
}

void Save_Slot_GUI::saved()
{
	auto gui = new Main_Menu_GUI(6);
	shim::guis.push_back(gui);
	exit();
}

void Save_Slot_GUI::save_failed()
{
	TTH_GLOBALS->save_slot = prev_slot;
}

void Save_Slot_GUI::update()
{
	TTH_GUI::update();

	for (int i = 0; i < NUM_SLOTS; i++) {
		if (slots[i]->pressed()) {
			selected = i;
			if (loading == false) {
				prev_slot = TTH_GLOBALS->save_slot;
				TTH_GLOBALS->save_slot = i;
			}
			if (slots[i]->exists() == false) {
				if (loading) {
					int n = get_selected_number();

					/*
					if (n >= 0 || n < NUM_SLOTS) {
						TTH_GLOBALS->save_slot = n;
						INSTANCE = new Globals::Instance(NULL);
						//TTH_INSTANCE->difficulty = this->difficulty;
						AREA = new Area_Game();
						AREA->start_area(NULL);
						exit();
					}
					*/
					std::vector<std::string> choices;
					choices.push_back(GLOBALS->game_t->translate(595)/* Originally: 100 Levels */);
					choices.push_back(GLOBALS->game_t->translate(596)/* Originally: 1000 Levels */);
					Positioned_Multiple_Choice_GUI *gui = new Positioned_Multiple_Choice_GUI(true, GLOBALS->game_t->translate(70)/* Originally: Select Difficulty */, choices, -2, 0, 0, 0, 0, 0, 0, 0, 0, difficulty_callback, this, 2, 125);
					gui->set_wrap(true);
					gui->set_selected(0);
					gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
					shim::guis.push_back(gui);
				}
				else {
					std::string msg;
					util::Callback cb;
					if (save()) {
						msg = GLOBALS->game_t->translate(158)/* Originally: Game saved! */;
						TTH_GLOBALS->loaded_autosave = false;
						cb = saved_callback;
					}
					else {
						msg = GLOBALS->game_t->translate(159)/* Originally: Error saving game! */;
						cb = save_failed_callback;
					}
					Notification_GUI *gui = new Notification_GUI(msg, cb, this);
					shim::guis.push_back(gui);
				}
			}
			else {
				if (loading) {
					int count;

					if (autosave_help_shown) {
						count = 0;
					}
					else {
						bool tmp_exists[NUM_AUTOSAVES];
						bool tmp_corrupt[NUM_AUTOSAVES];
						std::string tmp_time[NUM_AUTOSAVES];
						std::string tmp_place[NUM_AUTOSAVES];

						for (count = 0; count < NUM_AUTOSAVES; count++) {
							int dummy;

							get_save_info(count, tmp_exists[count], tmp_corrupt[count], tmp_time[count], tmp_place[count], dummy, "auto" + util::itos(selected+1) + "-");

							if (tmp_exists[count] == false || tmp_corrupt[count] == true) {
								break;
							}
						}
					}

					if (count > 0 && autosave_help_shown == false) {
						autosave_help_shown = true;
						save_settings();
						Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(369)/* Originally: This game features autosaves. Click "Load" to load your last manual save. Click "Show Autosaves" to show automatic saves. */, autosave_help_done, this);
						shim::guis.push_back(gui);
						return;
					}
					else {
						show_sub_menu(0);
					}
				}
				else {
					Yes_No_GUI *yn_gui = new Yes_No_GUI(GLOBALS->game_t->translate(157)/* Originally: Overwrite this save? */, true, save_callback, this);
					shim::guis.push_back(yn_gui);
				}
			}
		}
	}
}

void Save_Slot_GUI::draw_back()
{
	GUI::draw_back();

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), shim::screen_size);
}

void Save_Slot_GUI::draw()
{
	TTH_GUI::draw();
	std::string text = GLOBALS->game_t->translate(59)/* Originally: Choose a Slot */;
	SDL_Colour c1;
	c1.r = 253;
	c1.g = 77;
	c1.b = 79;
	c1.a = 255;
	SDL_Colour c2 = shim::black;
	TTH_GLOBALS->bold_font->enable_shadow(c1, gfx::Font::FULL_SHADOW);
	TTH_GLOBALS->bold_font->draw(c2, text, util::Point<int>(shim::screen_size.w/2-shim::font->get_text_width(text)/2, 5));
	TTH_GLOBALS->bold_font->disable_shadow();
	GUI::draw();
}

void Save_Slot_GUI::erase()
{
	int n = get_selected_number();
	
	if (n < 0 || n >= NUM_SLOTS) {
		return;
	}

	std::string filename = save_filename(n);
#ifdef TVOS
	util::tvos_delete_file(filename);
#else
	remove(filename.c_str());
#endif

	for (int i = 0; i < NUM_AUTOSAVES; i++) {
		std::string filename = save_filename(i, "auto" + util::itos(n+1) + "-");
#ifdef TVOS
		util::tvos_delete_file(filename);
#else
		remove(filename.c_str());
#endif
	}

	set_text();
}

void Save_Slot_GUI::show_autosaves()
{
	bool tmp_exists[NUM_AUTOSAVES];
	bool tmp_corrupt[NUM_AUTOSAVES];
	std::string tmp_time[NUM_AUTOSAVES];
	std::string tmp_place[NUM_AUTOSAVES];

	int count;

	for (count = 0; count < NUM_AUTOSAVES; count++) {
		int dummy;

		get_save_info(count, tmp_exists[count], tmp_corrupt[count], tmp_time[count], tmp_place[count], dummy, "auto" + util::itos(selected+1) + "-");

		if (tmp_exists[count] == false || tmp_corrupt[count] == true) {
			break;
		}
	}

	if (count == 0) {
		Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(68)/* Originally: No autosaves! */, show_autosaves_sub, this);
		shim::guis.push_back(gui);
		return;
	}

	std::vector<std::string> times;
	std::vector<std::string> places;

	for (int i = 0; i < count; i++) {
		times.push_back(tmp_time[i]);
		places.push_back(tmp_place[i]);
	}

	Widget_Autosaves_List *list = new Widget_Autosaves_List(0, 0);
	list->set_items_extra(times, places);
				
	Positioned_Multiple_Choice_GUI *gui = new Positioned_Multiple_Choice_GUI(true, GLOBALS->game_t->translate(67)/* Originally: Select Autosave */, times, -3, 0, 0, 0, 0, 0, 0, 0, 0, autosave_callback, this, (int)times.size(), 0, false, 0.75f, list);
	gui->set_wrap(true);
	gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
	shim::guis.push_back(gui);
}

void Save_Slot_GUI::load(util::JSON *json)
{
	util::JSON::Node *root = json->get_root();
	INSTANCE = new Globals::Instance(root);
	AREA = new Area_Game();
	util::JSON::Node *saved = root->find("areas");
	wedge::Area *area = AREA->create_area(saved->children[0]);
	area->start();
	area->set_hooks(AREA->get_area_hooks(area->get_name(), area));

	for (size_t i = 0; i < saved->children.size(); i++) {
		INSTANCE->saved_levels[saved->children[i]->key] = saved->children[i]->to_json();
	}

	AREA->start_area(area);

	delete json;
	
	exit();
}

void Save_Slot_GUI::load()
{
	int n = get_selected_number();

	if (n < 0 || n >= NUM_SLOTS) {
		return;
	}

	TTH_GLOBALS->save_slot = n;
	TTH_GLOBALS->loaded_autosave = false;

	util::JSON *json = load_savegame(n);
	load(json);
}

void Save_Slot_GUI::load_autosave(int i)
{
	int n = get_selected_number();

	if (n < 0 || n >= NUM_SLOTS) {
		return;
	}

	TTH_GLOBALS->save_slot = n;
	TTH_GLOBALS->loaded_autosave = true;

	util::JSON *json = load_savegame(i, "auto" + util::itos(n+1) + "-");
	load(json);
}

int Save_Slot_GUI::get_selected_number()
{
	return selected;
}

void Save_Slot_GUI::set_text()
{
	for (int i = 0; i < NUM_SLOTS; i++) {
		slots[i]->set_text();
	}
}

#if 0
void Save_Slot_GUI::select_difficulty(Difficulty difficulty)
{
	this->difficulty = difficulty;

	std::vector<std::string> choices;
	choices.push_back(GLOBALS->game_t->translate(373)/* Originally: Select from List */);
	choices.push_back(GLOBALS->game_t->translate(372)/* Originally: Fighting Combos (Hard Mode) */);
	Positioned_Multiple_Choice_GUI *gui = new Positioned_Multiple_Choice_GUI(true, GLOBALS->game_t->translate(371)/* Originally: Battle Mode */, choices, -2, 0, 0, 0, 0, 0, 0, 0, 0, battle_mode_callback, this, 2);
	gui->set_wrap(true);
	gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
	shim::guis.push_back(gui);
}
#endif

void Save_Slot_GUI::new_game(int answer)
{
	int n = get_selected_number();

	if (n < 0 || n >= NUM_SLOTS) {
		return;
	}

	TTH_GLOBALS->save_slot = n;
	INSTANCE = new Globals::Instance(NULL);
	if (answer == 0) {
		INSTANCE->set_num_levels(100);
	}
	else {
		INSTANCE->set_num_levels(1000);;
	}
	AREA = new Area_Game();
	AREA->start_area(NULL);
	exit();
}

void Save_Slot_GUI::show_sub_menu(int selected)
{
	std::vector<std::string> choices;
	choices.push_back(GLOBALS->game_t->translate(62)/* Originally: Load */);
	choices.push_back(GLOBALS->game_t->translate(64)/* Originally: Erase */);
	Positioned_Multiple_Choice_GUI *gui = new Positioned_Multiple_Choice_GUI(true, GLOBALS->game_t->translate(65)/* Originally: Select Action */, choices, -2, 0, 0, 0, 0, 0, 0, 0, 0, load_callback, this, 2);
	gui->set_wrap(true);
	gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
	gui->set_selected(selected);
	shim::guis.push_back(gui);
}

//--

Yes_No_GUI::Yes_No_GUI(std::string text, bool escape_cancels, util::Callback callback, void *callback_data, bool shrink_to_fit) :
	escape_cancels(escape_cancels),
	callback(callback),
	callback_data(callback_data),
	_hook_omnipresent(false),
	hook_draw_last(false),
	count(0),
	drawing_hook(NULL)
{
	transition = true;
	transition_is_shrink = true;
	transition_duration = 200;
	started_transition_timer = false;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	SDL_Colour background_colour = shim::black;
	background_colour.r *= 0.5f;
	background_colour.g *= 0.5f;
	background_colour.b *= 0.5f;
	background_colour.a *= 0.5f;
	modal_main_widget->set_background_colour(background_colour);

	int window_w = int(shim::screen_size.w * 0.75f);

	bool full;
	int num_lines, width;
	int line_height = TTH_GLOBALS->bold_font->get_height() + 1;
	TTH_GLOBALS->bold_font->draw_wrapped(shim::white, text, util::Point<int>(0, 0), window_w - WIN_BORDER*4, line_height, -1, -1, 0, true, full, num_lines, width);
	
	yes_button = new Widget_Text_Button(GLOBALS->game_t->translate(60)/* Originally: Yes */);
	no_button = new Widget_Text_Button(GLOBALS->game_t->translate(61)/* Originally: No */);

	// Wrap
	yes_button->set_left_widget(no_button);
	no_button->set_right_widget(yes_button);

	if (shrink_to_fit) {
		window_w = MIN(window_w, MAX(yes_button->get_width() + no_button->get_width() + 2, width) + WIN_BORDER * 4);
	}

	Widget_Label *label = new Widget_Label(text, window_w - WIN_BORDER*4, TTH_GLOBALS->bold_font);
	label->set_centre_x(true);
	label->set_padding(WIN_BORDER);

	yes_button->set_centre_x(true);
	yes_button->set_padding_right(2);

	no_button->set_centre_x(true);
	no_button->set_padding_left(20);

	Widget *button_container = new Widget(1.0f, yes_button->get_height());
	button_container->set_float_bottom(true);

	Widget_Window *window = new Widget_Window(window_w, line_height * num_lines + yes_button->get_height() + WIN_BORDER*4);
	window->set_centre_x(true);
	window->set_centre_y(true);
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(WIN_BORDER);
	pad->set_parent(window);

	label->set_parent(pad);
	button_container->set_parent(pad);
	yes_button->set_parent(button_container);
	no_button->set_parent(button_container);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(yes_button);
}

Yes_No_GUI::~Yes_No_GUI()
{
	delete drawing_hook;
}

void Yes_No_GUI::update()
{
	TTH_GUI::update();

	if (transitioning_in || transitioning_out) {
		return;
	}

	if (yes_button->pressed()) {
		if (callback) {
			wedge::Yes_No_GUI_Callback_Data d;
			d.choice = true;
			d.cancelled = false;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
	else if (no_button->pressed()) {
		if (callback) {
			wedge::Yes_No_GUI_Callback_Data d;
			d.choice = false;
			d.cancelled = false;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
}

void Yes_No_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if (escape_cancels && (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)
	)) {
		TTH_GLOBALS->button->play(false);
		if (callback) {
			wedge::Yes_No_GUI_Callback_Data d;
			d.choice = false;
			d.cancelled = true;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
	else {
		GUI::handle_event(event);
	}
}

void Yes_No_GUI::set_selected(bool yes_no)
{
	if (yes_no) {
		gui->set_focus(yes_button);
	}
	else {
		gui->set_focus(no_button);
	}
}

void Yes_No_GUI::hook_omnipresent(bool hook, bool last)
{
	_hook_omnipresent = hook;
	hook_draw_last = last;
}

void Yes_No_GUI::draw_back()
{
	if (_hook_omnipresent == false) {
		gui::GUI::draw_back();
		return;
	}
}

void Yes_No_GUI::draw()
{
	if (_hook_omnipresent == false) {
		gui::GUI::draw();
		return;
	}
}

void Yes_No_GUI::draw_fore()
{
	if (_hook_omnipresent == false) {
		gui::GUI::draw_fore();
		return;
	}

	int mod = count % 2;
	count++;
	if (mod == 0) {
		if (drawing_hook == NULL) {
			drawing_hook = new GUI_Drawing_Hook_Step(this, hook_draw_last);
		}
		drawing_hook->hook();
		return;
	}

	gfx::Font::end_batches();

	gui::GUI::draw_back();
	gui::GUI::draw();
	gui::GUI::draw_fore();
}

void Yes_No_GUI::set_b1_text(std::string b1_text)
{
	yes_button->set_text(b1_text);
}

void Yes_No_GUI::set_b2_text(std::string b2_text)
{
	no_button->set_text(b2_text);
}

bool Yes_No_GUI::get_escape_cancels()
{
	return escape_cancels;
}

//--

Multiple_Choice_GUI::Multiple_Choice_GUI(bool tint_screen, std::string caption, std::vector<std::string> choices, int escape_choice, util::Callback callback, void *callback_data, int lines_to_show, int width, bool shrink_to_fit, Widget_List *custom_list, bool draw_window, int win_border, bool show_combo_tip) :
	caption(caption),
	callback(callback),
	callback_data(callback_data),
	exit_menu(false),
	escape_choice(escape_choice),
	lines_to_show(lines_to_show),
	shrink_to_fit(shrink_to_fit),
	win_border(win_border)
{
	transition_is_shrink = true;
	transition_duration = 200;
	started_transition_timer = false;

	int w = width;
	int h;
	int num_lines;

	get_height(w, h, num_lines);

	if (shrink_to_fit) {
		for (size_t i = 0; i < choices.size(); i++) {
			int choice_w = shim::font->get_text_width(choices[i]) + 1;
			w = MAX(w+1, choice_w);
		}
		w += win_border*2;
	}

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	
	if (tint_screen) {
		SDL_Colour background_colour = shim::black;
		background_colour.r *= 0.5f;
		background_colour.g *= 0.5f;
		background_colour.b *= 0.5f;
		background_colour.a *= 0.5f;
		modal_main_widget->set_background_colour(background_colour);
	}

	if (draw_window) {
		window = new Widget_Window(w + win_border*2 + GLOBALS->cursor->get_current_image()->size.w, h + win_border * (caption == "" ? 0 : 2));
	}
	else {
		window = new Widget(w + win_border*2 + GLOBALS->cursor->get_current_image()->size.w, h + win_border * (caption == "" ? 0 : 2));
	}
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(win_border);
	pad->set_parent(window);

	if (caption != "") {
		caption_label = new Widget_Label(caption, w - win_border*2, TTH_GLOBALS->bold_font);
		caption_label->set_centre_x(true);
		caption_label->set_padding(win_border);
		caption_label->set_parent(pad);
	}
	else {
		caption_label = NULL;
	}

	if (custom_list != NULL) {
		list = custom_list;
		list->set_width(1.0f);
		list->set_height(lines_to_show * TTH_GLOBALS->bold_font->get_height() + 4); // Widget_List constructor adds + 4
	}
	else {
		list = new Widget_List(1.0f, lines_to_show * TTH_GLOBALS->bold_font->get_height());
	}
	list->set_items(choices);
	if (show_combo_tip) {
		std::vector<std::string> tips;
		tips.push_back(GLOBALS->game_t->translate(396)/* Originally: Select this! */);
		list->set_tips(tips);
	}
	//list->set_float_bottom(true);
	list->set_padding_left(GLOBALS->cursor->get_current_image()->size.w);
	if (caption != "") {
		//list->set_padding_top(win_border+2);
		list->set_padding_top(num_lines * TTH_GLOBALS->bold_font->get_height() + win_border + 2);
	}
	else {
		//list->set_padding_top(win_border);
	}
	list->set_parent(pad);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(list);
}

Multiple_Choice_GUI::~Multiple_Choice_GUI()
{
}

void Multiple_Choice_GUI::update()
{
	TTH_GUI::update();

	if (transitioning_in || transitioning_out) {
		return;
	}

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		Callback_Data data;
		data.choice = pressed;
		data.userdata = callback_data;
		callback((void *)&data);
		exit();
	}
}

void Multiple_Choice_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if ((escape_choice >= 0 || escape_choice == -3) && ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back))) {
		TTH_GLOBALS->button->play(false);
		Callback_Data data;
		data.choice = escape_choice;
		data.userdata = callback_data;
		callback((void *)&data);
		exit();
	}
	if (escape_choice == -2 && ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back))) {
		TTH_GLOBALS->button->play(false);
		exit();
	}
	else {
		GUI::handle_event(event);
	}
}

void Multiple_Choice_GUI::resize(util::Size<int> new_size)
{
	assert(0 && "OVERRIDE THIS");
}

void Multiple_Choice_GUI::get_height(int &w, int &h, int &num_lines)
{
	bool full;
	int width;

	// Get caption # lines
	if (caption == "") {
		num_lines = 0;
		h = shim::font->get_height() * lines_to_show;
	}
	else {
		TTH_GLOBALS->bold_font->draw_wrapped(shim::palette[20], caption, util::Point<float>(0, 0), w, TTH_GLOBALS->bold_font->get_height(), -1, 0, 0, true, full, num_lines, width);
		if (shrink_to_fit) {
			w = MIN(w, width);
		}
		h = TTH_GLOBALS->bold_font->get_height() * num_lines + lines_to_show * shim::font->get_height() + 2;
	}
}

void Multiple_Choice_GUI::set_selected(int index)
{
	list->set_selected(index);
}

void Multiple_Choice_GUI::set_wrap(bool wrap)
{
	list->set_wrap(wrap);
}

Widget_List *Multiple_Choice_GUI::get_list()
{
	return list;
}

int Multiple_Choice_GUI::get_escape_choice()
{
	return escape_choice;
}

//--

Positioned_Multiple_Choice_GUI::Positioned_Multiple_Choice_GUI(bool tint_screen, std::string caption, std::vector<std::string> choices, int escape_choice, int horiz_pos/*-1, 0, 1=left, center, right*/, int vert_pos/*same as horiz_pos*/, int padding_left, int padding_right, int padding_top, int padding_bottom, float screen_margin_x, float screen_margin_y, util::Callback callback, void *callback_data, int lines_to_show, int width, bool shrink_to_fit, float width_ratio, Widget_List *custom_list, bool draw_window, int win_border, bool show_combo_tip) :
	Multiple_Choice_GUI(tint_screen, caption, choices, escape_choice, callback, callback_data, lines_to_show, width, shrink_to_fit, custom_list, draw_window, win_border, show_combo_tip),
	horiz_pos(horiz_pos),
	vert_pos(vert_pos),
	padding_left(padding_left),
	padding_right(padding_right),
	padding_top(padding_top),
	padding_bottom(padding_bottom),
	screen_margin_x(screen_margin_x),
	screen_margin_y(screen_margin_y),
	width_ratio(width_ratio),
	width(width)
{
}

Positioned_Multiple_Choice_GUI::~Positioned_Multiple_Choice_GUI()
{
}

void Positioned_Multiple_Choice_GUI::resize(util::Size<int> new_size)
{
	int margin_x = new_size.w * screen_margin_x;
	int margin_y = new_size.h * screen_margin_y;

	int pad_left = padding_left;
	int pad_right = padding_right;
	int pad_top = padding_top;
	int pad_bottom = padding_bottom;

	if (horiz_pos == -1) {
		window->set_float_right(false);
		window->set_centre_x(false);
		pad_left += margin_x;
	}
	else if (horiz_pos == 0) {
		window->set_float_right(false);
		window->set_centre_x(true);
	}
	else {
		window->set_float_right(true);
		window->set_centre_x(false);
		pad_right += margin_x;
	}

	if (vert_pos == -1) {
		window->set_float_bottom(false);
		window->set_centre_y(false);
		pad_top += margin_y;
	}
	else if (vert_pos == 0) {
		window->set_float_bottom(false);
		window->set_centre_y(true);
	}
	else {
		window->set_float_bottom(true);
		window->set_centre_y(false);
		pad_bottom += margin_y;
	}

	window->set_padding_left(pad_left);
	window->set_padding_right(pad_right);
	window->set_padding_top(pad_top);
	window->set_padding_bottom(pad_bottom);

	int w = width;
	int h, num_lines;
	get_height(w, h, num_lines);
	if (shrink_to_fit) {
		std::vector<std::string> choices = list->get_items();
		for (size_t i = 0; i < choices.size(); i++) {
			int choice_w = shim::font->get_text_width(choices[i]) + 1 + WIN_BORDER*2; // list adds WIN_BORDER*2 (don't use lowercase win_border here)
			w = MAX(w, choice_w);
		}
		w += win_border*2;
		//w += GLOBALS->cursor->get_current_image()->size.w;
	}
	else {
		w = new_size.w * width_ratio;
	}
	int pad = list->get_padding_left();
	window->set_width(w + pad);
	window->set_height(h+win_border*(caption_label == NULL ? 2 : 4));
	list->set_width(w-win_border*2);
	list->set_height(lines_to_show * TTH_GLOBALS->bold_font->get_height() + 4); // Widget_List constructor adds + 4
	if (caption_label) {
		caption_label->set_max_width(w-win_border*2);
	}

	gui->resize(new_size.w, new_size.h);
}

void Positioned_Multiple_Choice_GUI::set_padding(int padding_left, int padding_right, int padding_top, int padding_bottom)
{
	this->padding_left = padding_left;
	this->padding_right = padding_right;
	this->padding_top = padding_top;
	this->padding_bottom = padding_bottom;
}

//--

Notification_GUI::Notification_GUI(std::string text, util::Callback callback, void *callback_data, bool shrink_to_fit) :
	callback(callback),
	callback_data(callback_data),
	_hook_omnipresent(false),
	hook_draw_last(false),
	count(0),
	drawing_hook(NULL)
{
	transition_is_shrink = true;
	transition_duration = 200;
	started_transition_timer = false;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	SDL_Colour background_colour = shim::black;
	background_colour.r *= 0.5f;
	background_colour.g *= 0.5f;
	background_colour.b *= 0.5f;
	background_colour.a *= 0.5f;
	modal_main_widget->set_background_colour(background_colour);

	int window_w = 200;

	bool full;
	int num_lines, width;
	int line_height = TTH_GLOBALS->bold_font->get_height() + 1;
	TTH_GLOBALS->bold_font->draw_wrapped(shim::white, text, util::Point<int>(0, 0), window_w - WIN_BORDER*4, line_height, -1, -1, 0, true, full, num_lines, width);
	
	ok_button = new Widget_Text_Button(GLOBALS->game_t->translate(69)/* Originally: OK */);

	if (shrink_to_fit) {
		window_w = MIN(window_w, MAX(ok_button->get_width(), width) + WIN_BORDER * 4);
	}
	
	Widget_Label *label = new Widget_Label(text, window_w - WIN_BORDER*4, TTH_GLOBALS->bold_font);
	label->set_padding(WIN_BORDER);
	label->set_centre_x(true);

	ok_button->set_centre_x(true);
	ok_button->set_float_bottom(true);

	Widget_Window *window = new Widget_Window(window_w, line_height * num_lines + ok_button->get_height() + WIN_BORDER*4);
	window->set_centre_x(true);
	window->set_centre_y(true);
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(WIN_BORDER);
	pad->set_parent(window);
	
	label->set_parent(pad);
	ok_button->set_parent(pad);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(ok_button);
}

Notification_GUI::~Notification_GUI()
{
	delete drawing_hook;
}

void Notification_GUI::update()
{
	TTH_GUI::update();

	if (ok_button->pressed()) {
		if (callback) {
			Callback_Data d;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
}

void Notification_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) ||
		(event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)
	) {
		TTH_GLOBALS->button->play(false);
		if (callback) {
			Callback_Data d;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
	else {
		gui::GUI::handle_event(event);
	}
}

void Notification_GUI::draw_back()
{
	if (_hook_omnipresent == false) {
		gui::GUI::draw_back();
		return;
	}
}

void Notification_GUI::draw()
{
	if (_hook_omnipresent == false) {
		gui::GUI::draw();
		return;
	}
}

void Notification_GUI::draw_fore()
{
	if (_hook_omnipresent == false) {
		gui::GUI::draw_fore();
		return;
	}

	int mod = count % 2;
	count++;
	if (mod == 0) {
		if (drawing_hook == NULL) {
			drawing_hook = new GUI_Drawing_Hook_Step(this, hook_draw_last);
		}
		drawing_hook->hook();
		return;
	}

	gfx::Font::end_batches();

	gui::GUI::draw_back();
	gui::GUI::draw();
	gui::GUI::draw_fore();
}

void Notification_GUI::hook_omnipresent(bool hook, bool last)
{
	_hook_omnipresent = hook;
	hook_draw_last = last;
}

//--

#ifdef ANDROID
#include <jni.h>

static SDL_mutex *android_mutex;
static SDL_cond *android_cond;
static bool android_bool;
static bool achievements_bool;

extern "C" {
	JNIEXPORT void JNICALL Java_com_b1stable_tth_TTH_1Activity_resume_1after_1showing_1license
	  (JNIEnv *env, jobject obj)
	{
		SDL_LockMutex(android_mutex);
		android_bool = true;
		SDL_CondSignal(android_cond);
		SDL_UnlockMutex(android_mutex);
	}

	JNIEXPORT void JNICALL Java_com_b1stable_tth_TTH_1Activity_resume_1after_1showing_1manual
	  (JNIEnv *env, jobject obj)
	{
		SDL_LockMutex(android_mutex);
		android_bool = true;
		SDL_CondSignal(android_cond);
		SDL_UnlockMutex(android_mutex);
	}

	JNIEXPORT void JNICALL Java_com_b1stable_tth_TTH_1Activity_resume_1after_1showing_1achievements
	  (JNIEnv *env, jobject obj)
	{
		achievements_bool = true;
	}
}

static int get_show_license_result()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "getShowLicenseResult", "()I");

	int result = env->CallIntMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return result;
}

#if 0
static int get_show_manual_result()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "getShowManualResult", "()I");

	int result = env->CallIntMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	return result;
}
#endif
#endif

void video_mode_callback(void *data)
{
	Multiple_Choice_GUI::Callback_Data *d = static_cast<Multiple_Choice_GUI::Callback_Data *>(data);
	if (d->choice >= 0) {
		Settings_GUI *s = static_cast<Settings_GUI *>(d->userdata);
		Widget_Settings_List *list = s->get_list();
		Video_Settings_GUI *gui = new Video_Settings_GUI(d->choice == 0, list->get_top(), list->get_selected());
		shim::guis.push_back(gui);
		s->exit();
	}
}

#ifdef __APPLE__
static std::string macos_filename;

void macos_fullscreen_callback(void *data)
{
	wedge::Yes_No_GUI_Callback_Data *d = static_cast<wedge::Yes_No_GUI_Callback_Data *>(data);
	if (d->cancelled == false && d->choice == true) {
		delete_shim_args();
		set_shim_args(false, true, false);
		gfx::restart(SCR_W, SCR_H, false, 1280, 720);
		
		SDL_Delay(250);

		util::open_with_system(macos_filename);
	}
}
#endif

void Settings_GUI::static_start()
{
#ifdef ANDROID
	android_bool = true;
#endif
}

Settings_GUI::Settings_GUI(bool disable_language)
{
	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	TGUI_Widget *container = new TGUI_Widget();
	container->set_centre_x(true);
	container->set_centre_y(true);
	container->set_padding_top(10);
	container->set_parent(modal_main_widget);

	std::vector<std::string> v;
#if !defined ANDROID && !defined IOS
	v.push_back(GLOBALS->game_t->translate(84)/* Originally: Video */);
#endif
	v.push_back(GLOBALS->game_t->translate(614)/* Originally: Audio */);
#if !defined IOS
	v.push_back(GLOBALS->game_t->translate(86)/* Originally: Keyboard */);
#endif
	v.push_back(GLOBALS->game_t->translate(87)/* Originally: Gamepad */);
	v.push_back(GLOBALS->game_t->translate(88)/* Originally: Miscellaneous */);
#ifdef GOOGLE_PLAY
	v.push_back(GLOBALS->game_t->translate(605)/* Originally: Achievements */);
#endif
	v.push_back(GLOBALS->game_t->translate(92)/* Originally: Licenses */);
#if (defined __APPLE__ && !defined STEAMWORKS)
	v.push_back(GLOBALS->game_t->translate(90)/* Originally: Cloud Saves */);
#endif
		
	list = new Widget_Settings_List(4, "ui/settings_row.tga");
	list->set_items(v);
	list->set_parent(container);
	// Language menu is not available in Steam version

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(list);
}

Settings_GUI::~Settings_GUI()
{
}

void Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	TTH_GUI::handle_event(event);

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		GLOBALS->button->play(false);
		if (AREA) {
			auto gui = new Main_Menu_GUI(5);
			shim::guis.push_back(gui);
			exit();
		}
		else {
			GLOBALS->add_title_gui(true);
			static_cast<Title_GUI *>(shim::guis.back())->focus_button(1);
			static_cast<Title_GUI *>(shim::guis.back())->set_transition(true);
			exit();
		}
	}
}

void Settings_GUI::update()
{
	TTH_GUI::update();

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		pressed++; // skip Language

#if defined ANDROID
		// Skip Video
		if (pressed >= 1) {
			pressed++;
		}
#endif

		if (pressed == 0) {
			Language_GUI *gui = new Language_GUI();
			shim::guis.push_back(gui);
			exit();
		}
		else if (pressed == 1) {
			std::vector<std::string> choices;
			choices.push_back(GLOBALS->game_t->translate(115)/* Originally: Fullscreen */);
			choices.push_back(GLOBALS->game_t->translate(99)/* Originally: Windowed */);
			Positioned_Multiple_Choice_GUI *gui = new Positioned_Multiple_Choice_GUI(true, GLOBALS->game_t->translate(100)/* Originally: Select Mode */, choices, -2, 0, 0, 0, 0, 0, 0, 0, 0, video_mode_callback, this, 2);
			gui->set_wrap(true);
			gui->set_selected(0);
			gui->resize(shim::screen_size); // Multiple choice guis always need a resize right away
			shim::guis.push_back(gui);
		}
		else if (pressed == 2) {
			Audio_Settings_GUI *gui = new Audio_Settings_GUI(list->get_top(), list->get_selected());
			shim::guis.push_back(gui);
			exit();
		}
		else if (pressed == 3) {
			Controls_Settings_GUI *gui = new Controls_Settings_GUI(true, list->get_top(), list->get_selected());
			shim::guis.push_back(gui);
			exit();
		}
		else if (pressed == 4) {
			Controls_Settings_GUI *gui = new Controls_Settings_GUI(false, list->get_top(), list->get_selected());
			shim::guis.push_back(gui);
			exit();
		}
		else if (pressed == 5) {
			Miscellaneous_Settings_GUI *gui = new Miscellaneous_Settings_GUI(list->get_top(), list->get_selected());
			shim::guis.push_back(gui);
			exit();
		}
#ifdef GOOGLE_PLAY
		else if (pressed == 6) {
			achievements_bool = false;
			SDL_Colour black;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			black.a = 255;
			gfx::clear(black);
			gfx::flip();
			if (util::show_achievements() == false) {
				Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(531)/* Originally: An error occurred! */);
				shim::guis.push_back(gui);
			}
			else {
				while (achievements_bool == false) {
					SDL_Delay(10);
				}
				gfx::resize_window(shim::real_screen_size.w, shim::real_screen_size.h);
			}
		}
		else if (pressed == 7) {
#else
		else if (pressed == 6) {
#endif
#ifdef ANDROID
			if (android_bool == true) {
				android_bool = false;

				int result = 1;

				android_mutex = SDL_CreateMutex();

				if (android_mutex) {
					android_cond = SDL_CreateCond();

					if (android_cond) {
						JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
						jobject activity = (jobject)SDL_AndroidGetActivity();
						jclass clazz(env->GetObjectClass(activity));

						jmethodID method_id = env->GetMethodID(clazz, "showLicense", "()V");

						env->CallVoidMethod(activity, method_id);

						env->DeleteLocalRef(activity);
						env->DeleteLocalRef(clazz);

						float old_volume = shim::music->get_master_volume();

						std::string music_backup = shim::music->get_name();
						audio::stop_music();

						SDL_LockMutex(android_mutex);
						while (!android_bool) {
							SDL_CondWait(android_cond, android_mutex);
						}
						SDL_UnlockMutex(android_mutex);

						SDL_DestroyCond(android_cond);

						result = get_show_license_result();

						shim::music = new audio::MML(music_backup);
						shim::music->play(old_volume, true);
					}

					SDL_DestroyMutex(android_mutex);
				}

				if (result == 1) {
					Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(322)/* Originally: Displaying the licenses failed! */);
					shim::guis.push_back(gui);
				}
			}
#elif defined IOS
			shim::music->pause();
			bool ret = ios_show_license();
			if (ret == false) {
				Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(322)/* Originally: Displaying the licenses failed! */);
				shim::guis.push_back(gui);
			}
#else
#if defined __APPLE__
			char *base = SDL_GetBasePath();
			std::string filename = std::string(base) + "3rd_party.html";
			SDL_free(base);
#elif defined __linux__
			std::string argv0 = shim::argv[0];
			size_t slash = argv0.rfind("/");
			std::string filename = argv0.substr(0, slash) + "/" + "3rd_party.html";
			filename = std::string("\"") + filename + std::string("\"");
#else
			std::string filename = "3rd_party.html";
#endif
#ifdef __APPLE__
			if (noo::gfx::internal::gfx_context.fullscreen) {
				macos_filename = filename;
				Yes_No_GUI *gui = new Yes_No_GUI(GLOBALS->game_t->translate(527)/* Originally: Exit fullscreen mode to view licenses? */, true, macos_fullscreen_callback);
				shim::guis.push_back(gui);
			}
			else {
				util::open_with_system(filename);
			}
#else
			util::open_with_system(filename);
#endif
#endif
		}
	}
}

void Settings_GUI::draw_back()
{
	GUI::draw_back();

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), shim::screen_size);
}

void Settings_GUI::draw()
{
	TTH_GUI::draw();
	std::string text = GLOBALS->game_t->translate(50)/* Originally: Settings */;
	SDL_Colour c1;
	c1.r = 253;
	c1.g = 77;
	c1.b = 79;
	c1.a = 255;
	SDL_Colour c2 = shim::black;
	TTH_GLOBALS->bold_font->enable_shadow(c1, gfx::Font::FULL_SHADOW);
	TTH_GLOBALS->bold_font->draw(c2, text, util::Point<int>(shim::screen_size.w/2-shim::font->get_text_width(text)/2, 5));
	TTH_GLOBALS->bold_font->disable_shadow();
	GUI::draw();
}

void Settings_GUI::set_top_selected(int top, int selected)
{
	list->set_top(top);
	list->set_selected(selected);
}

Widget_Settings_List *Settings_GUI::get_list()
{
	return list;
}

//--

void change_language_callback(void *data)
{
	Notification_GUI::Callback_Data *d = static_cast<Notification_GUI::Callback_Data *>(data);
	Language_GUI *gui = static_cast<Language_GUI *>(d->userdata);
	gui->done();
}

Language_GUI::Language_GUI()
{
	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);

	TGUI_Widget *container = new TGUI_Widget();
	container->set_centre_x(true);
	container->set_centre_y(true);
	container->set_padding_bottom(5);
	container->set_parent(modal_main_widget);

	std::vector<std::string> v;
	v.push_back(GLOBALS->game_t->translate(93)/* Originally: English */);
	v.push_back(GLOBALS->game_t->translate(94)/* Originally: French */);
		
	list = new Widget_Settings_List(int(MIN(4, v.size())), "ui/settings_row.tga");
	list->set_items(v);
	list->set_parent(container);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(list);
}

Language_GUI::~Language_GUI()
{
}

void Language_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	TTH_GUI::handle_event(event);

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		GLOBALS->button->play(false);
		Settings_GUI *gui = new Settings_GUI(false);
		shim::guis.push_back(gui);
		exit();
	}
}

void Language_GUI::update()
{
	TTH_GUI::update();

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		GLOBALS->language = GLOBALS->english_game_t->translate(GLOBALS->game_t->get_id(list->get_item(pressed)));
		save_settings();
		GLOBALS->load_translation();
		Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(95)/* Originally: This menu will now exit. */, change_language_callback, this);
		shim::guis.push_back(gui);
	}
}

void Language_GUI::draw_back()
{
	GUI::draw_back();

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), shim::screen_size);
}

void Language_GUI::draw()
{
	TTH_GUI::draw();
	std::string text = GLOBALS->game_t->translate(83)/* Originally: Language */;
	SDL_Colour c1;
	c1.r = 253;
	c1.g = 77;
	c1.b = 79;
	c1.a = 255;
	SDL_Colour c2 = shim::black;
	TTH_GLOBALS->bold_font->enable_shadow(c1, gfx::Font::FULL_SHADOW);
	TTH_GLOBALS->bold_font->draw(c2, text, util::Point<int>(shim::screen_size.w/2-shim::font->get_text_width(text)/2, 5));
	TTH_GLOBALS->bold_font->disable_shadow();
	GUI::draw();
}

void Language_GUI::done()
{
	GLOBALS->add_title_gui(true);
	static_cast<Title_GUI *>(shim::guis.back())->set_transition(true);
	exit();
}

//--

Video_Settings_GUI::Video_Settings_GUI(bool fullscreen, int settings_top, int settings_selected) :
	fs(fullscreen),
	settings_top(settings_top),
	settings_selected(settings_selected)
{
	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	list = new Widget_Settings_List(4, "ui/settings_row.tga");
	list->set_centre_x(true);
	list->set_centre_y(true);
	list->set_padding_top(int(TTH_GLOBALS->bold_font->get_height()));
	list->set_parent(modal_main_widget);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
	
	set_text();
}

Video_Settings_GUI::~Video_Settings_GUI()
{
}

void Video_Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	TTH_GUI::handle_event(event);

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		GLOBALS->button->play(false);
		Settings_GUI *gui = new Settings_GUI(false);
		gui->set_top_selected(settings_top, settings_selected);
		shim::guis.push_back(gui);
		exit();
	}
}

void Video_Settings_GUI::update()
{
	TTH_GUI::update();

	if (transitioning_in || transitioning_out) {
		return;
	}

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		int top = list->get_top();
		delete_shim_args();
		if (fs) {
			set_shim_args(false, false, true);
		}
		else {
			set_shim_args(false, true, false);
		}
		gfx::restart(SCR_W, SCR_H, false, modes[pressed].w, modes[pressed].h);
		list->set_selected(pressed);
		list->set_top(top);
		windowed = !fs;
		fullscreen = fs;
		screen_w = modes[pressed].w;
		screen_h = modes[pressed].h;
		save_settings();
#ifndef ANDROID
		gfx::delete_mouse_cursors();
		gfx::create_mouse_cursors();
#endif
	}
}

void Video_Settings_GUI::draw_back()
{
	GUI::draw_back();

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), shim::screen_size);
}

void Video_Settings_GUI::draw()
{
	TTH_GUI::draw();
	
	SDL_Colour c1;
	c1.r = 253;
	c1.g = 77;
	c1.b = 79;
	c1.a = 255;
	SDL_Colour c2 = shim::black;
	TTH_GLOBALS->bold_font->enable_shadow(c1, gfx::Font::FULL_SHADOW);
	if (fs) {
		TTH_GLOBALS->bold_font->draw(c2, GLOBALS->game_t->translate(96)/* Originally: Screen Resolution */, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	}
	else {
		TTH_GLOBALS->bold_font->draw(c2, GLOBALS->game_t->translate(97)/* Originally: Window Size */, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	}
	TTH_GLOBALS->bold_font->disable_shadow();
	
	GUI::draw();
}

void Video_Settings_GUI::set_text()
{
	modes.clear();

	std::vector<std::string> items;

	if (fs) {
		modes = gfx::get_supported_video_modes();

		std::vector< util::Size<int> >::iterator it;
		for (it = modes.begin(); it != modes.end();) {
			util::Size<int> size = *it;
			if (size.w < 1280 || size.h < 720) {
				it = modes.erase(it);
			}
			else {
				it++;
			}
		}
	}
	else {
		modes.push_back(util::Size<int>(5120, 2880));
		modes.push_back(util::Size<int>(3840, 2160));
		modes.push_back(util::Size<int>(2560, 1440));
		modes.push_back(util::Size<int>(1920, 1080));
		modes.push_back(util::Size<int>(1440, 810));
		modes.push_back(util::Size<int>(1280, 720));
		modes.push_back(util::Size<int>(960, 540));

		// New method (doesn't work well?)
		/*
		util::Size<int> desktop_resolution = gfx::get_desktop_resolution();

		int max = MAX(desktop_resolution.w, desktop_resolution.h);
		int min = MIN(desktop_resolution.w, desktop_resolution.h);

		while (modes[0].w+80 > max || modes[0].h+80 > min) {
			modes.erase(modes.begin());
		}
		*/

		// Old method - doesn't work on Windows 10 tablet with portrait mode
		int max = gfx::get_max_comfortable_scale(util::Size<int>(SCR_W, SCR_H));

		while (modes[0].w > max*SCR_W || modes[0].h > max*SCR_H) {
			modes.erase(modes.begin());
		}
	}

	for (size_t i = 0; i < modes.size(); i++) {
		items.push_back(util::itos(modes[i].w) + "x" + util::itos(modes[i].h));
	}

	list->set_items(items);

	set_selected();
}

void Video_Settings_GUI::found_device()
{
	TTH_GUI::found_device();
	set_text();
}

void Video_Settings_GUI::set_selected()
{
	int index = 0;
	bool same = gfx::is_real_fullscreen() == fs;
	if (same) {
		for (size_t i = 0; i < modes.size(); i++) {
			if (modes[i].w == shim::real_screen_size.w && modes[i].h == shim::real_screen_size.h) {
				index = int(i);
				break;
			}
		}
	}
	list->set_selected(index);
	int vr = MIN(list->visible_rows(), (int)modes.size());
	list->set_top(MIN((int)modes.size()-vr, MAX(0, index-vr/2)));
}

//--

Audio_Settings_GUI::Audio_Settings_GUI(int settings_top, int settings_selected) :
	settings_top(settings_top),
	settings_selected(settings_selected)
{
	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	Widget *container = new Widget();
	container->set_centre_x(true);
	container->set_centre_y(true);
	container->set_parent(modal_main_widget);

	sound_on_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(612)/* Originally: Audio On */, music_volume <= 0.05f ? false : true);
	sound_on_checkbox->set_centre_x(true);
	sound_on_checkbox->set_parent(container);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Audio_Settings_GUI::~Audio_Settings_GUI()
{
}

void Audio_Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	bool tab = false;
	bool esc = false;
	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		esc = true;
	}

	if (tab || esc) {
		save_settings();
	}

	if (tab) {
		AREA->end_menu();
		exit();
	}
	else if (esc) {
		TTH_GLOBALS->button->play(false);
		Settings_GUI *gui = new Settings_GUI(false);
		gui->set_top_selected(settings_top, settings_selected);
		shim::guis.push_back(gui);
		exit();
	}
	else {
		bool sound_on_checked = sound_on_checkbox->is_checked();
		TTH_GUI::handle_event(event);
		if (sound_on_checkbox->is_checked() != sound_on_checked) {
			sound_on_checked = !sound_on_checked;
			if (sound_on_checked) {
				sfx_volume = 1.0f;
				music_volume = 1.0f;
			}
			else {
				sfx_volume = 0.0f;
				music_volume = 0.0f;
			}
			shim::sfx_volume = sfx_volume * sfx_amp;
			shim::music_volume = music_volume * music_amp;
			bool paused = AREA != NULL && AREA->is_paused();
			float maybe_paused = paused ? 0.333f : 1.0f;
			shim::music->set_master_volume(shim::music_volume * maybe_paused);
		}
	}
}

void Audio_Settings_GUI::draw_back()
{
	GUI::draw_back();

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), shim::screen_size);
}

void Audio_Settings_GUI::draw()
{
	TTH_GUI::draw();

	SDL_Colour c1;
	c1.r = 253;
	c1.g = 77;
	c1.b = 79;
	c1.a = 255;
	SDL_Colour c2 = shim::black;
	TTH_GLOBALS->bold_font->enable_shadow(c1, gfx::Font::FULL_SHADOW);
	TTH_GLOBALS->bold_font->draw(c2, GLOBALS->game_t->translate(613)/* Originally: Audio Settings */, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	TTH_GLOBALS->bold_font->disable_shadow();
	
	GUI::draw();
}

void Audio_Settings_GUI::update()
{
	TTH_GUI::update();
}

//--

#if 0
Audio_Settings_GUI::Audio_Settings_GUI(int settings_top, int settings_selected) :
	settings_top(settings_top),
	settings_selected(settings_selected)
{
	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	Widget *container = new Widget();
	container->set_centre_x(true);
	container->set_centre_y(true);
	container->set_padding_top(10);
	container->set_parent(modal_main_widget);

	Widget *left = new Widget(TGUI_Widget::FIT_Y, 50);
	left->set_centre_x(true);
	left->set_padding_right(5);
	left->set_parent(container);

	Widget_Label *sfx_label = new Widget_Label(GLOBALS->game_t->translate(101)/* Originally: Sounds */, -1);
	sfx_label->set_centre_x(true);
	sfx_label->set_parent(left);
	sfx_label->set_shadow_colour(shim::palette[19]);
	sfx_label->set_shadow_type(gfx::Font::FULL_SHADOW);

	sfx_slider = new Widget_Vertical_Slider(51, 51, sfx_volume * 50);
	sfx_slider->set_clear_float_x(true);
	sfx_slider->set_centre_x(true);
	sfx_slider->set_padding_top(15);
	sfx_slider->set_parent(left);

	Widget *right = new Widget(TGUI_Widget::FIT_Y, 50);
	right->set_centre_x(true);
	right->set_padding_left(5);
	right->set_parent(container);

	Widget_Label *music_label = new Widget_Label(GLOBALS->game_t->translate(102)/* Originally: Music */, -1);
	music_label->set_centre_x(true);
	music_label->set_parent(right);
	music_label->set_shadow_colour(shim::palette[19]);
	music_label->set_shadow_type(gfx::Font::FULL_SHADOW);

	music_slider = new Widget_Vertical_Slider(51, 51, music_volume * 50);
	music_slider->set_clear_float_x(true);
	music_slider->set_centre_x(true);
	music_slider->set_padding_top(15);
	music_slider->set_parent(right);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Audio_Settings_GUI::~Audio_Settings_GUI()
{
}

void Audio_Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	bool tab = false;
	bool esc = false;
	if (AREA && ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_menu) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_menu))) {
		tab = true;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		esc = true;
	}

	if (tab) {
		save_settings();
		AREA->end_menu();
		exit();
	}
	else if (esc) {
		save_settings();
		GLOBALS->button->play(false);
		Settings_GUI *gui = new Settings_GUI(false);
		gui->set_top_selected(settings_top, settings_selected);
		shim::guis.push_back(gui);
		exit();
	}
	else {
		TTH_GUI::handle_event(event);
	}
}

void Audio_Settings_GUI::update()
{
	TTH_GUI::update();

	if (transitioning_in || transitioning_out) {
		return;
	}

	sfx_volume = sfx_slider->get_value() / 50.0f;
	shim::sfx_volume = sfx_volume * sfx_amp;
	float vol = music_slider->get_value() / 50.0f;
	if (vol != music_volume) {
		music_volume = vol;
		shim::music_volume = music_volume * music_amp;
		bool paused = AREA != NULL && AREA->is_paused();
		float maybe_paused = paused ? 0.333f : 1.0f;
		shim::music->set_master_volume(shim::music_volume * maybe_paused);
	}
}

void Audio_Settings_GUI::draw_back()
{
	GUI::draw_back();

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), shim::screen_size);
}

void Audio_Settings_GUI::draw()
{
	TTH_GUI::draw();

	SDL_Colour c1;
	c1.r = 253;
	c1.g = 77;
	c1.b = 79;
	c1.a = 255;
	SDL_Colour c2 = shim::black;
	TTH_GLOBALS->bold_font->enable_shadow(c1, gfx::Font::FULL_SHADOW);
	TTH_GLOBALS->bold_font->draw(c2, GLOBALS->game_t->translate(103)/* Originally: Audio Volumes */, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	TTH_GLOBALS->bold_font->disable_shadow();

	GUI::draw();
}
#endif

//--

static void apply_callback(void *data)
{
	wedge::Yes_No_GUI_Callback_Data *d = static_cast<wedge::Yes_No_GUI_Callback_Data *>(data);
	Controls_Settings_GUI *gui = static_cast<Controls_Settings_GUI *>(d->userdata);
	if (d->cancelled == false && d->choice) {
		gui->quit(true);
	}
	else if (d->cancelled == false) {
		gui->quit(false);
	}
}

static void apply2_callback(void *data)
{
	wedge::Yes_No_GUI_Callback_Data *d = static_cast<wedge::Yes_No_GUI_Callback_Data *>(data);
	Controls_Settings_GUI *gui = static_cast<Controls_Settings_GUI *>(d->userdata);
	if (d->cancelled == false && d->choice) {
		gui->quit2(true);
	}
	else if (d->cancelled == false) {
		gui->quit2(false);
	}
}

static void duplicates_callback(void *data)
{
	wedge::Yes_No_GUI_Callback_Data *d = static_cast<wedge::Yes_No_GUI_Callback_Data *>(data);
	Controls_Settings_GUI *gui = static_cast<Controls_Settings_GUI *>(d->userdata);
	if (d->cancelled == false && d->choice) {
		gui->quit(false);
	}
}

static void duplicates2_callback(void *data)
{
	wedge::Yes_No_GUI_Callback_Data *d = static_cast<wedge::Yes_No_GUI_Callback_Data *>(data);
	Controls_Settings_GUI *gui = static_cast<Controls_Settings_GUI *>(d->userdata);
	if (d->cancelled == false && d->choice) {
		gui->quit2(false);
	}
}

Controls_Settings_GUI::Controls_Settings_GUI(bool keyboard, int settings_top, int settings_selected) :
	keyboard(keyboard),
	assigning(false),
	settings_top(settings_top),
	settings_selected(settings_selected)
{
	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	list = new Widget_Controls_List(4);
	list->set_centre_x(true);
	list->set_centre_y(true);
	list->set_padding_top(int(TTH_GLOBALS->bold_font->get_height()));
	list->set_parent(modal_main_widget);

	if (keyboard) {
		controls[ACTION] = GLOBALS->key_action;
		controls[BACK] = GLOBALS->key_back;
		//controls[DOUGHNUT] = GLOBALS->key_doughnut;
		controls[DIE] = GLOBALS->key_die;
		controls[L] = GLOBALS->key_l;
		controls[R] = GLOBALS->key_r;
		controls[U] = GLOBALS->key_u;
		controls[D] = GLOBALS->key_d;
	}
	else {
		controls[ACTION] = GLOBALS->joy_action;
		controls[BACK] = GLOBALS->joy_back;
		//controls[DOUGHNUT] = GLOBALS->joy_doughnut;
		controls[DIE] = GLOBALS->joy_die;
	}

	set_text();

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Controls_Settings_GUI::~Controls_Settings_GUI()
{
}

void Controls_Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if (assigning) {
		input::convert_focus_to_original(event);
		if (keyboard) {
			if (event->type == TGUI_KEY_DOWN) {
				TTH_GLOBALS->button->play(false);
				controls[index_of(which_assigning)] = event->keyboard.code;
				set_text();
				list->set_selected((int)which_assigning);
				gui->set_focus(list);
				assigning = false;
				GLOBALS->allow_global_mini_pause = true;
				GLOBALS->assigning_controls = false;
			}
			else if (event->type == TGUI_MOUSE_DOWN || event->type == TGUI_JOY_DOWN) {
				TTH_GLOBALS->button->play(false);
				gui->set_focus(list);
				assigning = false;
				GLOBALS->allow_global_mini_pause = true;
				GLOBALS->assigning_controls = false;
			}
		}
		else {
			if (event->type == TGUI_JOY_DOWN) {
				if (
					event->joystick.button != TGUI_B_L &&
					event->joystick.button != TGUI_B_R &&
					event->joystick.button != TGUI_B_U &&
					event->joystick.button != TGUI_B_D
				) {
					TTH_GLOBALS->button->play(false);
					controls[index_of(which_assigning)] = event->joystick.button;
					set_text();
					list->set_selected((int)which_assigning);
					gui->set_focus(list);
					assigning = false;
					GLOBALS->allow_global_mini_pause = true;
					GLOBALS->assigning_controls = false;
				}
			}
#ifdef __linux__
			// On Linux, the gamepads back button can cause a key input with code 0x40000000 (same as clicking Back in browser or Back mouse button)
			else if (event->type == TGUI_MOUSE_DOWN || (event->type == TGUI_KEY_DOWN && event->keyboard.code != 0x40000000)) {
#else
			else if (event->type == TGUI_MOUSE_DOWN || event->type == TGUI_KEY_DOWN) {
#endif
				TTH_GLOBALS->button->play(false);
				gui->set_focus(list);
				assigning = false;
				GLOBALS->allow_global_mini_pause = true;
				GLOBALS->assigning_controls = false;
			}
		}
		if (assigning == false) {
			shim::convert_directions_to_focus_events = true;
		}
	}
	else {
		bool tab = false;
		bool esc = false;
		if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
			esc = true;
			TTH_GLOBALS->button->play(false);
		}

		if (tab || esc) {
			bool changed = false;
			if (keyboard) {
				changed |= controls[ACTION] != GLOBALS->key_action;
				changed |= controls[BACK] != GLOBALS->key_back;
				//changed |= controls[DOUGHNUT] != GLOBALS->key_doughnut;
				changed |= controls[DIE] != GLOBALS->key_die;
				changed |= controls[L] != GLOBALS->key_l;
				changed |= controls[R] != GLOBALS->key_r;
				changed |= controls[U] != GLOBALS->key_u;
				changed |= controls[D] != GLOBALS->key_d;
			}
			else {
				changed |= controls[ACTION] != GLOBALS->joy_action;
				changed |= controls[BACK] != GLOBALS->joy_back;
				//changed |= controls[DOUGHNUT] != GLOBALS->joy_doughnut;
				changed |= controls[DIE] != GLOBALS->joy_die;
			}
			if (changed == false) {
				if (tab) {
					AREA->end_menu();
					exit();
				}
				else if (esc) {
					Settings_GUI *gui = new Settings_GUI(false);
					gui->set_top_selected(settings_top, settings_selected);
					shim::guis.push_back(gui);
					exit();
				}
			}
			else {
				if (tab) {
					Yes_No_GUI *gui = new Yes_No_GUI(GLOBALS->game_t->translate(104)/* Originally: Apply changes? */, true, apply2_callback, this);
					shim::guis.push_back(gui);
				}
				else {
					Yes_No_GUI *gui = new Yes_No_GUI(GLOBALS->game_t->translate(104)/* Originally: Apply changes? */, true, apply_callback, this);
					shim::guis.push_back(gui);
				}
			}
		}
		else {
			TTH_GUI::handle_event(event);
		}
	}
}

void Controls_Settings_GUI::update()
{
	TTH_GUI::update();

	if (transitioning_in || transitioning_out) {
		return;
	}

	int pressed;
	if ((pressed = list->pressed()) >= 0) {
		assigning = true;
		GLOBALS->allow_global_mini_pause = false;
		GLOBALS->assigning_controls = true;
		shim::convert_directions_to_focus_events = false;
		if (keyboard) {
			which_assigning = (Control)pressed;
		}
		else {
			which_assigning = (Control)pressed;
		}
		gui->set_focus(NULL);
	}
}

void Controls_Settings_GUI::draw_back()
{
	GUI::draw_back();

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), shim::screen_size);
}

void Controls_Settings_GUI::draw_fore()
{
	std::string menu_name = keyboard ? GLOBALS->game_t->translate(107)/* Originally: Keyboard Controls */ : GLOBALS->game_t->translate(108)/* Originally: Gamepad Controls */;
	SDL_Colour c1;
	c1.r = 253;
	c1.g = 77;
	c1.b = 79;
	c1.a = 255;
	SDL_Colour c2 = shim::black;
	TTH_GLOBALS->bold_font->enable_shadow(c1, gfx::Font::FULL_SHADOW);
	TTH_GLOBALS->bold_font->draw(c2, menu_name, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	TTH_GLOBALS->bold_font->disable_shadow();

	if (assigning) {
		std::string text;

		if (keyboard) {
			text = GLOBALS->game_t->translate(105)/* Originally: Press a key... */;
		}
		else {
			text = GLOBALS->game_t->translate(106)/* Originally: Press a button... */;
		}

		show_notice(text);
	}
	
	TTH_GUI::draw_fore();
}

void Controls_Settings_GUI::draw()
{
	TTH_GUI::draw();
	GUI::draw();
}

void Controls_Settings_GUI::set_text()
{
	std::vector<std::string> names;
	std::vector<std::string> assignments;

	if (keyboard) {
		names.push_back("Action");
		names.push_back("Back");
		//names.push_back("Doughnut");
		names.push_back("Die");
		names.push_back("Left");
		names.push_back("Right");
		names.push_back("Up");
		names.push_back("Down");

		assignments.push_back(get_key_name(controls[ACTION]));
		assignments.push_back(get_key_name(controls[BACK]));
		//assignments.push_back(get_key_name(controls[DOUGHNUT]));
		assignments.push_back(get_key_name(controls[DIE]));
		assignments.push_back(get_key_name(controls[L]));
		assignments.push_back(get_key_name(controls[R]));
		assignments.push_back(get_key_name(controls[U]));
		assignments.push_back(get_key_name(controls[D]));
	}
	else {
		names.push_back("Action");
		names.push_back("Back");
		//names.push_back("Doughnut");
		names.push_back("Die");

		assignments.push_back(input::get_joystick_button_colour_code(controls[ACTION]) + input::get_joystick_button_name(controls[ACTION]));
		assignments.push_back(input::get_joystick_button_colour_code(controls[BACK]) + input::get_joystick_button_name(controls[BACK]));
		//assignments.push_back(input::get_joystick_button_colour_code(controls[DOUGHNUT]) + input::get_joystick_button_name(controls[DOUGHNUT]));
		assignments.push_back(input::get_joystick_button_colour_code(controls[DIE]) + input::get_joystick_button_name(controls[DIE]));
	}

	list->set_items_extra(names, assignments);
}
	
std::string Controls_Settings_GUI::get_key_name(int code)
{
	if (code < 0) {
		return GLOBALS->game_t->translate(548)/* Originally: ??? */;
	}
	else {
		return ::get_key_name(code);
	}
}

void Controls_Settings_GUI::quit0(bool apply, bool tab)
{
	if (apply) {
		bool same = false;
		if (keyboard) {
			std::vector<int> v;
			v.push_back(controls[ACTION]);
			v.push_back(controls[BACK]);
			//v.push_back(controls[DOUGHNUT]);
			v.push_back(controls[DIE]);
			v.push_back(controls[L]);
			v.push_back(controls[R]);
			v.push_back(controls[U]);
			v.push_back(controls[D]);
			size_t sz = v.size();
			std::sort(v.begin(), v.end());
			std::vector<int>::iterator last = std::unique(v.begin(), v.end());
			v.erase(last, v.end());
			if (sz != v.size()) {
				same = true;
			}
			if (same == false) {
				v.clear();
				sz = v.size();
				std::sort(v.begin(), v.end());
				std::vector<int>::iterator last = std::unique(v.begin(), v.end());
				v.erase(last, v.end());
				if (sz != v.size()) {
					same = true;
				}
			}
		}
		else {
			std::vector<int> v;
			v.push_back(controls[ACTION]);
			v.push_back(controls[BACK]);
			//v.push_back(controls[DOUGHNUT]);
			v.push_back(controls[DIE]);
			size_t sz = v.size();
			std::sort(v.begin(), v.end());
			std::vector<int>::iterator last = std::unique(v.begin(), v.end());
			v.erase(last, v.end());
			if (sz != v.size()) {
				same = true;
			}
			if (same == false) {
				v.clear();
				sz = v.size();
				std::sort(v.begin(), v.end());
				std::vector<int>::iterator last = std::unique(v.begin(), v.end());
				v.erase(last, v.end());
				if (sz != v.size()) {
					same = true;
				}
			}
		}
		if (same) {
			if (tab) {
				Yes_No_GUI *gui = new Yes_No_GUI(GLOBALS->game_t->translate(109)/* Originally: Conflict found! Discard? */, true, duplicates2_callback, this);
				shim::guis.push_back(gui);
			}
			else {
				Yes_No_GUI *gui = new Yes_No_GUI(GLOBALS->game_t->translate(109)/* Originally: Conflict found! Discard? */, true, duplicates_callback, this);
				shim::guis.push_back(gui);
			}
			return;
		}
		else {
			if (keyboard) {
				GLOBALS->key_action = controls[ACTION];
				GLOBALS->key_back = controls[BACK];
				//GLOBALS->key_doughnut = controls[DOUGHNUT];
				GLOBALS->key_die = controls[DIE];
				GLOBALS->key_l = shim::key_l = controls[L];
				GLOBALS->key_r = shim::key_r = controls[R];
				GLOBALS->key_u = shim::key_u = controls[U];
				GLOBALS->key_d = shim::key_d = controls[D];
			}
			else {
				GLOBALS->joy_action = controls[ACTION];
				GLOBALS->joy_back = controls[BACK];
				//GLOBALS->joy_doughnut = controls[DOUGHNUT];
				GLOBALS->joy_die = controls[DIE];
			}
		}
	}

	if (tab) {
		save_settings();
		AREA->end_menu();
		exit();
	}
	else {
		save_settings();
		Settings_GUI *gui = new Settings_GUI(false);
		gui->set_top_selected(settings_top, settings_selected);
		shim::guis.push_back(gui);
		exit();
	}
}

void Controls_Settings_GUI::quit(bool apply)
{
	quit0(apply, false);
}

void Controls_Settings_GUI::quit2(bool apply)
{
	quit0(apply, true);
}

int Controls_Settings_GUI::index_of(int i)
{
	return i;
}

//--

static void misc_bail(void *data)
{
	auto d = static_cast<Notification_GUI::Callback_Data *>(data);
	auto gui = static_cast<Miscellaneous_Settings_GUI *>(d->userdata);
	gui->bail();
}

Miscellaneous_Settings_GUI::Miscellaneous_Settings_GUI(int settings_top, int settings_selected) :
	settings_top(settings_top),
	settings_selected(settings_selected)
{
	have_content = false;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);

	Widget *container = new Widget();
	container->set_centre_x(true);
	container->set_centre_y(true);
	container->set_parent(modal_main_widget);

#if 0
	easy_combos_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(370)/* Originally: Easy combos mode */, easy_combos);
	easy_combos_checkbox->set_centre_x(true);
	easy_combos_checkbox->set_parent(container);
	easy_combos_checkbox->set_padding_bottom(2);
	easy_combos_checkbox->set_padding_top(5);

	simple_turn_display_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(491)/* Originally: Basic turn order display */, simple_turn_display);
	simple_turn_display_checkbox->set_centre_x(true);
	simple_turn_display_checkbox->set_parent(container);
	simple_turn_display_checkbox->set_padding_bottom(2);
	simple_turn_display_checkbox->set_break_line(true);
	simple_turn_display_checkbox->set_clear_float_x(true);
#endif
	
//#if !defined IOS || defined TVOS
#if 0
	safe_mode_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(126)/* Originally: TV safe mode */, tv_safe_mode);
	safe_mode_checkbox->set_centre_x(true);
	safe_mode_checkbox->set_parent(container);
	safe_mode_checkbox->set_padding_bottom(2);
	safe_mode_checkbox->set_padding_top(5);
	have_content = true;
#else
	safe_mode_checkbox = NULL;
#endif

#if defined ANDROID || (defined IOS && !defined TVOS)
	bool go = input::system_has_touchscreen();
#else
	bool go = shim::force_tablet ? input::system_has_touchscreen() : false;
#endif

	if (go) {
		onscreen_controller_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(167)/* Originally: Use on-screen controller */, GLOBALS->onscreen_controller_was_enabled);
		onscreen_controller_checkbox->set_centre_x(true);
		onscreen_controller_checkbox->set_parent(container);
		onscreen_controller_checkbox->set_padding_bottom(2);
		onscreen_controller_checkbox->set_break_line(true);
		onscreen_controller_checkbox->set_clear_float_x(true);
		have_content = true;
	}
	else {
		onscreen_controller_checkbox = NULL;
	}

	prerendered_music_checkbox = nullptr;
#if 0
	prerendered_music_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(585)/* Originally: Prerendered Music */, TTH_GLOBALS->prerendered_music);
	prerendered_music_checkbox->set_centre_x(true);
	prerendered_music_checkbox->set_parent(container);
	prerendered_music_checkbox->set_break_line(true);
	prerendered_music_checkbox->set_padding_bottom(2);
	prerendered_music_checkbox->set_clear_float_x(true);
	have_content = true;
#endif

#ifndef TVOS
#ifdef ANDROID
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "has_vibrator", "()Z");

	bool has_vibrator = (bool)env->CallBooleanMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);

	if (has_vibrator) {
#endif
		rumble_enabled_checkbox = new Widget_Checkbox(GLOBALS->game_t->translate(127)/* Originally: Rumble Enabled */, GLOBALS->rumble_enabled);
		rumble_enabled_checkbox->set_centre_x(true);
		rumble_enabled_checkbox->set_parent(container);
		rumble_enabled_checkbox->set_break_line(true);
		rumble_enabled_checkbox->set_padding_bottom(2);
		rumble_enabled_checkbox->set_clear_float_x(true);
		have_content = true;
#ifdef ANDROID
	}
	else {
		rumble_enabled_checkbox = NULL;
	}
#endif
#else
	rumble_enabled_checkbox = NULL;
#endif
	
	//have_content = true;

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);
}

Miscellaneous_Settings_GUI::~Miscellaneous_Settings_GUI()
{
}

void Miscellaneous_Settings_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	bool tab = false;
	bool esc = false;
	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		esc = true;
	}

	if (tab || esc) {
#if 0
		if (easy_combos_checkbox != NULL) {
			easy_combos = easy_combos_checkbox->is_checked();
		}
		if (simple_turn_display_checkbox != NULL) {
			simple_turn_display = simple_turn_display_checkbox->is_checked();
		}
#endif
		if (onscreen_controller_checkbox != NULL) {
			//wedge::enable_onscreen_controller(onscreen_controller_checkbox->is_checked());
			GLOBALS->onscreen_controller_was_enabled = onscreen_controller_checkbox->is_checked();
		}
		if (safe_mode_checkbox != NULL) {
			tv_safe_mode = safe_mode_checkbox->is_checked();
		}
		if (prerendered_music_checkbox != NULL) {
			TTH_GLOBALS->prerendered_music = prerendered_music_checkbox->is_checked();
		}
		if (rumble_enabled_checkbox != NULL) {
			GLOBALS->rumble_enabled = rumble_enabled_checkbox->is_checked();
		}
		wedge::enable_onscreen_controller(GLOBALS->onscreen_controller_was_enabled);
		save_settings();
		wedge::enable_onscreen_controller(false);
	}

	if (tab) {
		AREA->end_menu();
		exit();
	}
	else if (esc) {
		TTH_GLOBALS->button->play(false);
		Settings_GUI *gui = new Settings_GUI(false);
		gui->set_top_selected(settings_top, settings_selected);
		shim::guis.push_back(gui);
		exit();
	}
	else {
		//bool easy_combos_checked;
		bool onscreen_controller_checked;
		bool safe_mode_checked;
		bool prerendered_music_checked;
		bool rumble_enabled_checked;
		//bool simple_turn_display_checked;
		/*
		if (easy_combos_checkbox != NULL) {
			easy_combos_checked = easy_combos_checkbox->is_checked();
		}
		else {
			easy_combos_checked = false;
		}
		*/
		if (onscreen_controller_checkbox != NULL) {
			onscreen_controller_checked = onscreen_controller_checkbox->is_checked();
		}
		else {
			onscreen_controller_checked = false;
		}
		if (safe_mode_checkbox != NULL) {
			safe_mode_checked = safe_mode_checkbox->is_checked();
		}
		else {
			safe_mode_checked = false;
		}
		if (prerendered_music_checkbox != NULL) {
			prerendered_music_checked = prerendered_music_checkbox->is_checked();
		}
		else {
			prerendered_music_checked = false;
		}
		if (rumble_enabled_checkbox != NULL) {
			rumble_enabled_checked = rumble_enabled_checkbox->is_checked();
		}
		else {
			rumble_enabled_checked = false;
		}
		/*
		if (simple_turn_display_checkbox != NULL) {
			simple_turn_display_checked = simple_turn_display_checkbox->is_checked();
		}
		else {
			simple_turn_display_checked = false;
		}
		*/
		TTH_GUI::handle_event(event);
		if (onscreen_controller_checkbox != NULL) {
			if (onscreen_controller_checkbox->is_checked() != onscreen_controller_checked) {
				onscreen_controller_checked = !onscreen_controller_checked;
				GLOBALS->onscreen_controller_was_enabled = onscreen_controller_checked;
			}
		}
		if (safe_mode_checkbox != NULL) {
			if (safe_mode_checkbox->is_checked() != safe_mode_checked) {
				safe_mode_checked = !safe_mode_checked;
				apply_tv_safe_mode(safe_mode_checked);
				lost_device();
				found_device();
			}
		}
		if (prerendered_music_checkbox != NULL) {
			if (prerendered_music_checkbox->is_checked() != prerendered_music_checked) {
				prerendered_music_checked = !prerendered_music_checked;
				TTH_GLOBALS->prerendered_music = prerendered_music_checked;
				std::string name = shim::music->get_name();
				name = name.substr(0, name.length()-4);
				if (prerendered_music_checked) {
					name += "_prerendered.mml";
				}
				else {
					name = name.substr(0, name.length()-12);
					name += ".mml";
				}
				audio::play_music(name);
				if (AREA != nullptr) {
					shim::music->set_master_volume(shim::music_volume/3.333f);
				}
			}
		}
		if (rumble_enabled_checkbox != NULL) {
			if (rumble_enabled_checkbox->is_checked() != rumble_enabled_checked) {
				rumble_enabled_checked = !rumble_enabled_checked;
				GLOBALS->rumble_enabled = rumble_enabled_checked;
				if (rumble_enabled_checked) {
					wedge::rumble(1.0f, 500);
				}
			}
		}
	}
}

void Miscellaneous_Settings_GUI::draw_back()
{
	GUI::draw_back();

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), shim::screen_size);
}

void Miscellaneous_Settings_GUI::draw()
{
	TTH_GUI::draw();

	SDL_Colour c1;
	c1.r = 253;
	c1.g = 77;
	c1.b = 79;
	c1.a = 255;
	SDL_Colour c2 = shim::black;
	TTH_GLOBALS->bold_font->enable_shadow(c1, gfx::Font::FULL_SHADOW);
	TTH_GLOBALS->bold_font->draw(c2, GLOBALS->game_t->translate(128)/* Originally: Miscellaneous Settings */, util::Point<int>(shim::screen_size.w/2, shim::screen_size.h*0.05), true, true);
	TTH_GLOBALS->bold_font->disable_shadow();
	
	GUI::draw();
}

void Miscellaneous_Settings_GUI::update()
{
	TTH_GUI::update();

	if (have_content == false && transitioning_in == false) {
		have_content = true; // cheap way to avoid coming back here
		Notification_GUI *gui = new Notification_GUI(GLOBALS->game_t->translate(253)/* Originally: No settings are available at this time! */, misc_bail, this);
		shim::guis.push_back(gui);
	}
}

void Miscellaneous_Settings_GUI::bail()
{
	Settings_GUI *gui = new Settings_GUI(false);
	gui->set_top_selected(settings_top, settings_selected);
	shim::guis.push_back(gui);
	exit();
}

//--

Get_Number_GUI::Get_Number_GUI(std::string text, int stops, int initial_value, util::Callback callback, void *callback_data) :
	callback(callback),
	callback_data(callback_data)
{
	transition = true;
	transition_is_shrink = true;
	transition_duration = 200;
	started_transition_timer = false;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	SDL_Colour background_colour = shim::black;
	background_colour.r *= 0.5;
	background_colour.g *= 0.5;
	background_colour.b *= 0.5;
	background_colour.a *= 0.5;
	modal_main_widget->set_background_colour(background_colour);

	Widget_Label *label = new Widget_Label(text, 100, TTH_GLOBALS->bold_font);
	label->set_centre_x(true);
	label->set_padding(WIN_BORDER);

	slider = new Widget_Slider(100, stops, initial_value);
	slider->set_centre_x(true);
	slider->set_clear_float_x(true);
	slider->set_break_line(true);
	slider->set_padding_left(WIN_BORDER);
	slider->set_padding_right(WIN_BORDER);

	value_label = new Widget_Label("", 50, TTH_GLOBALS->bold_font);
	value_label->set_centre_x(true);
	value_label->set_clear_float_x(true);
	value_label->set_break_line(true);
	value_label->set_padding(WIN_BORDER);

	ok_button = new Widget_Text_Button(GLOBALS->game_t->translate(69)/* Originally: OK */);
	ok_button->set_centre_x(true);
	ok_button->set_padding_bottom(WIN_BORDER);
	ok_button->set_padding_right(2);

	slider->set_down_widget(ok_button);
	slider->set_action_button(ok_button);

	cancel_button = new Widget_Text_Button(GLOBALS->game_t->translate(164)/* Originally: Cancel */);
	cancel_button->set_centre_x(true);
	cancel_button->set_padding_left(20);

	Widget *button_container = new Widget(1.0f, ok_button->get_height());
	button_container->set_float_bottom(true);

	Widget_Window *window = new Widget_Window(100 + WIN_BORDER*4, label->get_height() + slider->get_height() + value_label->get_height() + ok_button->get_height() + WIN_BORDER*6);
	window->set_centre_x(true);
	window->set_centre_y(true);
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(WIN_BORDER);
	pad->set_parent(window);
	
	label->set_parent(pad);
	slider->set_parent(pad);
	value_label->set_parent(pad);
	ok_button->set_padding_right(2);
	button_container->set_parent(pad);
	ok_button->set_parent(button_container);
	cancel_button->set_parent(button_container);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(slider);
}

Get_Number_GUI::~Get_Number_GUI()
{
}

void Get_Number_GUI::handle_event(TGUI_Event *event)
{
	if (
		(event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) ||
		(event->type== TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		TTH_GLOBALS->button->play(false);
		Callback_Data d;
		d.number = -1;
		d.userdata = callback_data;
		callback(&d);
		exit();
	}
	else {
		GUI::handle_event(event);
	}
}

void Get_Number_GUI::update()
{
	value_label->set_text(util::itos(slider->get_value()));

	gui->layout();

	if (ok_button->pressed()) {
		Callback_Data d;
		d.number = slider->get_value();
		d.userdata = callback_data;
		callback(&d);
		exit();
	}
	else if (cancel_button->pressed()) {
		Callback_Data d;
		d.number = -1;
		d.userdata = callback_data;
		callback(&d);
		exit();
	}
}

//--

Battle_Multi_Confirm_GUI::Battle_Multi_Confirm_GUI(util::Callback callback, void *callback_data) :
	callback(callback),
	callback_data(callback_data)
{
	transition = false;

	const int w = 160;

	Widget *modal_main_widget = new Widget(1.0f, 1.0f);
	SDL_Colour background_colour;
	background_colour.r = 0.0f;
	background_colour.g = 0.0f;
	background_colour.b = 0.0f;
	background_colour.a = 0.0f;
	modal_main_widget->set_background_colour(background_colour);

	label = new Widget_Label("", w, TTH_GLOBALS->bold_font);
	label->set_centre_x(true);
	label->set_padding(WIN_BORDER);

	confirm_button = new Widget_Text_Button(GLOBALS->game_t->translate(450)/* Originally: Confirm */);
	confirm_button->set_centre_x(true);
	confirm_button->set_clear_float_x(true);
	confirm_button->set_padding_bottom(WIN_BORDER);
	confirm_button->set_break_line(true);
	confirm_button->set_mouse_only(true);

	window = new Widget_Window(w + WIN_BORDER*4, label->get_height() + confirm_button->get_height() + WIN_BORDER*6);
	window->set_centre_x(true);
	window->set_centre_y(true);
	window->set_alpha(0.9f);
	window->set_parent(modal_main_widget);

	TGUI_Widget *pad = new TGUI_Widget(1.0f, 1.0f);
	pad->set_padding(WIN_BORDER);
	pad->set_parent(window);
	
	label->set_parent(pad);
	confirm_button->set_parent(pad);

	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	gui->set_focus(confirm_button);

	set_text();
}

Battle_Multi_Confirm_GUI::~Battle_Multi_Confirm_GUI()
{
}

void Battle_Multi_Confirm_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}

	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		GLOBALS->button->play(false);
		transition = false;
		if (callback) {
			Battle_Multi_Confirm_GUI::Callback_Data d;
			d.confirmed = false;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}
	else
	{
		gui::GUI::handle_event(event);
	}
}

void Battle_Multi_Confirm_GUI::update()
{
	TTH_GUI::update();

	if (confirm_button->pressed()) {
		transition = false;
		if (callback) {
			Battle_Multi_Confirm_GUI::Callback_Data d;
			d.confirmed = true;
			d.userdata = callback_data;
			callback(&d);
		}
		exit();
	}

	set_text();
}

void Battle_Multi_Confirm_GUI::resize(util::Size<int> new_size)
{
	int num_lines = 3;
	int w = 180;
	int h = shim::font->get_height() * num_lines + 3*2 + 3*2;
	int x = (shim::screen_size.w-w)/2;
	int y = (shim::screen_size.h-h)-5;

	window->set_relative_position(x, y);
	window->set_width(w);
	window->set_height(h);
	
	gui->resize(new_size.w, new_size.h);
}

void Battle_Multi_Confirm_GUI::set_text()
{
	std::string text;

	if (wedge::is_onscreen_controller_enabled()) {
		text = GLOBALS->game_t->translate(162)/* Originally: Press */ + " " + input::get_joystick_button_colour_code(TGUI_B_X) + input::get_joystick_button_name(TGUI_B_X) + "|21 " + GLOBALS->game_t->translate(451)/* Originally: to confirm */;
	}
	else {
		text = GLOBALS->game_t->translate(452)/* Originally: Tap to confirm */;
	}

	label->set_text(text);
	gui->layout();
}
