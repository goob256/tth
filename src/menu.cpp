#include <wedge3/map_entity.h>
#include <wedge3/onscreen_controller.h>
#include <wedge3/special_number.h>

#include "area_game.h"
#include "combo.h"
#include "general.h"
#include "globals.h"
#include "menu.h"
#include "milestones.h"
#include "stats.h"
#include "widgets.h"

Menu_Game::Menu_Game()
{
}

Menu_Game::~Menu_Game()
{
	for (std::vector<gui::GUI *>::iterator it = shim::guis.begin(); it != shim::guis.end(); it++) {
		gui::GUI *gui = *it;
		if (dynamic_cast<Menu_GUI *>(gui)) {
			gui->exit();
		}
	}
}

bool Menu_Game::start()
{
	if (Game::start() == false) {
		return false;
	}

	Main_Menu_GUI *gui = new Main_Menu_GUI(0);
	shim::guis.push_back(gui);

	return true;
}

bool Menu_Game::run()
{
	Game::run(); // don't return if false, there are no systems usually
	
	if (AREA->is_pausing() == false && AREA->is_paused() == false && (shim::guis.size() == 0 || dynamic_cast<Menu_GUI *>(shim::guis.back()) == NULL)) {
		return false;
	}

	return true;
}

void Menu_Game::draw()
{
	Game::draw();
}

void Menu_Game::draw_fore()
{
	Game::draw_fore();
}

//--

Menu_GUI::Menu_GUI()
{
	transition_is_slide = false;
	transition_is_slide_vertical = true;
}

Menu_GUI::~Menu_GUI()
{
}

void Menu_GUI::draw()
{
	GUI::draw();

	// FIXME: translate
	SDL_Colour c1;
	c1.r = 253;
	c1.g = 77;
	c1.b = 79;
	c1.a = 255;
	SDL_Colour c2 = shim::black;
	TTH_GLOBALS->bold_font->enable_shadow(c1, gfx::Font::FULL_SHADOW);
	TTH_GLOBALS->bold_font->draw(c2, util::uppercase(menu_name), util::Point<int>(shim::screen_size.w/2-TTH_GLOBALS->bold_font->get_text_width(menu_name)/2, EDGE_Y-TTH_GLOBALS->bold_font->get_height()/2));
	TTH_GLOBALS->bold_font->disable_shadow();
}

void Menu_GUI::handle_event(TGUI_Event *event)
{
	if (transitioning_in || transitioning_out) {
		return;
	}
	
	gui::GUI::handle_event(event);
}

//--

Main_Menu_GUI::Main_Menu_GUI(int selected)
{
	mim = new gfx::Sprite("mim");
	mim->set_animation("stand_s");

	// FIXME: translate
	menu_name = GLOBALS->game_t->translate(583)/* Originally: Menu */;

	TGUI_Widget *modal_main_widget = new TGUI_Widget(1.0f, 1.0f);
	modal_main_widget->set_padding_left(EDGE_X);
	modal_main_widget->set_padding_right(EDGE_X);
	modal_main_widget->set_padding_top(EDGE_Y);
	modal_main_widget->set_padding_bottom(EDGE_Y);

	TGUI_Widget *container = new TGUI_Widget(200, 60);
	container->set_centre_x(true);
	container->set_centre_y(true);
	container->set_parent(modal_main_widget);

	buttons = new TGUI_Widget(80, 60);
	buttons->set_float_right(true);
	buttons->set_parent(container);

	doughnut_button = new Widget_Text_Button(GLOBALS->game_t->translate(579)/* Originally: Doughnut */);
	doughnut_button->set_parent(buttons);

	settings_button = new Widget_Text_Button(GLOBALS->game_t->translate(50)/* Originally: Settings */);
	settings_button->set_break_line(true);
	settings_button->set_parent(buttons);
	
	quit_button = new Widget_Text_Button(GLOBALS->game_t->translate(51)/* Originally: Quit */);
	quit_button->set_break_line(true);
	quit_button->set_parent(buttons);

	// Wrap cursor
	doughnut_button->set_up_widget(quit_button);
	quit_button->set_down_widget(doughnut_button);
	//items_button->set_up_widget(save_button);
	//save_button->set_down_widget(items_button);
	
	gui = new TGUI(modal_main_widget, shim::screen_size.w, shim::screen_size.h);

	switch (selected) {
		case 0:
			gui->set_focus(doughnut_button);
			break;
		case 1:
			gui->set_focus(settings_button);
			break;
		case 2:
			gui->set_focus(quit_button);
			break;
	}
}

