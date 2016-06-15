/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.h>
#include <ejson/ejson.h>

namespace jus {
	//U32 message lenght
	#pragma pack(push,1)
	struct headerBin {
		uint32_t lenght;
		uint16_t versionProtocol; // protocol Version (might be 1)
		uint32_t transactionID;
		uint32_t clientID; // same as sevice ID
		int16_t partID; // if < 0 the partId ifs the last (start at 0 if multiple or 0x8000 if single message)
		uint16_t typeMessage; //TypeMessgae (1:call, 2:Answer, 4:event)
		uint16_t numberOfParameter; //TypeMessgae (1:call, 2:Answer, 4:event)
	};
	#pragma pack(pop)
	/*
	 // not needed ==> can be deduced with parameter number ... U16 Offset String call Name (start of the buffer) end with \0
	 ======================
	 == call
	 ======================
	 U16 param count
	 U16[param count] parameters offset (first offset is the "callName" and limit size of the number of parameter
	 CALL Name (funtion name)
	 [param 1]
	 [param 2]
	 [param 3]
	 [param 4]
	 ======================
	 == Answer
	 ======================
	 U16 param count = 3
	 U16[param count]
	 [param 1] (return value)
	 [param 2] (error value)
	 [param 3] (error help)
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
			headerBin m_header;
			std::vector<uint16_t> m_paramOffset;
			std::vector<uint8_t> m_data;
		public:
			const uint8_t* getHeader() const {
				return reinterpret_cast<const uint8_t*>(&m_header);
			}
			uint32_t getHeaderSize() const {
				return sizeof(headerBin);
			}
			const uint8_t* getParam() const {
				return reinterpret_cast<const uint8_t*>(&m_paramOffset[0]);
			}
			uint32_t getParamSize() const {
				return m_paramOffset.size() * 2;
			}
			const uint8_t* getData() const {
				return &m_data[0];
			}
			uint32_t getDataSize() const {
				return m_data.size();
			}
		private:
			void internalComposeWith(const uint8_t* _buffer, uint32_t _lenght);
		public:
			Buffer();
			void composeWith(const std::vector<uint8_t>& _buffer);
			void composeWith(const std::string& _buffer);
			std::string generateHumanString();
			void clear();
			uint16_t getProtocalVersion() const;
			void setProtocolVersion(uint16_t _value);
			uint32_t getTransactionId() const;
			void setTransactionId(uint32_t _value);
			uint32_t getClientId() const;// this is the same as serviceId
			void setClientId(uint32_t _value);
			uint32_t getServiceId() const {
				return getClientId();
			}
			void setServiceId(uint32_t _value) {
				setClientId(_value);
			}
			// note limited 15 bits
			uint16_t getPartId() const;
			void setPartId(uint16_t _value);
			bool getPartFinish() const;
			void setPartFinish(bool _value);
			enum class typeMessage {
				call = 0x0001,
				answer = 0x0002,
				event = 0x0004,
				data = 0x0008,
			};
			enum typeMessage getType() const;
			void setType(enum typeMessage _value);
			
		// ===============================================
		// == Section call
		// ===============================================
		private:
			template<class JUS_TYPE_DATA>
			JUS_TYPE_DATA internalGetParameter(int32_t _id) const;
			std::string internalGetParameterType(int32_t _id) const;
			const uint8_t* internalGetParameterPointer(int32_t _id) const;
			uint32_t internalGetParameterSize(int32_t _id) const;
		public:
			std::string getCall() const;
			void setCall(std::string _value);
			uint16_t getNumberParameter() const;
			std::string getParameterType(int32_t _id) const;
			const uint8_t* getParameterPointer(int32_t _id) const;
			uint32_t getParameterSize(int32_t _id) const;
			
		private:
			// add parameter for transcription of json element
			void addParameter(ejson::Value _value);
		public:
			template<class JUS_TYPE_DATA>
			void addParameter(const JUS_TYPE_DATA& _value);
			void addParameterEmptyVector();
			void addParameter();
			
			template<class JUS_TYPE_DATA>
			JUS_TYPE_DATA getParameter(int32_t _id) const {
				return internalGetParameter<JUS_TYPE_DATA>(_id+1);
			}
			
			
		// ===============================================
		// == Section Answer
		// ===============================================
		public:
			template<class JUS_TYPE_DATA>
			void addAnswer(const JUS_TYPE_DATA& _value) {
				addParameter(_value);
			}
			// TODO : Do it better check error ... ==> can be good ...
			template<class JUS_TYPE_DATA>
			JUS_TYPE_DATA getAnswer() const {
				return internalGetParameter<JUS_TYPE_DATA>(0);
			}
			void addError(const std::string& _value, const std::string& _comment);
			bool hasError();
			std::string getError();
			std::string getErrorHelp();
		
		public:
			//multiple section of data (part ...)
			void addData(void* _data, uint32_t _size);
		
			void prepare();
			ejson::Object toJson() const;
			void fromJson(const ejson::Object& _data);
	};
	std::ostream& operator <<(std::ostream& _os, enum jus::Buffer::typeMessage _value);
	
	template<class JUS_TYPE>
	ejson::Value convertBinaryToJson(const uint8_t* _data, uint32_t _size);
}

