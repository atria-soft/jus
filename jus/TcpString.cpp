/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#include <jus/TcpString.h>
#include <jus/debug.h>
#include <ethread/tools.h>
#include <unistd.h>

jus::TcpString::TcpString(enet::Tcp _connection) :
  m_connection(std::move(_connection)),
  m_thread(nullptr),
  signalIsConnected(),
  signalData() {
	m_threadRunning = false;
}

jus::TcpString::TcpString() :
  m_connection(),
  m_thread(nullptr),
  signalIsConnected(),
  signalData() {
	m_threadRunning = false;
}

void jus::TcpString::setInterface(enet::Tcp _connection) {
	m_connection = std::move(_connection);
}

jus::TcpString::~TcpString() {
	disconnect();
}

void jus::TcpString::setInterfaceName(const std::string& _name) {
	ethread::setName(*m_thread, "Tcp-" + _name);
}

void jus::TcpString::threadCallback() {
	ethread::setName("TcpString-input");
	// get datas:
	while (    m_threadRunning == true
	        && m_connection.getConnectionStatus() == enet::Tcp::status::link) {
		// READ section data:
		std::string data = std::move(read());
		JUS_VERBOSE("Receive data: '" << data << "'");
		if (data.size() != 0) {
			signalData.emit(data);
		}
	}
	JUS_DEBUG("End of thread");
}

void jus::TcpString::connect(bool _async){
	JUS_DEBUG("connect [START]");
	m_threadRunning = true;
	m_thread = new std::thread([&](void *){ this->threadCallback();}, nullptr);
	if (m_thread == nullptr) {
		m_threadRunning = false;
		JUS_ERROR("creating callback thread!");
		return;
	}
	while (    _async == false
	        && m_threadRunning == true
	        && m_connection.getConnectionStatus() != enet::Tcp::status::link) {
		usleep(50000);
	}
	//ethread::setPriority(*m_receiveThread, -6);
	if (_async == true) {
		JUS_DEBUG("connect [STOP] async mode");
	} else {
		JUS_DEBUG("connect [STOP]");
	}
}

void jus::TcpString::disconnect(){
	JUS_DEBUG("disconnect [START]");
	if (m_connection.getConnectionStatus() == enet::Tcp::status::link) {
		uint32_t size = 0xFFFFFFFF;
		m_connection.write(&size, 4);
	}
	if (m_thread != nullptr) {
		m_threadRunning = false;
	}
	if (m_connection.getConnectionStatus() != enet::Tcp::status::unlink) {
		m_connection.unlink();
	}
	if (m_thread != nullptr) {
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
	JUS_VERBOSE("Read [START]");
	std::string out;
	uint32_t size = 0;
	int32_t len = m_connection.read(&size, 4);
	if (len != 4) {
		JUS_ERROR("Protocol error occured ...");
	} else {
		if (size == -1) {
			JUS_WARNING("Remote close connection");
			m_threadRunning = false;
			//m_connection.unlink();
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
	}
	JUS_VERBOSE("Read [STOP]");
	return out;
}

