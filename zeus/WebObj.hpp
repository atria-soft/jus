/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

//#include <zeus/WebServer.hpp>
#include <etk/types.hpp>
#include <ememory/memory.hpp>
#include <zeus/message/Message.hpp>

namespace zeus {
	class WebServer;
	class WebObj : public ememory::EnableSharedFromThis<zeus::WebObj> {
		protected:
			ememory::SharedPtr<zeus::WebServer> m_interfaceWeb;
			uint16_t m_id;
			uint16_t m_objectId;
			std::vector<uint32_t> m_listRemoteConnected;
		public:
			WebObj(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _id, uint16_t _objectId);
			virtual ~WebObj();
			uint16_t getInterfaceId();
			uint16_t getObjectId();
			uint32_t getFullId();
			virtual void receive(ememory::SharedPtr<zeus::Message> _value);
			virtual void display();
			void addRemote(uint32_t _id);
			bool removeOwnership(uint32_t _id);
			void rmRemoteInterface(uint16_t _id);
			bool haveRemoteConnected() const;
			bool transferOwnership(uint32_t _sourceAddress, uint32_t _destinataireAddress);
		
	};
}

