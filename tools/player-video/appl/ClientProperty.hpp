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
			std::string m_fromUser;
			std::string m_toUser;
			std::string m_pass;
			std::string m_address;
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
			void setLogin(std::string _login);
			std::string getLogin();
			void setPassword(std::string _password);
			std::string getPassword();
	};
}

