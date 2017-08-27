/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#pragma once

#include <zeus/Client.hpp>
#include <ejson/ejson.hpp>

namespace appl {
	class ClientProperty {
		public:
			etk::String fromUser;
			etk::String toUser;
			etk::String pass;
			etk::String address;
			uint16_t port;
			zeus::Client connection;
			ClientProperty();
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

