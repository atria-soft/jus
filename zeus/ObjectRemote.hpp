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
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class ObjectRemoteBase : public zeus::WebObj {
		friend class ObjectRemote;
		private:
			std::string m_type;
			uint32_t m_remoteAddress;
			bool m_isLinked;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			ObjectRemoteBase():
			  zeus::WebObj(nullptr, 0, 0) {
				ZEUS_INFO("[XX/YY] Create");
			}
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			ObjectRemoteBase(const ememory::SharedPtr<zeus::WebServer>& _iface, uint16_t _localId, uint16_t _localObjectId, uint32_t _address, const std::string& _type);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			~ObjectRemoteBase();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			bool exist() const;
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			const std::string& getName() const;
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
			void display();
	};
	/**
	 * @brief 
	 * @param[in] 
	 * @return 
	 */
	class ObjectRemote {
		private:
			mutable ememory::SharedPtr<zeus::ObjectRemoteBase> m_interface;
		public:
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			ObjectRemote(ememory::SharedPtr<zeus::ObjectRemoteBase> _interface = nullptr);
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			~ObjectRemote();
			/**
			 * @brief 
			 * @param[in] 
			 * @return 
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
			zeus::FutureBase call(const std::string& _functionName, _ARGS&&... _args) const {
				if (    m_interface == nullptr
				     || m_interface->m_interfaceWeb == nullptr) {
					ememory::SharedPtr<zeus::message::Answer> ret = zeus::message::Answer::create(nullptr); // TODO : This is a real bad case ...
					if (ret != nullptr) {
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
			 * @brief 
			 * @param[in] 
			 * @return 
			 */
			template<class... _ARGS>
			zeus::FutureBase callAction(const std::string& _functionName, _ARGS&&... _args) const {
				zeus::FutureBase tmp = call(_functionName, _args...);
				tmp.setAction();
				return tmp;
			}
	};
	
	
	
}

