/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once
#include <etk/types.hpp>
#include <enet/WebSocket.hpp>
#include <zeus/ParamType.hpp>

namespace zeus {
	class WebServer;
	// define basic async call element ...
	using ActionAsyncClient = std::function<bool(WebServer* _interface, const uint32_t& _clientId, const uint32_t& _serviceId, uint64_t _transactionId, uint64_t _part)>;
}



namespace zeus {
	class BufferData;
	//U32 message lenght
	#pragma pack(push,1)
	struct headerBin {
		//uint16_t versionProtocol; // protocol Version (might be 1)
		uint32_t transactionId; //!< Transaction ID : Note the Upper byte is reserved for next protocol version (like change in protocol v2 with changing header)
		uint16_t sourceId; //!< Source of the message
		uint16_t sourceObjectId; //!< Source Object ID
		uint16_t destinationId; //!< Destination  of the message
		uint16_t destinationObjectId; //!< Destination Object ID
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
	parameter and return value is contituated like:
	TYPE,DATAs....(in raw)
	Type is write in ascii in the list end with '\0':
	    - void
	    - bool
	    - float32
	    - float64
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
	    - vector:float32
	    - vector:float64
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
	    - duration
	    - time
	*/
	#define ZEUS_BUFFER_FLAG_FINISH (0x80)
	#define ZEUS_BUFFER_FLAG_TYPE_MESSAGE (0x07)
	/**
	 * @brief Protocol buffer to transmit datas
	 */
	class Buffer {
		friend std::ostream& operator<<(std::ostream&, zeus::Buffer*);
		protected:
			ememory::SharedPtr<zeus::WebServer> m_iface;
		protected:
			/**
			 * @brief basic constructor (hidden to force the use of ememory::SharedPtr) @ref zeus::Buffer::create
			 */
			Buffer(ememory::SharedPtr<zeus::WebServer> _iface);
		public:
			/**
			 * Virtualize the buffer class
			 */
			virtual ~Buffer() = default;
			/**
			 * @brief Create a shared pointer on the buffer
			 * @return Allocated Buffer.
			 */
			static ememory::SharedPtr<zeus::Buffer> create(ememory::SharedPtr<zeus::WebServer> _iface);
			/**
			 * @brief Create a shared pointer on the buffer
			 * @param[in] _buffer Buffer on the data
			 * @return Allocated Buffer.
			 */
			static ememory::SharedPtr<zeus::Buffer> create(ememory::SharedPtr<zeus::WebServer> _iface, const std::vector<uint8_t>& _buffer);
		protected:
			uint32_t m_interfaceID; //!< For debug ==> unterface ID ...
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
			void appendBuffer(ememory::SharedPtr<zeus::Buffer> _obj);
			virtual void appendBufferData(ememory::SharedPtr<zeus::BufferData> _obj);
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
			 * @brief Get the interface identifier of the packet
			 * @return value of the interface
			 */
			uint32_t getInterfaceId() const;
			/**
			 * @brief Set the interface identifier of the packet
			 * @param[in] _value New interface id
			 */
			void setInterfaceId(uint32_t _value);
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
			 * @brief Get the Source handle (getSourceId()<<16 + getSourceObjectId())
			 * @return Value of the Source handle
			 */
			uint32_t getSource() const;
			/**
			 * @brief Set the Source handle (getSourceId()<<16 + getSourceObjectId())
			 * @param[in] _value New value of the Source handle
			 */
			void setSource(uint32_t _value);
			/**
			 * @brief Get the Source identifier of the packet
			 * @return Value of the Source identifier
			 */
			uint16_t getSourceId() const;
			/**
			 * @brief Set the Source identifier of the packet
			 * @param[in] _value New value of the Source identifier
			 */
			void setSourceId(uint16_t _value);
			/**
			 * @brief Get the Source Object identifier of the packet
			 * @return Value of the Source Object identifier
			 */
			uint16_t getSourceObjectId() const;
			/**
			 * @brief Set the Source Object identifier of the packet
			 * @param[in] _value New value of the Source Object identifier
			 */
			void setSourceObjectId(uint16_t _value);
			/**
			 * @brief Get the Destination handle (getDestinationId()<<16 + getDestinationObjectId())
			 * @return Value of the Destination identifier
			 */
			uint32_t getDestination() const;
			/**
			 * @brief Set the Destination handle (getDestinationId()<<16 + getDestinationObjectId())
			 * @param[in] _value New value of the Destination handle
			 */
			void setDestination(uint32_t _value);
			/**
			 * @brief Get the Destination identifier of the packet
			 * @return Value of the Destination identifier
			 */
			uint16_t getDestinationId() const;
			/**
			 * @brief Set the Destination identifier of the packet
			 * @param[in] _value New value of the Destination identifier
			 */
			void setDestinationId(uint16_t _value);
			/**
			 * @brief Get the Destination Object identifier of the packet
			 * @return Value of the Destination Object identifier
			 */
			uint16_t getDestinationObjectId() const;
			/**
			 * @brief Set the Destination Object identifier of the packet
			 * @param[in] _value New value of the Destination Object identifier
			 */
			void setDestinationObjectId(uint16_t _value);
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
				ctrl = 0x0001, //!< Control message
				call = 0x0002, //!< Remote call on a service ID
				answer = 0x0003, //!< Answer from a previous call
				data = 0x0004, //!< data message happend when partId > 0 it compleate the data of a parameter or an answer or an event
				event = 0x0005, //!< event message
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
	std::ostream& operator <<(std::ostream& _os, ememory::SharedPtr<zeus::Buffer> _obj);
	
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
}

