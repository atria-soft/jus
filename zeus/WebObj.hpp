/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/types.hpp>
#include <ememory/memory.hpp>
#include <zeus/message/Message.hpp>

namespace zeus {
	class WebServer;
	/**
	 * @brief generic handle for remote object
	 */
	class WebObj : public ememory::EnableSharedFromThis<zeus::WebObj> {
		protected:
			ememory::SharedPtr<zeus::WebServer> m_interfaceWeb; //!< web interface
			uint16_t m_id; //!< Id Of the local interface
			uint16_t m_objectId; //!< Local Object Id
			std::vector<uint32_t> m_listRemoteConnected; //!< Number of service connected
		public:
			/**
			 * @brief Contructor
			 * @param[in] _iface Network interface to interact
			 * @param[in] _id Local user ID
			 * @param[in] _objectId This object unique ID
			 */
			WebObj(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _id, uint16_t _objectId);
			/**
			 * @brief Descructor
			 */
			virtual ~WebObj();
			/**
			 * @brief Get the local client ID
			 * @return Local client ID
			 */
			uint16_t getInterfaceId();
			/**
			 * @brief Get the local Object ID
			 * @return Unique local object ID
			 */
			uint16_t getObjectId();
			/**
			 * @brief Get the full ID of the object (IFaceId<<16+ObjectId)
			 * @return the Full object ID
			 */
			uint32_t getFullId();
			/**
			 * @brief Something send a message to the Object
			 * @param[in] _value Value to process
			 */
			virtual void receive(ememory::SharedPtr<zeus::Message> _value);
			/**
			 * @brief Display object properties
			 */
			virtual void display();
			/**
			 * @brief Add a remote Object link on this one
			 * @param[in] _id Id of the remote object
			 */
			void addRemote(uint32_t _id);
			/**
			 * @brief Remove a remote object link on this one
			 * @param[in] _id Id of the remote object
			 * @return true The owner has been fined and remove
			 * @return false other bad case ...
			 */
			bool removeOwnership(uint32_t _id);
			/**
			 * @brief A client interface has been removed ==> need to call this function for every object.
			 * @param[in] _id Id of the interface that is down
			 */
			void rmRemoteInterface(uint16_t _id);
			/**
			 * @brief Check if someone request a link on this object
			 * @return true Someone is connected
			 * @return false standalone object
			 */
			bool haveRemoteConnected() const;
			/**
			 * @brief Transfer the onership from an object address to an other object address
			 * @param[in] _sourceAddress Object address to change
			 * @param[in] _destinataireAddress New Object address to set
			 * @return true Onership has change
			 * @return false An error occured
			 * @note Why we need the onership transfer? when you request a new service, this is the client GW that id doing it, then the remote addreess is not the good one, then we update the address to imply the good one
			 */
			bool transferOwnership(uint32_t _sourceAddress, uint32_t _destinataireAddress);
		
	};
}

