#ifndef NOO_FONT_H
#define NOO_FONT_H

#include "shim3/main.h"

namespace noo {

namespace gfx {

class Image;

ALIGN(16) Font {
public:
	static std::string strip_codes(std::string text, bool strip_colour_codes, bool strip_wrap_codes, bool strip_extra_glyph_codes);
	static void static_start();
	static void release_all();
	static void end_batches();

	enum Shadow_Type {
		NO_SHADOW = 0,
		DROP_SHADOW,
		FULL_SHADOW,
		NUM_SHADOW_TYPES
	};

	SHIM3_EXPORT Font();
	SHIM3_EXPORT virtual ~Font();

	SHIM3_EXPORT virtual bool cache_glyphs(std::string text) = 0;
	SHIM3_EXPORT virtual void clear_cache() = 0;

	SHIM3_EXPORT virtual int get_text_width(std::string text, bool interpret_colour_codes = true, bool interpret_extra_glyphs = true);
	SHIM3_EXPORT virtual int get_height() = 0;

	SHIM3_EXPORT void enable_shadow(SDL_Colour shadow_colour, Shadow_Type shadow_type);
	SHIM3_EXPORT void disable_shadow();

	// If interpret_colour_codes is true, this returns the last colour used. Otherwise, it returns the colour passed in.
	SHIM3_EXPORT SDL_Colour draw(SDL_Colour colour, std::string text, util::Point<float> dest_position, bool interpret_colour_codes = true, bool centre = false, bool interpret_extra_glyphs = true);

	// Returns number of characters drawn, plus whether or not it filled the max in bool &full
	SHIM3_EXPORT int draw_wrapped(SDL_Colour colour, std::string text, util::Point<float> dest_position, int w, int line_height, int max_lines, int elapsed, int delay, bool dry_run, bool &full, int &num_lines, int &width, bool interpret_colour_codes = true, bool centre = false, int first_line_indent = 0, bool interpret_extra_glyphs = true);

	SHIM3_EXPORT bool is_batching();

	SHIM3_EXPORT void set_vertex_cache_id(Uint32 cache_id);

	SHIM3_EXPORT void start_batch();
	SHIM3_EXPORT void end_batch();
	SHIM3_EXPORT void flush_vertex_cache();

	SHIM3_EXPORT void add_extra_glyph(int code, Image *image); // for @00 images
	SHIM3_EXPORT void remove_extra_glyph(int code);
	SHIM3_EXPORT void set_extra_glyph_offset(util::Point<float> offset);

	// For 16 byte alignment to make glm::mat4 able to use SIMD
#ifdef _WIN32
	SHIM3_EXPORT void *operator new(size_t i);
	SHIM3_EXPORT void operator delete(void* p);
#endif
	
protected:
	static std::vector<Font *> loaded_fonts;

	SHIM3_EXPORT void select_vertex_cache();
	SHIM3_EXPORT void select_previous_vertex_cache();

	struct Glyph {
		util::Point<int> position;
		util::Size<int> size;
	};

	std::map<Uint32, Glyph *> glyphs;

	SDL_Colour shadow_colour;
	Shadow_Type shadow_type;

	bool batching;
	Uint32 vertex_cache_id;
	Uint32 prev_vertex_cache_id;

	Image *sheet;

	int size_w;
	int size;

	std::map<int, Image *> extra_glyphs;
	util::Point<float> extra_glyph_offset;
};

} // End namespace gfx

} // End namespace noo

#endif // NOO_FONT_H
