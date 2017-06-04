/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <enet/TcpClient.hpp>
#include <zeus/Client.hpp>
#include <zeus/debug.hpp>

static const std::string protocolError = "PROTOCOL-ERROR";


void zeus::Client::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceWeb->answerError(_transactionId, 0, ZEUS_ID_SERVICE_ROOT, protocolError, _errorHelp);
	m_interfaceWeb->disconnect();
	ZEUS_TODO("Do this error return ... " << _errorHelp);
}

zeus::Client::Client() :
  propertyIp(this, "ip", "127.0.0.1", "Ip to connect server", &zeus::Client::onPropertyChangeIp),
  propertyPort(this, "port", 1983, "Port to connect server", &zeus::Client::onPropertyChangePort) {
	
}

zeus::Client::~Client() {
	
}

void zeus::Client::onClientData(ememory::SharedPtr<zeus::Message> _value) {
	if (_value == nullptr) {
		return;
	}
	ZEUS_DEBUG("receive message : " << _value);
	// TODO : We will receive here some notification and call ...like : 
	/*
	if (call && id = 0 && objectid == 0) {
		we will have :
		if call == "ValidateConnection"
		in param : local interface ID (can not change it...)
		local name like clientname::subID (if multiple connection in parallele)
		... and after we can do many thing like provide servies ...
	}
	*/
	// TODO: all the basic checks ...
	
	if (_value == nullptr) {
		return;
	}
	//APPL_ERROR("    ==> parse DATA ...");
	uint32_t transactionId = _value->getTransactionId();
	if (transactionId == 0) {
		ZEUS_ERROR("Protocol error ==>missing id");
		answerProtocolError(transactionId, "missing parameter: 'id'");
		return;
	}
	// Check if we are the destinated Of this message 
	if (_value->getDestinationId() != m_interfaceWeb->getAddress()) {
		ZEUS_ERROR("Protocol error ==> Wrong ID of the interface " << _value->getDestinationId() << " != " << m_interfaceWeb->getAddress());
		answerProtocolError(transactionId, "wrong adress: request " + etk::to_string(_value->getDestinationId()) + " have " + etk::to_string(m_interfaceWeb->getAddress()));
		return;
	}
	if (_value->getDestinationObjectId() == ZEUS_ID_GATEWAY_OBJECT) {
		if (_value->getType() == zeus::message::type::call) {
			ememory::SharedPtr<zeus::message::Call> callObj = ememory::staticPointerCast<zeus::message::Call>(_value);
			std::string callFunction = callObj->getCall();
			if (callFunction == "link") {
				// link with a specific service:
				std::string serviceName = callObj->getParameter<std::string>(0);
				for (auto &it : m_listServicesAvaillable) {
					if (it.first == serviceName) {
						ZEUS_INFO("find service : " << it.first);
						// Create new service object
						it.second(transactionId, m_interfaceWeb, _value->getSource());
						// TODO : Do it better ...
						
						/*
						// Check if it is not already connected to this service, if it is true ==> reject IT
						
						// Create new service object
						uint16_t tmpId = m_interfaceWeb->getNewObjectId();
						ememory::SharedPtr<zeus::Object> newService = it.second(this, tmpId, _value->getSourceId());
						// TODO : Do it better ...
						
						//m_listProvicedService.push_back(newService);
						// Return the Value of the object service .... this is really bad, Maybe add a message type for this...
						m_interfaceWeb->answerValue(transactionId, _value->getDestination(), _value->getSource(), (uint32_t(m_interfaceWeb->getAddress())<<16)+tmpId);
						*/
						return;
					}
				}
				m_interfaceWeb->answerError(transactionId, _value->getDestination(), _value->getSource(), "UNKNOW-SERVICE");
				return;
			} else if (callFunction == "unlink") {
				uint32_t objectAddress = callObj->getParameter<uint32_t>(0);
				ZEUS_VERBOSE("unlink requested of " << (objectAddress>>16) << "/" << (objectAddress&0xFFFF) << "  local interface adress : " << m_interfaceWeb->getAddress());
				if ((objectAddress>> 16) != m_interfaceWeb->getAddress()) {
					m_interfaceWeb->answerError(transactionId, _value->getDestination(), _value->getSource(), "REMOVE-OWNERSHIP-WRONG-INTERFACE");
					return;
				}
				bool ret = m_interfaceWeb->removeObjectOwnership(objectAddress&0xFFFF, _value->getSource());
				if (ret == true) {
					m_interfaceWeb->answerVoid(transactionId, _value->getDestination(), _value->getSource());
					return;
				}
				m_interfaceWeb->answerError(transactionId, _value->getDestination(), _value->getSource(), "REMOVE-OWNERSHIP-ERROR", "the object does not exist ...");
				return;
			} else if (callFunction == "movelink") {
				uint32_t objectAddress = callObj->getParameter<uint32_t>(0);
				uint32_t destinataireAddress = callObj->getParameter<uint32_t>(1);
				if ((objectAddress>> 16) != m_interfaceWeb->getAddress()) {
					m_interfaceWeb->answerError(transactionId, _value->getDestination(), _value->getSource(), "TRANSFER-OWNERSHIP-WRONG-INTERFACE");
					return;
				}
				bool ret = m_interfaceWeb->transferRemoteObjectOwnership(objectAddress, _value->getSource(), destinataireAddress);
				if (ret == true) {
					m_interfaceWeb->answerVoid(transactionId, _value->getDestination(), _value->getSource());
					return;
				}
				m_interfaceWeb->answerError(transactionId, _value->getDestination(), _value->getSource(), "TRANSFER-OWNERSHIP-ERROR");
				return;
			} else if (callFunction == "removeInterface") {
				ZEUS_VERBOSE("Remove Object : " << callObj);
				m_interfaceWeb->interfaceRemoved(callObj->getParameter<std::vector<uint16_t>>(0));
				return;
			}
			answerProtocolError(transactionId, "interact with client, musty only call: link/unlink/movelink/removeInterface");
			return;
		}
		m_interfaceWeb->answerError(transactionId, _value->getDestination(), _value->getSource(), "UNKNOW-ACTION");
		return;
	}
	// find the object to communicate the adress to send value ...
	uint16_t objId = _value->getDestinationObjectId();
	ZEUS_ERROR("Get Data On the Communication interface that is not understand ... : " << _value);
}

