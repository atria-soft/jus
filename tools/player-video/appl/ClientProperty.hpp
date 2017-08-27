/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#pragma once

#include <ewol/widget/WidgetScrolled.hpp>
#include <ewol/compositing/Image.hpp>
#include <ewol/compositing/Text.hpp>
#include <ewol/compositing/Drawing.hpp>
#include <ewol/widget/Manager.hpp>
#include <esignal/Signal.hpp>

#include <zeus/Client.hpp>
#include <ejson/ejson.hpp>

namespace appl {
	class ClientProperty {
		protected:
			etk::String m_fromUser;
			etk::String m_toUser;
			etk::String m_pass;
			etk::String m_address;
			uint16_t m_port;
			zeus::Client m_connection;
		public:
			ClientProperty();
			
			zeus::Client& getConnection() {
				return m_connection;
			};
			void connect();
			void disconnect();
			ejson::Object toJson();
			void fromJson(ejson::Object _obj);
			void setLogin(etk::String _login);
			etk::String getLogin();
			void setPassword(etk::String _password);
			etk::String getPassword();
	};
}

