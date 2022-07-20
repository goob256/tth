#ifndef WEDGE3_GENERAL_H
#define WEDGE3_GENERAL_H

#include "wedge3/main.h"
#include "wedge3/globals.h"
#include "wedge3/stats.h"

namespace wedge {

util::Point<int> WEDGE3_EXPORT add_direction(util::Point<int> pos, Direction direction, int n);
Direction WEDGE3_EXPORT direction_from_offset(util::Point<int> offset);

std::string WEDGE3_EXPORT integer_point_to_string(util::Point<int> p);
std::string WEDGE3_EXPORT float_point_to_string(util::Point<float> p);
std::string WEDGE3_EXPORT integer_size_to_string(util::Size<int> s);
std::string WEDGE3_EXPORT float_size_to_string(util::Size<float> s);
std::string WEDGE3_EXPORT direction_to_string(Direction d);
std::string WEDGE3_EXPORT bool_to_string(bool b);
std::string WEDGE3_EXPORT sprite_to_string(gfx::Sprite *sprite);
std::string WEDGE3_EXPORT image_to_string(gfx::Image *image);

util::Point<int> WEDGE3_EXPORT json_to_integer_point(util::JSON::Node *json);
util::Point<float> WEDGE3_EXPORT json_to_float_point(util::JSON::Node *json);
util::Size<int> WEDGE3_EXPORT json_to_integer_size(util::JSON::Node *json);
util::Size<float> WEDGE3_EXPORT json_to_float_size(util::JSON::Node *json);
gfx::Sprite WEDGE3_EXPORT *json_to_sprite(util::JSON::Node *json);
gfx::Image WEDGE3_EXPORT *json_to_image(util::JSON::Node *json);

std::string WEDGE3_EXPORT save();
bool WEDGE3_EXPORT save(std::string s, std::string filename);
bool WEDGE3_EXPORT save(std::string filename);
bool WEDGE3_EXPORT save_play_time(std::string filename);

util::Point<int> WEDGE3_EXPORT get_mouse_position();

void WEDGE3_EXPORT pause_player_input(bool paused);
void WEDGE3_EXPORT pause_presses(bool paused, bool repeat_pressed = false);
bool WEDGE3_EXPORT are_presses_paused();

// add 'tiles' tiles to pos,offset
void WEDGE3_EXPORT add_tiles(util::Point<int> &pos, util::Point<float> &offset, util::Point<float> tiles);
void WEDGE3_EXPORT abs_to_tile(util::Point<float> pos, util::Point<int> &out_tile, util::Point<float> &out_offset);
void WEDGE3_EXPORT tile_to_abs(util::Point<int> pos, util::Point<float> offset, util::Point<float> &out_pos);

void WEDGE3_EXPORT rumble(float force, int milliseconds);

util::JSON WEDGE3_EXPORT *load_savegame(std::string filename);

void WEDGE3_EXPORT quit_all();

int WEDGE3_EXPORT JOYF_TO_I(float v, float orig);

void WEDGE3_EXPORT clip_rect_to_rect(util::Point<float> clip_pos, util::Size<float> clip_sz, util::Point<float> &pos, util::Size<float> &sz);

}

#endif // WEDGE3_GENERAL_H
