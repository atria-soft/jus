/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.h>
#include <enet/WebSocket.h>
#include <zeus/ParamType.h>

namespace zeus {
	class WebServer;
	// define basic async call element ...
	using ActionAsyncClient = std::function<bool(WebServer* _interface, const uint32_t& _serviceId, uint64_t _transactionId, uint64_t _part)>;
}



namespace zeus {
	class BufferData;
	//U32 message lenght
	#pragma pack(push,1)
	/*
	struct headerBin {
		//uint16_t versionProtocol; // protocol Version (might be 1)
		uint32_t transactionID;
		uint32_t clientID; // same as sevice ID
		int16_t partID; // if < 0 the partId ifs the last (start at 0 if multiple or 0x8000 if single message)
		uint16_t typeMessage; //TypeMessgae (1:call, 2:Answer, 4:event)
		uint16_t numberOfParameter;
	};
	*/
	struct headerBin {
		//uint16_t versionProtocol; // protocol Version (might be 1)
		uint32_t transactionID;
		uint32_t clientID; // same as sevice ID
		uint8_t flags; // List of flags & type message:
		               //    - 0-2: Type of the message
		               //    - 3-5: Reserved
		               //    - 6: message data is zipped
		               //    - 7: message finished
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
	#define ZEUS_BUFFER_FLAG_FINISH (0x80)
	#define ZEUS_BUFFER_FLAG_TYPE_MESSAGE (0x07)
	/**
	 * @brief Protocol buffer to transmit datas
	 */
	class Buffer {
		friend std::ostream& operator<<(std::ostream&, zeus::Buffer*);
		protected:
			/**
			 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::Buffer::create
			 */
			Buffer();
		public:
			/**
			 * Virtualize the buffer class
			 */
			virtual ~Buffer() = default;
			/**
			 * @brief Create a shared pointer on the buffer
			 * @return Allocated Buffer.
			 */
			static ememory::SharedPtr<zeus::Buffer> create();
			/**
			 * @brief Create a shared pointer on the buffer
			 * @param[in] _buffer Buffer on the data
			 * @return Allocated Buffer.
			 */
			static ememory::SharedPtr<zeus::Buffer> create(const std::vector<uint8_t>& _buffer);
		protected:
			headerBin m_header; //!< header of the protocol
			std::vector<zeus::ActionAsyncClient> m_multipleSend; //!< Async element to send data on the webinterface when too big ...
		public:
			/**
			 * @brief Check if async element are present on this buffer
			 * @return return true if somme data must be send asyncronously
			 */
			bool haveAsync() const {
				return m_multipleSend.size() != 0;
			}
			/**
			 * @brief Get the list of async data to send
			 * @return Vector of the async data (the async are moved out ... call only one time)
			 */
			std::vector<zeus::ActionAsyncClient> moveAsync() {
				return std::move(m_multipleSend);
			}
			/**
			 * @brief When multiple frame buffer, they need to concatenate the data... call this function with the new data to append it ...
			 * @param[in] _obj Buffer to add
			 */
			void appendBuffer(const ememory::SharedPtr<zeus::Buffer>& _obj);
			virtual void appendBufferData(const ememory::SharedPtr<zeus::BufferData>& _obj);
		protected:
			/**
			 * @brief When receive new data form websocket, it might be added by this input (set all the frame ...)
			 * @param[in] _buffer Pointer on the data to add.
			 * @param[in] _lenght number of octet to add.
			 */
			virtual void composeWith(const uint8_t* _buffer, uint32_t _lenght);
		public:
			/**
			 * @brief Chear the buffer
			 */
			void clear();
			/**
			 * @brief Get the transaction identifier of the packet
			 * @return value of the transaction
			 */
			uint32_t getTransactionId() const;
			/**
			 * @brief Set the transaction identifier of the packet
			 * @param[in] _value New transaction id
			 */
			void setTransactionId(uint32_t _value);
			/**
			 * @brief Get the Client identifier of the packet
			 * @return Value of the Client identifier
			 */
			uint32_t getClientId() const;// this is the same as serviceId
			/**
			 * @brief Set the Client identifier of the packet
			 * @param[in] _value New value of the Client identifier
			 */
			void setClientId(uint32_t _value);
			/**
			 * @brief Get the Service identifier of the packet (same as client)
			 * @return Value of the Service identifier
			 */
			uint32_t getServiceId() const {
				return getClientId();
			}
			/**
			 * @brief Set the Service identifier of the packet (same as client)
			 * @param[in] _value New value of the Service identifier
			 */
			void setServiceId(uint32_t _value) {
				setClientId(_value);
			}
			/**
			 * @brief Check if it is the last packet of the buffer
			 * @return If "true" The Buffer wait no more datas
			 */
			bool getPartFinish() const;
			/**
			 * @brief set the finish state of the buffer
			 * @param[in] _value set the sate of finish of the buffer
			 */
			void setPartFinish(bool _value);
			/**
			 * @brief Type of the massage send or receive
			 */
			enum class typeMessage {
				unknow = 0x0000, //!< Init value
				call = 0x0001, //!< Remote call on a service ID
				answer = 0x0002, //!< Answer from a previous call
				data = 0x0003, //!< data message happend when partId > 0 it compleate the data of a parameter or an answer or an event
				event = 0x0004, //!< event message
			};
			/**
			 * @brief Get the type of the buffer
			 * @return the current type of the buffer
			 */
			virtual enum typeMessage getType() const;
		public:
			/**
			 * @brief Write the buffer on a specific interface
			 * @param[in] _interface socket to write data
			 * @return true of no error appear
			 */
			virtual bool writeOn(enet::WebSocket& _interface);
			virtual void generateDisplay(std::ostream& _os) const ;
	};
	/**
	 * @brief generate a display of the typemessage
	 * @param[in] _os stream to add data
	 * @value[in] _value Value to add
	 * @return a reference of the stream
	 */
	std::ostream& operator <<(std::ostream& _os, enum zeus::Buffer::typeMessage _value);
	/**
	 * @brief generate a display of the typemessage
	 * @param[in] _os stream to add data
	 * @value[in] _obj Buffer to display
	 * @return a reference of the stream
	 */
	std::ostream& operator <<(std::ostream& _os, zeus::Buffer* _obj);
	
	// internal:
	/**
	 * @brief Add a parameter generic type in the buffer
	 * @param[in] _data Buffer to add type
	 * @param[in] _type generic type to add
	 */
	void addType(std::vector<uint8_t>& _data, zeus::ParamType _type);
	/**
	 * @brief Add a parameter object type in the buffer
	 * @param[in] _data Buffer to add type
	 * @param[in] _type string of the type to add
	 */
	void addTypeObject(std::vector<uint8_t>& _data, const std::string _type);
	/**
	 * @brief Add a parameter RAW type in the buffer
	 * @param[in] _data Buffer to add type
	 */
	void addTypeRaw(std::vector<uint8_t>& _data);
}

