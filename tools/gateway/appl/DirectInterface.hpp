/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
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
			bool m_serviceAccess;
		public:
			DirectInterface(enet::Tcp _connection, bool _serviceAccess);
			virtual ~DirectInterface();
			bool start(appl::GateWay* _gateway);
			void receive(ememory::SharedPtr<zeus::Message> _data);
			void send(ememory::SharedPtr<zeus::Message> _data);
			etk::String requestURI(const etk::String& _uri, const etk::Map<etk::String,etk::String>& _options);
			//void answerProtocolError(uint32_t _transactionId, const etk::String& _errorHelp);
			zeus::WebServer* getInterface() {
				return &m_interfaceWeb;
			}
			bool isConnected() { return m_interfaceWeb.isActive(); };
	};
}

