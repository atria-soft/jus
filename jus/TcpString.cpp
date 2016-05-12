/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/TcpString.h>
#include <jus/debug.h>

jus::TcpString::TcpString() :
  propertyIp(this, "ip", "127.0.0.1", "ip to open or connect server", &jus::TcpString::onPropertyChangeIp),
  propertyPort(this, "port", 1983, "Connection port of the server", &jus::TcpString::onPropertyChangePort),
  propertyServer(this, "server", false, "is a server or not", &jus::TcpString::onPropertyChangeServer) {
	m_connection.setHostNane(*propertyIp);
	m_connection.setPort(*propertyPort);
	m_connection.setServer(*propertyServer);
}

jus::TcpString::~TcpString() {
	
}

void jus::TcpString::connect(){
	if (m_connection.link() == false) {
		JUS_ERROR("can not connect to the socket...");
	}
}

void jus::TcpString::disconnect(){
	if (m_connection.unlink() == false) {
		JUS_ERROR("can not disconnect to the socket...");
	}
}

int32_t jus::TcpString::write(const std::string& _data) {
	if (_data.size() == 0) {
		return 0;
	}
	uint32_t size = _data.size();
	m_connection.write(&size, 4);
	return m_connection.write(_data.c_str(), _data.size());
}

std::string jus::TcpString::read() {
	std::string out;
	uint32_t size = 0;
	int32_t len = m_connection.read(&size, 4);
	if (len != 4) {
		JUS_ERROR("Protocol error occured ...");
	} else {
		out.resize(size);
		len = m_connection.read(&out[0], size);
		if (len != 4) {
			JUS_ERROR("Protocol error occured .2.");
		}
	}
	return out;
}

void jus::TcpString::onPropertyChangeIp() {
	m_connection.setHostNane(*propertyIp);
}

void jus::TcpString::onPropertyChangePort() {
	m_connection.setPort(*propertyPort);
}

void jus::TcpString::onPropertyChangeServer() {
	m_connection.setServer(*propertyServer);
}

