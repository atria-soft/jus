/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/TcpString.h>
#include <eproperty/Value.h>
#include <zeus/debug.h>
#include <zeus/AbstractFunction.h>
#include <zeus/mineType.h>
#include <etk/os/FSNode.h>


namespace zeus {
	template <class ZEUS_CLASS_TYPE, class ZEUS_RETURN, class... ZEUS_TYPES>
	void executeClassCall(const ememory::SharedPtr<zeus::TcpString>& _interfaceClient,
	                      uint64_t _transactionId,
	                      uint64_t _clientId,
	                      ZEUS_CLASS_TYPE* _pointer,
	                      ZEUS_RETURN (ZEUS_CLASS_TYPE::*_func)(ZEUS_TYPES...),
	                      zeus::Buffer& _obj) {
		std::vector<ActionAsyncClient> asyncAction;
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			ZEUS_RETURN ret = (*_pointer.*_func)(_obj.getParameter<ZEUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			ZEUS_RETURN ret = (*_pointer.*_func)(_obj.getParameter<ZEUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
			ZEUS_RETURN ret;
			return;
		#endif
		if (asyncAction.size() != 0) {
			ZEUS_ERROR("Missing send async messages");
		}
		_interfaceClient->addAsync([=](TcpString* _interface) {
			    _interface->answerValue(_transactionId, ret, _clientId);
			    return true;
			});
	}
	class SendFile {
		private:
			zeus::FileServer m_data;
			uint64_t m_transactionId;
			uint64_t m_clientId;
			uint32_t m_partId;
			etk::FSNode m_node;
			uint64_t m_size;
		public:
			SendFile(zeus::FileServer _data,
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
				zeus::Buffer answer;
				answer.setTransactionId(m_transactionId);
				answer.setClientId(m_clientId);
				answer.setPartId(m_partId);
				answer.setPartFinish(false);
				if (m_partId == 0) {
					answer.setType(zeus::Buffer::typeMessage::answer);
					m_node.fileOpenRead();
					std::string extention = std::string(m_data.getFileName().begin()+m_data.getFileName().size() -3, m_data.getFileName().end());
					ZEUS_WARNING("send file: '" << m_data.getFileName() << "' with extention: '" << extention << "'");
					m_size = m_node.fileSize();
					zeus::File tmpFile(zeus::getMineType(extention), std::vector<uint8_t>(), m_size);
					answer.addAnswer(tmpFile);
					_interface->writeBinary(answer);
					m_partId++;
					return false;
				}
				answer.setType(zeus::Buffer::typeMessage::data);
				int32_t tmpSize = 1024;
				if (m_size < 1024) {
					tmpSize = m_size;
				}
				uint8_t tmpData[1024];
				m_node.fileRead(tmpData, 1, tmpSize);
				answer.addData(tmpData, tmpSize);
				m_size -= tmpSize;
				if (m_size <= 0) {
					answer.setPartFinish(true);
					m_node.fileClose();
				}
				_interface->writeBinary(answer);;
				m_partId++;
				if (m_size <= 0) {
					return true;
				}
				return false;
			}
	};
	template <class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	void executeClassCall(const ememory::SharedPtr<zeus::TcpString>& _interfaceClient,
	                      uint64_t _transactionId,
	                      uint64_t _clientId,
	                      ZEUS_CLASS_TYPE* _pointer,
	                      zeus::FileServer (ZEUS_CLASS_TYPE::*_func)(ZEUS_TYPES...),
	                      zeus::Buffer& _obj) {
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			zeus::FileServer tmpElem = (*_pointer.*_func)(_obj.getParameter<ZEUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			zeus::FileServer tmpElem = (*_pointer.*_func)(_obj.getParameter<ZEUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
			zeus::FileServer tmpElem;
			return;
		#endif
		_interfaceClient->addAsync(SendFile(tmpElem, _transactionId, _clientId));
	}
	
	template <class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	void executeClassCall(const ememory::SharedPtr<zeus::TcpString>& _interfaceClient,
	                      uint64_t _transactionId,
	                      uint64_t _clientId,
	                      ZEUS_CLASS_TYPE* _pointer,
	                      void (ZEUS_CLASS_TYPE::*_func)(ZEUS_TYPES...),
	                      zeus::Buffer& _obj) {
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			(*_pointer.*_func)(_obj.getParameter<ZEUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			(*_pointer.*_func)(_obj.getParameter<ZEUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
			return;
		#endif
		_interfaceClient->addAsync([=](TcpString* _interface) {
			    _interface->answerVoid(_transactionId, _clientId);
			    return true;
			});
	}
	
	template <class ZEUS_RETURN, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	class AbstractFunctionTypeClass: public zeus::AbstractFunction {
		protected:
			static const ParamType m_returnType;
			static const ParamType m_paramType[sizeof...(ZEUS_TYPES)];
		public:
			using functionType = ZEUS_RETURN (ZEUS_CLASS_TYPE::*)(ZEUS_TYPES...);
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
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
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
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
					out.push_back(m_paramType[iii].getName());
				}
				return out;
			}
			void execute(const ememory::SharedPtr<zeus::TcpString>& _interfaceClient,
			             uint64_t _transactionId,
			             uint64_t _clientId,
			             zeus::Buffer& _obj,
			             void* _class) override {
				ZEUS_CLASS_TYPE* tmpClass = nullptr;
				if (_class != nullptr) {
					tmpClass = (ZEUS_CLASS_TYPE*)_class;
				}
				
				// check parameter number
				if (_obj.getNumberParameter() != sizeof...(ZEUS_TYPES)) {
					ZEUS_ERROR("Wrong number of Parameters ...");
					std::string help = "request ";
					help += etk::to_string(_obj.getNumberParameter());
					help += " parameters and need ";
					help += etk::to_string(sizeof...(ZEUS_TYPES));
					help += " parameters. prototype function:";
					help += getPrototype();
					_interfaceClient->answerError(_transactionId,
					                              "WRONG-PARAMETER-NUMBER",
					                              help,
					                              _clientId);
					return;
				}
				// check parameter compatibility
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
					if (checkCompatibility(m_paramType[iii], _obj.getParameterType(iii)) == false) {
						_interfaceClient->answerError(_transactionId,
						                              "WRONG-PARAMETER-TYPE",
						                              std::string("Parameter id ") + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'",
						                              _clientId);
						return;
					}
				}
				// execute cmd:
				zeus::executeClassCall(_interfaceClient, _transactionId, _clientId, tmpClass, m_function, _obj);
			}
	};
	
	template <class ZEUS_RETURN, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	const ParamType AbstractFunctionTypeClass<ZEUS_RETURN, ZEUS_CLASS_TYPE, ZEUS_TYPES...>::m_returnType = createType<ZEUS_RETURN>();
	
	template <class ZEUS_RETURN, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	const ParamType AbstractFunctionTypeClass<ZEUS_RETURN, ZEUS_CLASS_TYPE, ZEUS_TYPES...>::m_paramType[sizeof...(ZEUS_TYPES)] = {createType<ZEUS_TYPES>()...};
	
	
	template <typename ZEUS_RETURN, class ZEUS_CLASS_TYPE, typename... ZEUS_TYPES>
	AbstractFunction* createAbstractFunctionClass(const std::string& _name, const std::string& _desc, ZEUS_RETURN (ZEUS_CLASS_TYPE::*_fffp)(ZEUS_TYPES...)) {
		return new AbstractFunctionTypeClass<ZEUS_RETURN, ZEUS_CLASS_TYPE, ZEUS_TYPES...>(_name, _desc, _fffp);
	}
}

