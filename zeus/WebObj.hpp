/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

//#include <zeus/WebServer.hpp>
#include <etk/types.hpp>
#include <ememory/memory.hpp>

namespace zeus {
	class WebServer;
	class WebObj : public ememory::EnableSharedFromThis<zeus::WebObj> {
		protected:
			ememory::SharedPtr<zeus::WebServer> m_interfaceWeb;
			uint16_t m_id;
			uint16_t m_objectId;
		public:
			WebObj(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _id, uint16_t _objectId) :
			  m_interfaceWeb(_iface),
			  m_id(_id),
			  m_objectId(_objectId) {
				
			}
			uint32_t getFullId() {
				return (uint32_t(m_id) << 16 ) + m_objectId;
			}
			virtual void receive(ememory::SharedPtr<zeus::Buffer> _value) {
				ZEUS_ERROR("Receive a message ==> not implemented magaging ..." << _value);
			}
		
	};
}

