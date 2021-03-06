/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.hpp>
#include <zeus/debug.hpp>
#include <zeus/AbstractFunction.hpp>
#include <zeus/ObjectRemote.hpp>
#include <zeus/Future.hpp>
#include <zeus/WebServer.hpp>
#include <zeus/WebObj.hpp>

namespace zeus {
	//class Client;
	class ObjectRemote;
	/**
	 * @brief Basic element of a remote element access.
	 */
	class ObjectRemoteBase : public zeus::WebObj {
		friend class ObjectRemote;
		private:
			etk::String m_type; //!< name of the remote object
			uint32_t m_remoteAddress; //!< remote adress of the object
			bool m_isLinked; //!< link status of the object
			ememory::SharedPtr<void> m_adressLocalObject; //!< Pointer on the local object if we are in the same process.
		public:
			/**
			 * @brief Contructor of the remote object
			 */
			ObjectRemoteBase():
			  zeus::WebObj(null, 0, 0) {
				ZEUS_INFO("[XX/YY] Create");
			}
			/**
			 * @brief Contuctor of the remote object accessor
			 * @param[in] _iface Web interface of the Object
			 * @param[in] _localId Current Web interface Id
			 * @param[in] _localObjectId Current object Id (local)
			 * @param[in] _address Remote object Address
			 * @param[in] _type Type of the remote object
			 */
			ObjectRemoteBase(const ememory::SharedPtr<zeus::WebServer>& _iface,
			                 uint16_t _localId,
			                 uint16_t _localObjectId,
			                 uint32_t _address,
			                 const etk::String& _type);
			/**
			 * @brief Generic destructor
			 */
			~ObjectRemoteBase();
			/**
			 * @brief permit to check if the remote hs been corectly created
			 * @return true The object exist
			 * @return false The object is NOT accessible
			 */
			bool exist() const;
			/**
			 * @brief Get the name of the object (use in client service check name)
			 * @TODO: Remove this API
			 */
			const etk::String& getName() const;
			/**
			 * @brief Get the remote interface ID
			 * @return The Id of the remote interface
			 */
			uint16_t getRemoteInterfaceId() {
				return m_remoteAddress>>16;
			}
			/**
			 * @brief Set the Remote object has been removed
			 */
			void setRemoteObjectRemoved() {
				ZEUS_WARNING("The object remote has been removed : " << m_remoteAddress);
				m_isLinked = false;
				m_remoteAddress = 0;
			}
			/**
			 * @breif Display property of the service
			 */
			void display();
	};
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class ObjectRemote {
		private:
			mutable ememory::SharedPtr<zeus::ObjectRemoteBase> m_interface; //!< Remote interface access (use sharedPtr to prevent multiple link with a simgle service is a single client)
		public:
			/**
			 * @brief Contructor of a remote Object
			 * @param[in] _interface Interface of the object
			 */
			ObjectRemote(ememory::SharedPtr<zeus::ObjectRemoteBase> _interface = null);
			/**
			 * @brief permit to check if the remote hs been corectly created
			 * @return true The object exist
			 * @return false The object is NOT accessible
			 */
			bool exist() const;
		public:
			/**
			 * @brief Call a remote function of the current object
			 * @param[in] _functionName Name of the function
			 * @param[in] _args All argument function needed by the remote to process the function
			 * @return A generic future with all datas
			 */
			template<class... _ARGS>
			zeus::FutureBase call(const etk::String& _functionName, _ARGS&&... _args) const {
				if (    m_interface == null
				     || m_interface->m_interfaceWeb == null) {
					ememory::SharedPtr<zeus::message::Answer> ret = zeus::message::Answer::create(null); // TODO : This is a real bad case ...
					if (ret != null) {
						ret->addError("NULLPTR", "call " + _functionName + " with no interface open");
					}
					return zeus::FutureBase(0, ret);
				}
				return m_interface->m_interfaceWeb->call(m_interface->getFullId(),
				                                         m_interface->m_remoteAddress,
				                                         _functionName,
				                                         _args...);
			}
			/**
			 * @brief Specific case for action call. This permit to check if user does not wait on event on something that is not an action.
			 * @param[in] _functionName Name of the function
			 * @param[in] _args All argument function needed by the remote to process the function
			 * @return A generic future with all datas
			 */
			template<class... _ARGS>
			zeus::FutureBase callAction(const etk::String& _functionName, _ARGS&&... _args) const {
				zeus::FutureBase tmp = call(_functionName, _args...);
				tmp.setAction();
				return tmp;
			}
	};
	
	
	
}

