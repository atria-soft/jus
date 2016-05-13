/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/TcpString.h>
#include <jus/debug.h>
#include <ethread/tools.h>

jus::TcpString::TcpString() :
  m_thread(nullptr),
  propertyIp(this, "ip", "127.0.0.1", "ip to open or connect server", &jus::TcpString::onPropertyChangeIp),
  propertyPort(this, "port", 1983, "Connection port of the server", &jus::TcpString::onPropertyChangePort),
  propertyServer(this, "server", false, "is a server or not", &jus::TcpString::onPropertyChangeServer),
  signalIsConnected(),
  signalData() {
	m_connection.setHostNane(*propertyIp);
	m_connection.setPort(*propertyPort);
	m_connection.setServer(*propertyServer);
	m_threadRunning = false;
}

jus::TcpString::~TcpString() {
	disconnect();
}

void jus::TcpString::threadCallback() {
	ethread::setName("TcpString-input");
	// Connect ...
	if (m_connection.link() == false) {
		JUS_ERROR("can not connect to the socket...");
		signalIsConnected.emit(false);
		return;
	}
	signalIsConnected.emit(true);
	// get datas:
	while (m_threadRunning == true) {
		// READ section data:
		std::string data = std::move(read());
		JUS_WARNING("Receive data: '" << data << "'");
		if (data.size() != 0) {
			signalData.emit(data);
		}
	}
	// disconnect ...
	if (m_connection.unlink() == false) {
		JUS_ERROR("can not disconnect to the socket...");
	}
	signalIsConnected.emit(false);
	JUS_DEBUG("End of thread");
}

void jus::TcpString::connect(){
	JUS_DEBUG("connect [START]");
	m_threadRunning = true;
	m_thread = new std::thread([&](void *){ this->threadCallback();}, nullptr);
	if (m_thread == nullptr) {
		m_threadRunning = false;
		JUS_ERROR("creating callback thread!");
		return;
	}
	//ethread::setPriority(*m_receiveThread, -6);
	JUS_DEBUG("connect [STOP]");
}

void jus::TcpString::disconnect(){
	JUS_DEBUG("disconnect [START]");
	if (m_thread != nullptr) {
		m_threadRunning = false;
		m_connection.unlink();
		m_thread->join();
		delete m_thread;
		m_thread = nullptr;
	}
	JUS_DEBUG("disconnect [STOP]");
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
	// TODO : Do it better with a correct way to check data size ...
	std::string out;
	uint32_t size = 0;
	int32_t len = m_connection.read(&size, 4);
	if (len != 4) {
		JUS_ERROR("Protocol error occured ...");
	} else {
		out.resize(size);
		len = m_connection.read(&out[0], size);
		if (len == 0) {
			JUS_WARNING("Read No data");
		} else if (len != size) {
			// TODO  do it again ...
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

