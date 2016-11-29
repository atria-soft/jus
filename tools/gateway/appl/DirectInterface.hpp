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
			~DirectInterface();
			bool start(appl::GateWay* _gateway, uint16_t _id);
			void receive(ememory::SharedPtr<zeus::Buffer> _data);
			void send(ememory::SharedPtr<zeus::Buffer> _data);
			bool requestURI(const std::string& _uri);
			//void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			zeus::WebServer* getInterface() {
				return &m_interfaceWeb;
			}
	};
}