Main_Menu_GUI::~Main_Menu_GUI()
{
}
	
void Main_Menu_GUI::handle_event(TGUI_Event *event)
{
	Menu_GUI::handle_event(event);

	if (transitioning_in || transitioning_out) {
		return;
	}
	
	if ((event->type == TGUI_KEY_DOWN && event->keyboard.code == GLOBALS->key_back) || (event->type == TGUI_JOY_DOWN && event->joystick.button == GLOBALS->joy_back)) {
		AREA->end_menu();
		exit();
	}
}

void Main_Menu_GUI::draw_back()
{
	GUI::draw_back();

	gfx::draw_filled_rectangle(shim::black, util::Point<int>(0, 0), shim::screen_size);

	int x = buttons->get_x();
	int y = buttons->get_y();
	int w = buttons->get_width();
	int h = buttons->get_height();

	gfx::draw_9patch(TTH_GLOBALS->menu_window, util::Point<int>(x-w-20, y), util::Size<int>(w, h));

	mim->get_current_image()->draw({x-w-20.0f+5.0f, y+5.0f+16.0f-mim->get_current_image()->size.h});
	
	TTH_GLOBALS->bold_font->draw(shim::white, GLOBALS->game_t->translate(584)/* Originally: Mim */, {x-w-20.0f+5.0f+16.0f+3.0f, y+5.0f+8.0f-shim::font->get_height()/2.0f});

	TTH_GLOBALS->heart->get_current_image()->draw({x-w-20.0f+5.0f+2.0f, y+5.0f+16.0f+5.0f});
	TTH_GLOBALS->doughnut->get_current_image()->draw({x-w-20.0f+5.0f+2.0f, y+5.0f+16.0f+5.0f+2.0f+shim::font->get_height()});
	
	shim::font->draw(shim::white, util::string_printf("%2d/%2d", INSTANCE->stats[0].base.hp, INSTANCE->stats[0].base.fixed.max_hp), {x-w-20.0f+5.0f+16.0f+4.0f, y+5.0f+16.0f+5.0f});
	shim::font->draw(shim::white, util::string_printf("%d", INSTANCE->get_gold()), {x-w-20.0f+5.0f+16.0f+4.0f, y+5.0f+16.0f+5.0f+2.0f+shim::font->get_height()});
}

void Main_Menu_GUI::draw()
{
	Menu_GUI::draw();
}

static void end_jump(void *data)
{
	gfx::Sprite *sprite = static_cast<gfx::Sprite *>(data);
	sprite->set_animation("stand_s");
}

void Main_Menu_GUI::update()
{
	TTH_GUI::update();

	if (doughnut_button->pressed()) {
		if (INSTANCE->get_gold() <= 0) {
			TTH_GLOBALS->error_sfx->play(false);
		}
		else {
			TTH_GLOBALS->doughnut_sfx->play(false);
			TTH_GLOBALS->jump_sfx->play(false);
			mim->set_animation("jump", end_jump, mim);
			INSTANCE->stats[0].base.hp = MIN(INSTANCE->stats[0].base.fixed.max_hp, INSTANCE->stats[0].base.hp+DOUGHNUT_HP);
			INSTANCE->add_gold(-1);
		}
	}
	else if (quit_button->pressed()) {
		GLOBALS->mini_pause();
	}
	else if (settings_button->pressed()) {
		Settings_GUI *gui = new Settings_GUI(true);
		shim::guis.push_back(gui);
		exit();
	}
}
