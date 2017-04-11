/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once

#include <ewol/widget/Windows.hpp>
#include <ewol/widget/Composer.hpp>

namespace appl {
	class Windows;
	using WindowsShared = ememory::SharedPtr<appl::Windows>;
	using WindowsWeak = ememory::WeakPtr<appl::Windows>;
	class Windows : public ewol::widget::Windows {
		protected:
			Windows();
			void init();
			ewol::widget::ComposerShared m_composer;
			ememory::SharedPtr<ClientProperty> m_clientProp;
		public:
			DECLARE_FACTORY(Windows);
		public: // callback functions
			void onCallbackReboot();
			void onCallbackShutdown();
			void load_db();
			void store_db();
			
			void onCallbackConnectionValidate(const ememory::SharedPtr<ClientProperty>& _prop);
			void onCallbackConnectionCancel();
			void onCallbackShortCut(const std::string& _value);
			void onCallbackMenuEvent(const std::string& _value);
	};
}

