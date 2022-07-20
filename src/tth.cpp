#include <wedge3/main.h>
#include <wedge3/globals.h>
#include <wedge3/onscreen_controller.h>

#include "battle_player.h"
#include "enemies.h"
#include "general.h"
#include "globals.h"
#include "gui.h"
#include "widgets.h"

int orig_argc;
char **orig_argv;

std::string extra_args;
std::string extra_args_orig;

float sfx_volume = 1.0f;
float music_volume = 1.0f;
bool tv_safe_mode = false;
int windowed = -1;
int fullscreen = -1;
int screen_w = -1;
int screen_h = -1;
bool autosave_help_shown = false;
bool easy_combos = true;
bool simple_turn_display = false;

#ifdef GOOGLE_PLAY
#include <jni.h>

void start_google_play_games_services()
{
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject activity = (jobject)SDL_AndroidGetActivity();
	jclass clazz(env->GetObjectClass(activity));

	jmethodID method_id = env->GetMethodID(clazz, "start_google_play_games_services", "()V");

	env->CallVoidMethod(activity, method_id);

	env->DeleteLocalRef(activity);
	env->DeleteLocalRef(clazz);
}
#endif

int main(int argc, char **argv)
{
	try {
#ifdef _WIN32
		SDL_RegisterApp("Tower To Heaven", 0, 0);
#endif

#if defined IOS
		SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown");
#endif

		orig_argc = argc;
		orig_argv = argv;

		// this must be before static_start which inits SDL
#ifdef _WIN32
		bool directsound = util::bool_arg(true, argc, argv, "directsound");
		bool wasapi = util::bool_arg(false, argc, argv, "wasapi");
		bool winmm = util::bool_arg(false, argc, argv, "winmm");

		if (directsound) {
			_putenv_s("SDL_AUDIODRIVER", "directsound");
		}
		else if (wasapi) {
			_putenv_s("SDL_AUDIODRIVER", "wasapi");
		}
		else if (winmm) {
			_putenv_s("SDL_AUDIODRIVER", "winmm");
		}
#endif

		shim::window_title = "Tower To Heaven";
		shim::organisation_name = "b1stable";
		shim::game_name = "Tower To Heaven";
		//
		shim::logging = true;

		// Need to be set temporarily to original values so +error-level works. They get changed below
		shim::argc = orig_argc;
		shim::argv = orig_argv;
	
		shim::static_start_all();

		shim::create_depth_buffer = true;

		start_autosave_thread();
		
		bool settings_loaded = load_settings();

		// This can happen if you use a large external monitor and switch to a smaller screen with lower resolution
		// If this happens, use the default res.
		auto modes = gfx::get_supported_video_modes();
		int max_w = 0;
		int max_h = 0;
		for (auto m : modes) {
			if (m.w > max_w) {
				max_w = m.w;
			}
			if (m.h > max_h) {
				max_h = m.h;
			}
		}
		if (screen_w > max_w || screen_h > max_h) {
			screen_w = -1;
			screen_h = -1;
			util::infomsg("screen_w/screen_h greater than max resolution. Using default.\n");
		}

		set_shim_args(true, windowed == 1, fullscreen == 1);

		if (wedge::start(util::Size<int>(SCR_W, SCR_H), util::Size<int>(screen_w, screen_h)) == false) {
			util::debugmsg("wedge::start returned false.\n");
			return 1;
		}

		if (tv_safe_mode) {
			apply_tv_safe_mode(true);
		}

		Widget::static_start();

		Settings_GUI::static_start();

		//Enemy_Wrath::static_start();

		SDL_Colour text;
		text.r = 192;
		text.g = 192;
		text.b = 192;
		text.a = 255;
		SDL_Colour hilight1;
		hilight1.r = 128;
		hilight1.g = 0;
		hilight1.b = 128;
		hilight1.a = 255;
		SDL_Colour hilight2;
		hilight2.r = 255;
		hilight2.g = 0;
		hilight2.b = 255;
		hilight2.a = 255;

		Widget_List::set_default_normal_text_colour1(text);
		Widget_List::set_default_normal_text_colour2(shim::white);
		Widget_List::set_default_text_shadow_colour(shim::transparent);
		Widget_List::set_default_highlight_text_colour1(hilight1);
		Widget_List::set_default_highlight_text_colour2(hilight2);
		Widget_List::set_default_disabled_text_colour1(shim::black);
		Widget_List::set_default_disabled_text_colour2(shim::black);
		Widget_List::set_default_disabled_text_shadow_colour(shim::transparent);
		Widget_Label::set_default_text_colour(shim::white);
		Widget_Label::set_default_shadow_colour(shim::transparent);

		//shim::white = shim::palette[255]; // use our own white, it's actual white but could change
		//shim::black = shim::palette[0]; // black doesn't exist in our palette

		wedge::globals = new Globals();

		std::string tmp_language = "English";

		if (settings_loaded == false) {
			std::map<std::string, std::string> languages;

			languages["german"] = "German";
			languages["french"] = "French";
			languages["dutch"] = "Dutch";
			languages["greek"] = "Greek";
			languages["italian"] = "Italian";
			languages["polish"] = "Polish";
			languages["portuguese"] = "Portuguese";
			languages["russian"] = "Russian";
			languages["spanish"] = "Spanish";
			languages["korean"] = "Korean";
			languages["english"] = "English";
			languages["brazilian"] = "Brazilian";

			std::string syslang = util::get_system_language();

			tmp_language = languages[syslang];

			if (tmp_language != "English" && tmp_language != "French") { // FIXME: add others
				tmp_language = "English";
			}
		}

		// Defaults
		util::JSON::Node *root = cfg->get_root();

		GLOBALS->key_action = root->get_nested_int("input>key_action", &GLOBALS->key_action, GLOBALS->key_action);
		GLOBALS->key_back = root->get_nested_int("input>key_back", &GLOBALS->key_back, GLOBALS->key_back);
		//GLOBALS->key_doughnut = root->get_nested_int("input>key_doughnut", &GLOBALS->key_doughnut, GLOBALS->key_doughnut);
		GLOBALS->key_die = root->get_nested_int("input>key_die", &GLOBALS->key_die, GLOBALS->key_die);
		GLOBALS->key_l = root->get_nested_int("input>key_l", &GLOBALS->key_l, GLOBALS->key_l);
		GLOBALS->key_r = root->get_nested_int("input>key_r", &GLOBALS->key_r, GLOBALS->key_r);
		GLOBALS->key_u = root->get_nested_int("input>key_u", &GLOBALS->key_u, GLOBALS->key_u);
		GLOBALS->key_d = root->get_nested_int("input>key_d", &GLOBALS->key_d, GLOBALS->key_d);
		GLOBALS->joy_action = root->get_nested_int("input>joy_action", &GLOBALS->joy_action, GLOBALS->joy_action);
		GLOBALS->joy_back = root->get_nested_int("input>joy_back", &GLOBALS->joy_back, GLOBALS->joy_back);
		GLOBALS->joy_doughnut = root->get_nested_int("input>joy_doughnut", &GLOBALS->joy_doughnut, GLOBALS->joy_doughnut);
		GLOBALS->joy_die = root->get_nested_int("input>joy_die", &GLOBALS->joy_die, GLOBALS->joy_die);
		GLOBALS->language = root->get_nested_string("misc>language", &GLOBALS->language, tmp_language);
		//autosave_help_shown = root->get_nested_bool("misc>autosave_help_shown", &autosave_help_shown, false);

		GLOBALS->onscreen_controller_was_enabled = root->get_nested_bool("wedge>use_onscreen_controller", &GLOBALS->onscreen_controller_was_enabled, GLOBALS->onscreen_controller_was_enabled);
#if !defined ANDROID && !(defined IOS && !defined TVOS)
		if (shim::force_tablet == false) {
			GLOBALS->onscreen_controller_was_enabled = false;
		}
#endif
		//GLOBALS->prerendered_music = root->get_nested_bool("wedge>prerendered_music", &GLOBALS->prerendered_music, GLOBALS->prerendered_music);
		GLOBALS->rumble_enabled = root->get_nested_bool("wedge>rumble_enabled", &GLOBALS->rumble_enabled, GLOBALS->rumble_enabled);

		GLOBALS->load_translation();

#ifdef GOOGLE_PLAY
		start_google_play_games_services();
#endif

		if (wedge::go() == false) {
			util::debugmsg("wedge::go return false.\n");
			return 1;
		}

		save_settings();

		wedge::end();

		end_autosave_thread();
	
		shim::static_end();
	}
	catch (util::Error &e) {
		util::errormsg("Fatal error: %s\n", e.error_message.c_str());
		gui::fatalerror("Fatal Error", e.error_message, gui::OK);
		return 1;
	}

#if defined IOS || defined ANDROID
	exit(0);
#endif

	return 0;
}
