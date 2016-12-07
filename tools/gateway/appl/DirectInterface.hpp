/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <appl/GateWay.hpp>
#include <appl/IOInterface.hpp>

namespace appl {
	class GateWay;
	class DirectInterface : public appl::IOInterface {
		public:
			zeus::WebServer m_interfaceWeb;
		public:
			DirectInterface(enet::Tcp _connection);
			virtual ~DirectInterface();
			bool start(appl::GateWay* _gateway);
			void receive(ememory::SharedPtr<zeus::Message> _data);
			void send(ememory::SharedPtr<zeus::Message> _data);
			bool requestURI(const std::string& _uri);
			//void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			zeus::WebServer* getInterface() {
				return &m_interfaceWeb;
			}
			bool isConnected() { return m_interfaceWeb.isActive(); };
	};
}

