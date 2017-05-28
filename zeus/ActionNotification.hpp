/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <zeus/debug.hpp>


namespace zeus {
	class ActionNotification {
		private:
			ememory::SharedPtr<zeus::WebServer> m_interface;
			uint32_t m_transactionId;
			uint32_t m_source;
			uint32_t m_destination;
		public:
			ActionNotification(ememory::SharedPtr<zeus::WebServer> _interface = nullptr,
			                   uint32_t _transactionId = 0,
			                   uint32_t _source = 0,
			                   uint32_t _destination = 0):
			  m_interface(_interface),
			  m_transactionId(_transactionId),
			  m_source(_source),
			  m_destination(_destination) {}
			
			void notify(const std::string& _value) {
				if (m_interface != nullptr) {
					m_interface->answerValue(m_transactionId, m_source, m_destination, _value);
				}
			}
	};
}