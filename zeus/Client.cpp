/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <enet/TcpClient.hpp>
#include <zeus/Client.hpp>
#include <zeus/debug.hpp>



zeus::Client::Client() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &zeus::Client::onPropertyChangeIp),
  propertyPort(this, "port", 1983, "Port to connect server", &zeus::Client::onPropertyChangePort) {
	
}

zeus::Client::~Client() {
	
}

void zeus::Client::onClientData(ememory::SharedPtr<zeus::Buffer> _value) {
	if (_value == nullptr) {
		return;
	}
	ZEUS_ERROR("Get Data On the Communication interface that is not understand ... : " << _value);
}

zeus::ServiceRemote zeus::Client::getService(const std::string& _name) {
	ZEUS_TODO("Lock here");
	auto it = m_listConnectedService.begin();
	while (it != m_listConnectedService.end()) {
		ememory::SharedPtr<zeus::ServiceRemoteBase> val = it->lock();
		if (val == nullptr) {
			it = m_listConnectedService.erase(it);
			continue;
		}
		if (val->getName() == _name) {
			return zeus::ServiceRemote(val);
		}
	}
	ememory::SharedPtr<zeus::ServiceRemoteBase> tmp = ememory::makeShared<zeus::ServiceRemoteBase>(m_interfaceClient, _name);
	m_listConnectedService.push_back(tmp);
	return zeus::ServiceRemote(tmp);
}

void zeus::Client::onPropertyChangeIp() {
	disconnect();
}

void zeus::Client::onPropertyChangePort(){
	disconnect();
}


bool zeus::Client::connectTo(const std::string& _address) {
	ZEUS_DEBUG("connect [START]");
	disconnect();
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort));
	m_interfaceClient = ememory::makeShared<zeus::WebServer>();
	if (m_interfaceClient == nullptr) {
		ZEUS_ERROR("Allocate connection error");
		return false;
	}
	ZEUS_WARNING("Request connect user " << _address);
	m_interfaceClient->connect(this, &zeus::Client::onClientData);
	m_interfaceClient->setInterface(std::move(connection), false, _address);
	m_interfaceClient->connect();
	/*
	ZEUS_WARNING("Request connect user " << _address);
	zeus::Future<bool> ret = call("connectToUser", _address, "zeus-client");
	ret.wait();
	if (ret.hasError() == true) {
		ZEUS_WARNING("Can not connect to user named: '" << _address << "' ==> return error");
		ZEUS_WARNING("    error: '" << ret.getErrorType() << "'");
		ZEUS_WARNING("    help: '" << ret.getErrorHelp() << "'");
		return false;
	}
	if (ret.get() == true) {
		ZEUS_WARNING("    ==> accepted connection");
	} else {
		ZEUS_WARNING("    ==> Refuse connection");
	}
	return ret.get();
	*/
	// TODO: Check if connection is retruen OK or arror ...
	return true;
}

bool zeus::Client::connect(const std::string& _address) {
	bool ret = connectTo(_address);
	if (ret==false) {
		return false;
	}
	zeus::Future<bool> retIdentify = call("anonymous").wait();
	if (retIdentify.hasError() == true) {
		disconnect();
		return false;
	}
	if (retIdentify.get() == false) {
		disconnect();
	}
	return retIdentify.get();
}

bool zeus::Client::connect(const std::string& _address, const std::string& _userPassword) {
	bool ret = connectTo(_address);
	if (ret==false) {
		return false;
	}
	zeus::Future<bool> retIdentify = call("auth", _userPassword).wait();
	if (retIdentify.hasError() == true) {
		disconnect();
		return false;
	}
	if (retIdentify.get() == false) {
		disconnect();
	}
	return retIdentify.get();
}

bool zeus::Client::connect(const std::string& _address, const std::string& _clientName, const std::string& _clientTocken) {
	bool ret = connectTo(_address);
	if (ret==false) {
		return false;
	}
	zeus::Future<bool> retIdentify = call("identify", _clientName, _clientTocken).wait();
	if (retIdentify.hasError() == true) {
		disconnect();
		return false;
	}
	if (retIdentify.get() == false) {
		disconnect();
	}
	return retIdentify.get();
}

void zeus::Client::disconnect() {
	ZEUS_DEBUG("disconnect [START]");
	if (m_interfaceClient != nullptr) {
		m_interfaceClient->disconnect();
		m_interfaceClient.reset();
	} else {
		ZEUS_VERBOSE("Nothing to disconnect ...");
	}
	ZEUS_DEBUG("disconnect [STOP]");
}
