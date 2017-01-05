/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <appl/GateWay.hpp>

namespace appl {
	class GateWay;
	enum class clientState {
		unconnect, // starting sate
		connect, // zeus get a TCP connection (through the Router)
		connectDirect, // zeus get a TCP connection 5direct connection on the gateway)
		clientIdentify, // client defien the mode of the acces (anonymous,client/user)
		disconnect // client is dead or loal disconnection
	};
	class IOInterface : public ememory::EnableSharedFromThis<appl::IOInterface> {
		public:
			appl::GateWay* m_gateway;
		protected:
			uint16_t m_uid; //!< Client unique ID (for routing)
			std::vector<std::string> m_listService;
		public:
			const std::vector<std::string>& getServiceList();
		public:
			enum clientState m_state; // state machine ...
			IOInterface();
			virtual ~IOInterface();
			bool start(appl::GateWay* _gateway, uint16_t _id);
			// Data arrive from the IO
			virtual void receive(ememory::SharedPtr<zeus::Message> _value);
			// Data must be send to the IO
			virtual void send(ememory::SharedPtr<zeus::Message> _data) = 0;
			// Verify wich ID is provided by the IO
			bool checkId(uint16_t _id) const {
				return m_uid == _id;
			}
			uint16_t getId() const {
				return m_uid;
			}
			void answerProtocolError(uint32_t _transactionId, const std::string& _errorHelp);
			virtual zeus::WebServer* getInterface() = 0;
			virtual bool isConnected() { return false; };
	};
}

