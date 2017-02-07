/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#include <appl/debug.hpp>
#include <ewol/widget/Widget.hpp>



#include <appl/ClientProperty.hpp>
#include <appl/debug.hpp>
#include <appl/widget/ListViewer.hpp>
#include <ewol/object/Manager.hpp>
#include <etk/tool.hpp>

#include <egami/egami.hpp>
#include <zeus/zeus.hpp>
#include <zeus/Client.hpp>
#include <zeus/service/ProxyVideo.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/ObjectRemote.hpp>
#include <echrono/Steady.hpp>
#include <zeus/FutureGroup.hpp>
#include <etk/stdTools.hpp>
#include <ejson/ejson.hpp>

appl::ClientProperty::ClientProperty() {
	address = "127.0.0.1";
	port = 1983;
}

ejson::Object appl::ClientProperty::toJson() {
	ejson::Object out;
	out.add("user", ejson::String(fromUser));
	out.add("pass", ejson::String(pass));
	out.add("address", ejson::String(address));
	out.add("port", ejson::Number(port));
	return out;
}

void appl::ClientProperty::fromJson(ejson::Object _obj) {
	fromUser = _obj["user"].toString().get();
	toUser = fromUser;
	pass = _obj["pass"].toString().get();
	address = _obj["address"].toString().get();
	port = _obj["port"].toNumber().getU64();
}

void appl::ClientProperty::connect() {
	// Generate IP and Port in the client interface
	if (address == "") {
		connection.propertyIp.set("127.0.0.1");
	} else {
		connection.propertyIp.set(address);
	}
	if (port == 0) {
		connection.propertyPort.set(1983);
	} else {
		connection.propertyPort.set(port);
	}
	// Connection depending on the mode requested
	if (fromUser == toUser) {
		bool ret = connection.connect(fromUser, pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Authentify with '" << toUser << "'");
			return;
		} else {
			APPL_INFO("    ==> Authentify with '" << toUser << "'");
		}
	} else if (fromUser != "") {
		bool ret = connection.connect(fromUser, toUser, pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected to '" << toUser << "' with '" << fromUser << "'");
			return;
		} else {
			APPL_INFO("    ==> Connected with '" << toUser << "' with '" << fromUser << "'");
		}
	} else {
		bool ret = connection.connect(toUser);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected with 'anonymous' to '" << toUser << "'");
			return;
		} else {
			APPL_INFO("    ==> Connected with 'anonymous' to '" << toUser << "'");
		}
	}
}

void appl::ClientProperty::setLogin(std::string _login) {
	fromUser = "";
	toUser = "";
	// separate loggin and IP adress ...
	std::string login;
	std::vector<std::string> listElem = etk::split(_login, '~');
	if (listElem.size() == 0) {
		APPL_ERROR("Not enouth element in the login ...");
		return;
	}
	fromUser = listElem[0];
	toUser = listElem[0];
	if (listElem.size() == 1) {
		// connnect on local host ... nothing to do
	} else {
		std::vector<std::string> listElem2 = etk::split(listElem[1], ':');
		if (listElem2.size() >= 1) {
			address = listElem2[0];
		}
		if (listElem2.size() >= 2) {
			port = etk::string_to_uint32_t(listElem2[1]);
		}
	}
}

std::string appl::ClientProperty::getLogin() {
	std::string out = fromUser;
	bool hasTild = false;
	if (address != "") {
		if (hasTild == false) {
			out += "~" ;
			hasTild = true;
		}
		out += address;
	}
	if (    port != 1983
	     && port != 0) {
		if (hasTild == false) {
			out += "~" ;
			hasTild = true;
		}
		out += ":" + etk::to_string(port);
	}
	return out;
}

void appl::ClientProperty::setPassword(std::string _password) {
	pass = _password;
}

std::string appl::ClientProperty::getPassword() {
	return pass;
}

#include <esignal/details/Signal.hxx>
ESIGNAL_DECLARE_SIGNAL(ememory::SharedPtr<appl::ClientProperty>);

