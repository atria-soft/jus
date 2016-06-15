/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <jus/TcpString.h>
#include <eproperty/Value.h>
#include <ejson/ejson.h>
#include <jus/debug.h>
#include <jus/AbstractFunction.h>
#include <jus/mineType.h>
#include <etk/os/FSNode.h>
#include <ejson/base64.h>


namespace jus {
	template <class JUS_CLASS_TYPE, class JUS_RETURN, class... JUS_TYPES>
	void executeClassCall(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
	                      uint64_t _transactionId,
	                      uint64_t _clientId,
	                      JUS_CLASS_TYPE* _pointer,
	                      JUS_RETURN (JUS_CLASS_TYPE::*_func)(JUS_TYPES...),
	                      jus::Buffer& _obj) {
		std::vector<ActionAsyncClient> asyncAction;
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			JUS_RETURN ret = (*_pointer.*_func)(_obj.getParameter<JUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(JUS_TYPES))-1;
			JUS_RETURN ret = (*_pointer.*_func)(_obj.getParameter<JUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
			JUS_RETURN ret;
			return;
		#endif
		if (asyncAction.size() != 0) {
			JUS_ERROR("Missing send async messages");
		}
		_interfaceClient->addAsync([=](TcpString* _interface) {
			    _interface->answerValue(_transactionId, ret, _clientId);
			    return true;
			});
	}
	class SendFile {
		private:
			jus::FileServer m_data;
			uint64_t m_transactionId;
			uint64_t m_clientId;
			uint32_t m_partId;
			etk::FSNode m_node;
			uint64_t m_size;
		public:
			SendFile(jus::FileServer _data,
			         uint64_t _transactionId,
			         uint64_t _clientId) :
			  m_data(_data),
			  m_transactionId(_transactionId),
			  m_clientId(_clientId),
			  m_partId(0),
			  m_node(_data.getFileName()),
			  m_size(0) {
				
			}
			~SendFile() {
				//m_node.fileClose();
			}
			bool operator() (TcpString* _interface) {
				jus::Buffer answer;
				answer.setTransactionId(m_transactionId);
				answer.setClientId(m_clientId);
				answer.setPartId(m_partId);
				answer.setPartFinish(false);
				if (m_partId == 0) {
					m_node.fileOpenRead();
					std::string extention = std::string(m_data.getFileName().begin()+m_data.getFileName().size() -3, m_data.getFileName().end());
					JUS_WARNING("send file: '" << m_data.getFileName() << "' with extention: '" << extention << "'");
					m_size = m_node.fileSize();
					jus::File tmpFile(jus::getMineType(extention), std::vector<uint8_t>(), m_size);
					answer.addAnswer(tmpFile);
					// TODO : Manage JSON ...
					_interface->writeBinary(answer);
					m_partId++;
					return false;
				}
				int32_t tmpSize = 1024;
				if (m_size < 1024) {
					tmpSize = m_size;
				}
				uint8_t tmpData[1024];
				m_node.fileRead(tmpData, 1, tmpSize);
				answer.addData(tmpData, tmpSize);
				//answer.add("data", ejson::String(ejson::base64::encode(tmpData, tmpSize)));
				m_size -= tmpSize;
				if (m_size <= 0) {
					answer.setPartFinish(true);
					m_node.fileClose();
				}
				//JUS_INFO("Answer: " << answer.generateHumanString());
				_interface->writeBinary(answer);;
				m_partId++;
				if (m_size <= 0) {
					return true;
				}
				return false;
			}
	};
	template <class JUS_CLASS_TYPE, class... JUS_TYPES>
	void executeClassCall(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
	                      uint64_t _transactionId,
	                      uint64_t _clientId,
	                      JUS_CLASS_TYPE* _pointer,
	                      jus::FileServer (JUS_CLASS_TYPE::*_func)(JUS_TYPES...),
	                      jus::Buffer& _obj) {
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			jus::FileServer tmpElem = (*_pointer.*_func)(_obj.getParameter<JUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(JUS_TYPES))-1;
			jus::FileServer tmpElem = (*_pointer.*_func)(_obj.getParameter<JUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
			jus::FileServer tmpElem;
			return;
		#endif
		_interfaceClient->addAsync(SendFile(tmpElem, _transactionId, _clientId));
	}
	
	template <class JUS_CLASS_TYPE, class... JUS_TYPES>
	void executeClassCall(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
	                      uint64_t _transactionId,
	                      uint64_t _clientId,
	                      JUS_CLASS_TYPE* _pointer,
	                      void (JUS_CLASS_TYPE::*_func)(JUS_TYPES...),
	                      jus::Buffer& _obj) {
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			(*_pointer.*_func)(_obj.getParameter<JUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(JUS_TYPES))-1;
			(*_pointer.*_func)(_obj.getParameter<JUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
			return;
		#endif
		_interfaceClient->addAsync([=](TcpString* _interface) {
			    _interface->answerVoid(_transactionId, _clientId);
			    return true;
			});
	}
	
	template <class JUS_RETURN, class JUS_CLASS_TYPE, class... JUS_TYPES>
	class AbstractFunctionTypeClass: public jus::AbstractFunction {
		protected:
			static const ParamType m_returnType;
			static const ParamType m_paramType[sizeof...(JUS_TYPES)];
		public:
			using functionType = JUS_RETURN (JUS_CLASS_TYPE::*)(JUS_TYPES...);
			functionType m_function;
			AbstractFunctionTypeClass(const std::string& _name, const std::string& _desc, functionType _fptr):
			  AbstractFunction(_name, _desc),
			  m_function(_fptr) {
			}
			std::string getPrototype() const override {
				std::string ret;
				ret += m_returnType.getName();
				ret += " ";
				ret += m_name;
				ret += "(";
				for (size_t iii=0; iii<sizeof...(JUS_TYPES); ++iii) {
					if (iii != 0) {
						ret += ", ";
					}
					ret += m_paramType[iii].getName();
				}
				ret += ");";
				return ret;
			}
			std::string getPrototypeReturn() const override {
				return m_returnType.getName();
			}
			std::vector<std::string> getPrototypeParam() const override {
				std::vector<std::string> out;
				for (size_t iii=0; iii<sizeof...(JUS_TYPES); ++iii) {
					out.push_back(m_paramType[iii].getName());
				}
				return out;
			}
			void execute(const ememory::SharedPtr<jus::TcpString>& _interfaceClient,
			             uint64_t _transactionId,
			             uint64_t _clientId,
			             jus::Buffer& _obj,
			             void* _class) override {
				JUS_CLASS_TYPE* tmpClass = nullptr;
				if (_class != nullptr) {
					tmpClass = (JUS_CLASS_TYPE*)_class;
				}
				
				// check parameter number
				if (_obj.getNumberParameter() != sizeof...(JUS_TYPES)) {
					JUS_ERROR("Wrong number of Parameters ...");
					std::string help = "request ";
					help += etk::to_string(_obj.getNumberParameter());
					help += " parameters and need ";
					help += etk::to_string(sizeof...(JUS_TYPES));
					help += " parameters. prototype function:";
					help += getPrototype();
					_interfaceClient->answerError(_transactionId,
					                              "WRONG-PARAMETER-NUMBER",
					                              help,
					                              _clientId);
					return;
				}
				// check parameter compatibility
				for (size_t iii=0; iii<sizeof...(JUS_TYPES); ++iii) {
					if (checkCompatibility(m_paramType[iii], _obj.getParameterType(iii)) == false) {
						_interfaceClient->answerError(_transactionId,
						                              "WRONG-PARAMETER-TYPE",
						                              std::string("Parameter id ") + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'",
						                              _clientId);
						return;
					}
				}
				// execute cmd:
				jus::executeClassCall(_interfaceClient, _transactionId, _clientId, tmpClass, m_function, _obj);
			}
	};
	
	template <class JUS_RETURN, class JUS_CLASS_TYPE, class... JUS_TYPES>
	const ParamType AbstractFunctionTypeClass<JUS_RETURN, JUS_CLASS_TYPE, JUS_TYPES...>::m_returnType = createType<JUS_RETURN>();
	
	template <class JUS_RETURN, class JUS_CLASS_TYPE, class... JUS_TYPES>
	const ParamType AbstractFunctionTypeClass<JUS_RETURN, JUS_CLASS_TYPE, JUS_TYPES...>::m_paramType[sizeof...(JUS_TYPES)] = {createType<JUS_TYPES>()...};
	
	
	template <typename JUS_RETURN, class JUS_CLASS_TYPE, typename... JUS_TYPES>
	AbstractFunction* createAbstractFunctionClass(const std::string& _name, const std::string& _desc, JUS_RETURN (JUS_CLASS_TYPE::*_fffp)(JUS_TYPES...)) {
		return new AbstractFunctionTypeClass<JUS_RETURN, JUS_CLASS_TYPE, JUS_TYPES...>(_name, _desc, _fffp);
	}
}

