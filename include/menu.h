#ifndef MENU_H
#define MENU_H

#include <wedge3/main.h>

#include "combo.h"
#include "gui.h"

class Widget_Image_Button;
class Widget_List;
class Widget_Quantity_List;
class Widget_Map;

class Menu_Game : public wedge::Game
{
public:
	Menu_Game();
	virtual ~Menu_Game();

	bool start();
	bool run();
	void draw();
	void draw_fore();
};

class Menu_GUI : public TTH_GUI
{
public:
	static const int BORDER = 3;
	static const int EDGE_X = 10;
	static const int EDGE_Y = 8;

	Menu_GUI();
	virtual ~Menu_GUI();

	void draw();
	void handle_event(TGUI_Event *event);

protected:
	std::string menu_name;
};

class Main_Menu_GUI : public Menu_GUI
{
public:
	Main_Menu_GUI(int selected);
	virtual ~Main_Menu_GUI();

	void handle_event(TGUI_Event *event);
	void draw_back();
	void draw();
	void update();

private:
	
	TGUI_Widget *buttons;
	Widget_Text_Button *doughnut_button;
	Widget_Text_Button *settings_button;
	Widget_Text_Button *quit_button;
	gfx::Sprite *mim;
};

#endif // MENU_H
