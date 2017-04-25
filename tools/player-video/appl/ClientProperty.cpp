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
	m_address = "127.0.0.1";
	m_port = 1983;
}

ejson::Object appl::ClientProperty::toJson() {
	ejson::Object out;
	out.add("user", ejson::String(m_fromUser));
	out.add("pass", ejson::String(m_pass));
	out.add("address", ejson::String(m_address));
	out.add("port", ejson::Number(m_port));
	return out;
}

void appl::ClientProperty::fromJson(ejson::Object _obj) {
	m_fromUser = _obj["user"].toString().get();
	m_toUser = m_fromUser;
	m_pass = _obj["pass"].toString().get();
	m_address = _obj["address"].toString().get();
	m_port = _obj["port"].toNumber().getU64();
}

void appl::ClientProperty::connect() {
	if (m_connection.isAlive() == true) {
		m_connection.pingIsAlive();
		if (m_connection.isAlive() == true) {
			return;
		}
	}
	// Generate IP and Port in the client interface
	if (m_address == "") {
		m_connection.propertyIp.set("127.0.0.1");
	} else {
		m_connection.propertyIp.set(m_address);
	}
	if (m_port == 0) {
		m_connection.propertyPort.set(1983);
	} else {
		m_connection.propertyPort.set(m_port);
	}
	// Connection depending on the mode requested
	if (m_fromUser == m_toUser) {
		bool ret = m_connection.connect(m_fromUser, m_pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Authentify with '" << m_toUser << "'");
			return;
		} else {
			APPL_INFO("    ==> Authentify with '" << m_toUser << "'");
		}
	} else if (m_fromUser != "") {
		bool ret = m_connection.connect(m_fromUser, m_toUser, m_pass);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected to '" << m_toUser << "' with '" << m_fromUser << "'");
			return;
		} else {
			APPL_INFO("    ==> Connected with '" << m_toUser << "' with '" << m_fromUser << "'");
		}
	} else {
		bool ret = m_connection.connect(m_toUser);
		if (ret == false) {
			APPL_ERROR("    ==> NOT Connected with 'anonymous' to '" << m_toUser << "'");
			return;
		} else {
			APPL_INFO("    ==> Connected with 'anonymous' to '" << m_toUser << "'");
		}
	}
}

void appl::ClientProperty::setLogin(std::string _login) {
	m_fromUser = "";
	m_toUser = "";
	m_address = "";
	m_port = 0;
	// separate loggin and IP adress ...
	std::string login;
	std::vector<std::string> listElem = etk::split(_login, '~');
	if (listElem.size() == 0) {
		APPL_ERROR("Not enouth element in the login ...");
		return;
	}
	m_fromUser = listElem[0];
	m_toUser = m_fromUser;
	if (listElem.size() == 1) {
		// connnect on local host ... nothing to do
	} else {
		std::vector<std::string> listElem2 = etk::split(listElem[1], ':');
		if (listElem2.size() >= 1) {
			m_address = listElem2[0];
		}
		if (listElem2.size() >= 2) {
			m_port = etk::string_to_uint32_t(listElem2[1]);
		}
	}
}

std::string appl::ClientProperty::getLogin() {
	std::string out = m_fromUser;
	bool hasTild = false;
	if (m_address != "") {
		if (hasTild == false) {
			out += "~" ;
			hasTild = true;
		}
		out += m_address;
	}
	if (    m_port != 1983
	     && m_port != 0) {
		if (hasTild == false) {
			out += "~" ;
			hasTild = true;
		}
		out += ":" + etk::to_string(m_port);
	}
	return out;
}

void appl::ClientProperty::setPassword(std::string _password) {
	m_pass = _password;
}

std::string appl::ClientProperty::getPassword() {
	return m_pass;
}

#include <esignal/details/Signal.hxx>
ESIGNAL_DECLARE_SIGNAL(ememory::SharedPtr<appl::ClientProperty>);

