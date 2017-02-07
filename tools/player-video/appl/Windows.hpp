/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */
#pragma once

#include <ewol/widget/Windows.hpp>
#include <ewol/widget/Composer.hpp>
#include <appl/widget/ListViewer.hpp>

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
			appl::widget::ListViewerShared m_listViewer;
			std::vector<std::string> m_list;
			int32_t m_id;
		public:
			DECLARE_FACTORY(Windows);
		public: // callback functions
			void onCallbackBack();
			void onCallbackPrevious();
			void onCallbackPlay(const bool& _isPressed);
			void onCallbackNext();
			void onCallbackFPS(const int32_t& _fps);
			void onCallbackPosition(const echrono::Duration& _time);
			void addFile(const std::string& _file);
			void onCallbackSeekRequest(const float& _value);
			
			
			void onCallbackConnectionValidate(const ememory::SharedPtr<ClientProperty>& _prop);
			void onCallbackConnectionCancel();
			
			void onCallbackShortCut(const std::string& _value);
			void onCallbackMenuEvent(const std::string& _value);
		protected:
			std::string m_login;
			std::string m_password;
			void onCallbackConnectLogin(const std::string& _value);
			void onCallbackConnectPassword(const std::string& _value);
			void onCallbackConnectConnect();
			void load_db();
			void store_db();
			void onCallbackSelectFilms();
			void onCallbackSelectAnnimation();
			void onCallbackSelectTVShow();
			void onCallbackSelectTvAnnimation();
			void onCallbackSelectTeather();
			void onCallbackSelectOneManShow();
			void onCallbackSelectSourses();
			void onCallbackSelectMedia(const uint32_t& _value);
	};
}

