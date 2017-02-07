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
		public:
			std::string fromUser;
			std::string toUser;
			std::string pass;
			std::string address;
			uint16_t port;
			zeus::Client connection;
			ClientProperty();
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

