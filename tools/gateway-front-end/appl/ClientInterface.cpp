/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/ClientInterface.hpp>
#include <zeus/Future.hpp>
#include <appl/GateWay.hpp>


#include <zeus/AbstractFunction.hpp>


static const std::string protocolError = "PROTOCOL-ERROR";

appl::ClientInterface::ClientInterface(enet::Tcp _connection, appl::GateWay* _gatewayInterface) :
  m_state(appl::ClientInterface::state::unconnect),
  m_gatewayInterface(_gatewayInterface),
  m_interfaceClient(std::move(_connection), true) {
	APPL_INFO("----------------");
	APPL_INFO("-- NEW Client --");
	APPL_INFO("----------------");
}

appl::ClientInterface::~ClientInterface() {
	APPL_INFO("Call All unlink ...");
	stop();
	APPL_INFO("-------------------");
	APPL_INFO("-- DELETE Client --");
	APPL_INFO("-------------------");
}

bool appl::ClientInterface::requestURI(const std::string& _uri) {
	APPL_WARNING("request connect on CLIENT: '" << _uri << "'");
	if(m_gatewayInterface == nullptr) {
		APPL_ERROR("Can not access to the main GateWay interface (nullptr)");
		return false;
	}
	std::string tmpURI = _uri;
	if (tmpURI.size() == 0) {
		APPL_ERROR("Empty URI ... not supported ...");
		return false;
	}
	if (tmpURI[0] == '/') {
		tmpURI = std::string(tmpURI.begin() + 1, tmpURI.end());
	}
	// TODO : Remove subParameters xxx?YYY
	m_userGateWay = m_gatewayInterface->get(tmpURI);
	if (m_userGateWay == nullptr) {
		APPL_ERROR("Can not connect on Client ==> it does not exist ...");
		return false;
	}
	APPL_INFO("Connect on client done : '" << tmpURI << "'");
	return true;
}

void appl::ClientInterface::start(uint64_t _uid, uint64_t _uid2) {
	m_uid = _uid;
	//m_uid2 = _uid2;
	m_state = appl::ClientInterface::state::connect;
	m_interfaceClient.connect(this, &appl::ClientInterface::onClientData);
	m_interfaceClient.connectUri(this, &appl::ClientInterface::requestURI);
	m_interfaceClient.connect(true);
	m_interfaceClient.setInterfaceName("cli-" + etk::to_string(m_uid));
}

void appl::ClientInterface::stop() {
	for (auto &it : m_listConnectedService) {
		if (it == nullptr) {
			continue;
		}
		it->m_interfaceClient.callClient(m_uid, "_delete");
	}
	/*
	if (m_userGateWay != nullptr) {
		m_userGateWay->m_interfaceClient.callClient(m_uid2, "_delete");
		m_userGateWay = nullptr;
	}
	*/
	m_listConnectedService.clear();
	m_interfaceClient.disconnect();
}

bool appl::ClientInterface::isAlive() {
	return m_interfaceClient.isActive();
}

void appl::ClientInterface::answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp) {
	m_interfaceClient.answerError(_transactionId, protocolError, _errorHelp);
	m_state = appl::ClientInterface::state::disconnect;
	m_interfaceClient.disconnect(true);
}


void appl::ClientInterface::onClientData(ememory::SharedPtr<zeus::Buffer> _value) {
	if (_value == nullptr) {
		return;
	}
	APPL_ERROR("receive data : " << _value);
	uint32_t transactionId = _value->getTransactionId();
	if (transactionId == 0) {
		APPL_ERROR("Protocol error ==>missing id");
		answerProtocolError(transactionId, "missing parameter: 'id'");
		return;
	}
	// Directly send to the user-GateWay
	if (m_userGateWay == nullptr) {
		APPL_ERROR("USER is not existing ...");
		answerProtocolError(transactionId, "GateWay error");
		// TODO : Need to kill socket ...
		return;
	}
	// Special case for data, they are transiting messages ...
	if (_value->getType() != zeus::Buffer::typeMessage::data) {
		m_userGateWay->m_interfaceClient.callForward(
		    m_uid,
		    _value,
		    (uint64_t(m_uid) << 32) + uint64_t(transactionId),
		    [=](zeus::FutureBase _ret) {
		    		ememory::SharedPtr<zeus::Buffer> tmpp = _ret.getRaw();
		    		if (tmpp == nullptr) {
		    			return true;
		    		}
		    		APPL_DEBUG("    ==> transmit : " << tmpp->getTransactionId() << " -> " << transactionId);
		    		APPL_DEBUG("    msg=" << tmpp);
		    		tmpp->setTransactionId(transactionId);
		    		//tmpp->setServiceId(serviceId+1);
		    		APPL_DEBUG("transmit=" << tmpp);
		    		m_interfaceClient.writeBinary(tmpp);
		    		// multiple send element ...
		    		return tmpp->getPartFinish();
		    });
	} else {
		// simply forward messages to the user gateWay ...
		m_userGateWay->m_interfaceClient.callForwardMultiple(
		    m_uid,
		    _value,
		    (uint64_t(m_uid) << 32) + uint64_t(transactionId));
		// TODO : Check errors ...
	}
}

void appl::ClientInterface::returnMessage(ememory::SharedPtr<zeus::Buffer> _data) {
	APPL_ERROR("Get call from the Service to the user ...");
}

