/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <appl/GateWay.hpp>

namespace appl {
	class GateWay;
	enum class clientState {
		unconnect, // starting sate
		connect, // zeust get a TCP connection
		clientIdentify, // client defien the mode of the acces (anonymous,client/user)
		disconnect // client is dead or loal disconnection
	};
	class IOInterface {
		public:
			appl::GateWay* m_gateway;
		protected:
			uint16_t m_uid; //!< Client unique ID (for routing)
		public:
			enum clientState m_state; // state machine ...
			IOInterface();
			virtual ~IOInterface();
			bool start(appl::GateWay* _gateway, uint16_t _id);
			// Data arrive from the IO
			virtual void receive(ememory::SharedPtr<zeus::Buffer> _value);
			// Data must be send to the IO
			virtual void send(ememory::SharedPtr<zeus::Buffer> _data) = 0;
			// Verify wich ID is provided by the IO
			bool checkId(uint16_t _id) const {
				return m_uid == _id;
			}
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			virtual zeus::WebServer* getInterface() = 0;
	};
}

