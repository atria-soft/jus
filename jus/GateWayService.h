/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <ememory/memory.h>

namespace jus {
	class GateWayService {
		public:
			ememory::SharedPtr<jus::TcpString> m_interfaceService;
			std::string m_name;
		public:
			GateWayService(const ememory::SharedPtr<jus::TcpString>& _interface);
			virtual ~GateWayService();
	};
}

