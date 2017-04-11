/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#include <etk/types.hpp>
#include <ewol/ewol.hpp>
#include <gale/context/commandLine.hpp>


#include <appl/debug.hpp>
#include <appl/Windows.hpp>
#include <ewol/object/Object.hpp>
#include <ewol/widget/Manager.hpp>
#include <ewol/context/Context.hpp>
#include <zeus/zeus.hpp>

namespace appl {
	class MainApplication : public ewol::context::Application {
		public:
			void onCreate(ewol::Context& _context) override {
				APPL_INFO("==> CREATE ... " PROJECT_NAME " (BEGIN)");
				localCreate(_context);
				APPL_INFO("==> CREATE ... " PROJECT_NAME " (END)");
			}
			void localCreate(ewol::Context& _context) {
				// parse all the argument of the application
				for (int32_t iii=0 ; iii<_context.getCmd().size(); iii++) {
					std::string tmpppp = _context.getCmd().get(iii);
					if (    tmpppp == "-h"
					     || tmpppp == "--help") {
						APPL_INFO("  -h/--help display this help" );
						exit(0);
					}
				}
				// TODO : Remove this: Move if in the windows properties
				_context.setSize(vec2(800, 600));
				// eneble the search of the font in the system font path
				_context.getFontDefault().setUseExternal(true);
				// select font preference of der with a basic application size
				_context.getFontDefault().set("DejaVuSerif;FreeSerif;DejaVuSansMono", 19);
				// Create the windows
				ememory::SharedPtr<appl::Windows> basicWindows = appl::Windows::create();
				// configure the ewol context to use the new windows
				_context.setWindows(basicWindows);
			}
	};
}

/**
 * @brief Main of the program (This can be set in every case, but it is not used in Andoid...).
 * @param std IO
 * @return std IO
 */
int main(int _argc, const char *_argv[]) {
	audio::river::init();
	zeus::init(_argc, _argv);
	return ewol::run(new appl::MainApplication(), _argc, _argv);
}