bool zeus::Client::serviceAdd(const std::string& _serviceName, factoryService _factory) {
	// Check if we can provide new service:
	zeus::Future<bool> futValidate = m_interfaceWeb->call(uint32_t(m_interfaceWeb->getAddress())<<16, ZEUS_GATEWAY_ADDRESS, "serviceAdd", _serviceName);
	futValidate.wait(); // TODO: Set timeout ...
	if (futValidate.hasError() == true) {
		ZEUS_ERROR("Can not provide a new sevice ... '" << futValidate.getErrorType() << "' help:" << futValidate.getErrorHelp());
		return false;
	}
	m_listServicesAvaillable.insert(std::make_pair(_serviceName, _factory));
	return true;
}

bool zeus::Client::serviceRemove(const std::string& _serviceName) {
	// Check if we can provide new service:
	zeus::Future<bool> futValidate = m_interfaceWeb->call(uint32_t(m_interfaceWeb->getAddress())<<16, ZEUS_GATEWAY_ADDRESS, "serviceRemove", _serviceName);
	futValidate.wait(); // TODO: Set timeout ...
	if (futValidate.hasError() == true) {
		ZEUS_ERROR("Can not provide a new sevice ... '" << futValidate.getErrorType() << "' help:" << futValidate.getErrorHelp());
		return false;
	}
	ZEUS_TODO("remove service : " << _serviceName);
	return true;
}

zeus::ObjectRemote zeus::Client::getService(const std::string& _name) {
	ZEUS_TODO("Lock here");
	auto it = m_listConnectedService.begin();
	while (it != m_listConnectedService.end()) {
		ememory::SharedPtr<zeus::ObjectRemoteBase> val = it->lock();
		if (val == nullptr) {
			it = m_listConnectedService.erase(it);
			continue;
		}
		if (val->getName() == _name) {
			return zeus::ObjectRemote(val);
		}
	}
	// little hack : Call the service manager with the service ID=0 ...
	zeus::Future<ememory::SharedPtr<zeus::ObjectRemoteBase>> ret = m_interfaceWeb->call(uint32_t(m_interfaceWeb->getAddress())<<16, ZEUS_GATEWAY_ADDRESS, "link", _name);
	ret.wait();
	if (ret.hasError() == true) {
		ZEUS_WARNING("Can not unlink with the service id: '" << _name << "' ==> link error");
		return zeus::ObjectRemote();
	}
	return zeus::ObjectRemote(ret.get());
}


zeus::Future<int32_t> zeus::Client::getServiceCount() {
	return call(ZEUS_NO_ID_OBJECT, ZEUS_ID_GATEWAY, "getServiceCount");
}

zeus::Future<std::vector<std::string>> zeus::Client::getServiceList() {
	return call(ZEUS_NO_ID_OBJECT, ZEUS_ID_GATEWAY, "getServiceList");
}

