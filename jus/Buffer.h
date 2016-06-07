/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.h>

namespace jus {
	/*
	 U32 message lenght
	 U16 protocol Version (might be 1)
	 U32 transactionID;
	 U32 clientID; ==> sevice ID
	 U1  finish part
	 U15 partID;
	 // not needed ==> can be deduced with parameter number ... U16 Offset String call Name (start of the buffer) end with \0
	 U8 param count
	 U16[param count] parameters offset
	 ... DATAS ...
	*/
	class Buffer {
		private:
			std::vector<uint8_t> m_data;
			uint64_t m_transactionId;
			uint64_t m_clientId; // clientId/service
			uint32_t m_partId;
			bool m_partFinish;
			uint32_t m_parameterCount;
			
		public:
			Buffer();