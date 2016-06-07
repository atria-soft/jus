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
	 U16 TypeMessgae (1:call, 2:Answer, 4:event)
	 // not needed ==> can be deduced with parameter number ... U16 Offset String call Name (start of the buffer) end with \0
	 ======================
	 == call
	 ======================
	 U16[param count] parameters offset (first offset is the "callName" and limit size of the number of parameter
	 CALL Name (funtion name)
	 [param 1]
	 [param 2]
	 [param 3]
	 [param 4]
	 ======================
	 == Answer
	 ======================
	 U16 ErrorOffset
	 [return value 1]
	 [error] (constituated with 2 strings (error type and comment)
	 ======================
	 == event
	 ======================
	 U16[param count] parameters offset (first offset is the "callName" and limit size of the number of parameter
	 event Name
	 [param 1]
	 [param 2]
	 [param 3]
	 [param 4]
	---------------------------
	parameter and return value is contituated like : 
	TYPE,DATAs....(in raw)
	Type is write in ascii in the list end with '\0':
	    - void
	    - bool
	    - float
	    - double
	    - int64
	    - int32
	    - int16
	    - int8
	    - uint64
	    - uint32
	    - uint16
	    - uint8
	    - string
	    - vector:bool
	    - vector:float
	    - vector:double
	    - vector:int64
	    - vector:int32
	    - vector:int16
	    - vector:int8
	    - vector:uint64
	    - vector:uint32
	    - vector:uint16
	    - vector:uint8
	    - vector:string
	    - obj:file
	*/
	class Buffer {
		private:
			std::vector<uint8_t> m_data;
		public:
			Buffer();
			clear();
			uint16_t getProtocalVersion();
			void setProtocolVersion(uint16_t _value);
			uint32_t getTransactionId();
			void setTransactionId(uint32_t _value);
			uint32_t getClientId();// this is the same as serviceId
			void setClientId(uint32_t _value);
			uint32_t getServiceId() {
				return getClientId();
			}
			void setServiceId(uint32_t _value) {
				setClientId(_value);
			}
			// note limited 15 bits
			uint16_t getPartId();
			void setPartId(uint16_t _value);
			bool getPartFinish();
			void setPartFinish(bool _value);
			enum class typeMessage {
				call = 0x0001,
				answer = 0x0002,
				event = 0x0004,
			}
			enum typeMessage getType();
			void setType(enum typeMessage _value);
		// ===============================================
		// == Section call
		// ===============================================
		public:
			std::string getCall();
			void setCall(std::string _value);
			uint16_t getNumberParameter();
			
			template<class JUS_TYPE_DATA>
			void addParameter(const JUS_TYPE_DATA& _value);
			
		// ===============================================
		// == Section Answer
		// ===============================================
		public:
			template<class JUS_TYPE_DATA>
			void addAnswer(const JUS_TYPE_DATA& _value);
	};
}