// TODO : This is an active waiting ==> this is bad ... ==> use future, it will be better
bool zeus::Client::waitForService(const std::string& _serviceName) {
	int32_t delayMax = 10;
	while (delayMax > 0) {
		auto listValues = getServiceList();
		listValues.wait();
		for (auto &it: listValues.get()) {
			if (it == _serviceName) {
				return true;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		delayMax--;
	}
	return false;
}


void zeus::Client::onPropertyChangeIp() {
	disconnect();
}

void zeus::Client::onPropertyChangePort(){
	disconnect();
}


bool zeus::Client::connectTo(const std::string& _address, echrono::Duration _timeOut) {
	ZEUS_DEBUG("connect [START]");
	disconnect();
	enet::Tcp connection = std::move(enet::connectTcpClient(*propertyIp, *propertyPort, 1));
	m_interfaceWeb = ememory::makeShared<zeus::WebServer>();
	if (m_interfaceWeb == nullptr) {
		ZEUS_ERROR("Allocate connection error");
		return false;
	}
	ZEUS_WARNING("Request connect user " << _address);
	m_interfaceWeb->connect(this, &zeus::Client::onClientData);
	m_interfaceWeb->setInterface(std::move(connection), false, _address);
	m_interfaceWeb->connect();
	
	zeus::Future<uint16_t> retIdentify = call(0, ZEUS_ID_GATEWAY, "getAddress").wait();
	if (retIdentify.hasError() == true) {
		ZEUS_ERROR("Has error when getting Address of client ... " << retIdentify.getErrorType() << " ? " << retIdentify.getErrorHelp());
		disconnect();
		return false;
	}
	m_interfaceWeb->setAddress(retIdentify.get());
	
	/*
	ZEUS_WARNING("Now, we get information relative with our name and adress" << _address);
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

bool zeus::Client::connect(echrono::Duration _timeOut) {
	bool ret = connectTo("directIO", _timeOut);
	if (ret==false) {
		return false;
	}
	zeus::Future<bool> retIdentify = call(0, ZEUS_ID_GATEWAY, "service").waitFor(_timeOut);
	if (retIdentify.hasError() == true) {
		disconnect();
		return false;
	}
	if (retIdentify.get() == false) {
		disconnect();
	}
	return retIdentify.get();
}

bool zeus::Client::connect(const std::string& _address, echrono::Duration _timeOut) {
	m_clientName = _address;
	bool ret = connectTo(_address, _timeOut);
	if (ret==false) {
		return false;
	}
	zeus::Future<bool> retIdentify = call(0, ZEUS_ID_GATEWAY, "anonymous").waitFor(_timeOut);
	if (retIdentify.hasError() == true) {
		disconnect();
		return false;
	}
	if (retIdentify.get() == false) {
		disconnect();
	}
	return retIdentify.get();
}

bool zeus::Client::connect(const std::string& _address, const std::string& _userPassword, echrono::Duration _timeOut) {
	m_clientName = _address;
	bool ret = connectTo(_address, _timeOut);
	if (ret==false) {
		return false;
	}
	zeus::Future<bool> retIdentify = call(0, ZEUS_ID_GATEWAY, "auth", _userPassword).waitFor(_timeOut);
	if (retIdentify.hasError() == true) {
		disconnect();
		return false;
	}
	if (retIdentify.get() == false) {
		disconnect();
	}
	return retIdentify.get();
}

bool zeus::Client::connect(const std::string& _address, const std::string& _clientName, const std::string& _clientTocken, echrono::Duration _timeOut) {
	m_clientName = _clientName;
	bool ret = connectTo(_address, _timeOut);
	if (ret==false) {
		return false;
	}
	zeus::Future<bool> retIdentify = call(0, ZEUS_ID_GATEWAY, "identify", _clientName, _clientTocken).waitFor(_timeOut);
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
	if (m_interfaceWeb != nullptr) {
		m_interfaceWeb->disconnect();
		m_interfaceWeb.reset();
	} else {
		ZEUS_VERBOSE("Nothing to disconnect ...");
	}
	ZEUS_DEBUG("disconnect [STOP]");
}

bool zeus::Client::isAlive() {
	if (m_interfaceWeb == nullptr) {
		return false;
	}
	return m_interfaceWeb->isActive();
}

void zeus::Client::pingIsAlive() {
	if (echrono::Steady::now() - m_interfaceWeb->getLastTimeSend() >= echrono::seconds(30)) {
		m_interfaceWeb->ping();
	}
}

void zeus::Client::displayConnectedObject() {
	if (m_interfaceWeb== nullptr) {
		return;
	}
	m_interfaceWeb->listObjects();
}


void zeus::Client::cleanDeadObject() {
	if (m_interfaceWeb== nullptr) {
		return;
	}
	m_interfaceWeb->cleanDeadObject();
}

