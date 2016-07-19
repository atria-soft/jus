/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <zeus/WebServer.h>
#include <zeus/debug.h>
#include <zeus/AbstractFunction.h>
#include <zeus/mineType.h>


namespace zeus {
	/**
	 * @brief Execute a call on the function with a return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Buffer input call (that have parameter already check)
	 * @param[in] _pointer Pointer on the class to call
	 * @param[in] _func pointer on the function to call
	 */
	template <class ZEUS_CLASS_TYPE, class ZEUS_RETURN, class... ZEUS_TYPES>
	void executeClassCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                      ememory::SharedPtr<zeus::BufferParameter> _obj,
	                      ZEUS_CLASS_TYPE* _pointer,
	                      ZEUS_RETURN (ZEUS_CLASS_TYPE::*_func)(ZEUS_TYPES...)) {
		if (_obj == nullptr) {
			return;
		}
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			ZEUS_RETURN ret = (*_pointer.*_func)(_obj->getParameter<ZEUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			ZEUS_RETURN ret = (*_pointer.*_func)(_obj->getParameter<ZEUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
			ZEUS_RETURN ret;
			return;
		#endif
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerValue(_obj->getTransactionId(), ret, _obj->getClientId());
			    return true;
			});
	}
	/**
	 * @brief Execute a call on the function with NO return value
	 * @param[in] _interfaceClient Web interface to send data
	 * @param[in] _obj Buffer input call (that have parameter already check)
	 * @param[in] _pointer Pointer on the class to call
	 * @param[in] _func pointer on the function to call
	 */
	template <class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	void executeClassCall(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
	                      ememory::SharedPtr<zeus::BufferParameter> _obj,
	                      ZEUS_CLASS_TYPE* _pointer,
	                      void (ZEUS_CLASS_TYPE::*_func)(ZEUS_TYPES...)) {
		if (_obj == nullptr) {
			return;
		}
		#if defined(__clang__)
			// clang generate a basic warning:
			//      warning: multiple unsequenced modifications to 'idParam' [-Wunsequenced]
			int32_t idParam = 0;
			(*_pointer.*_func)(_obj->getParameter<ZEUS_TYPES>(idParam++)...);
		#elif defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
			int32_t idParam = int32_t(sizeof...(ZEUS_TYPES))-1;
			(*_pointer.*_func)(_obj->getParameter<ZEUS_TYPES>(idParam--)...);
		#else
			#error Must be implemented ...
			return;
		#endif
		_interfaceClient->addAsync([=](WebServer* _interface) {
			    _interface->answerVoid(_obj->getTransactionId(), _obj->getClientId());
			    return true;
			});
	}
	/**
	 * @brief Chass that permit to declare a function that call intanced element or a class element
	 */
	template <class ZEUS_RETURN, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	class AbstractFunctionTypeClass: public zeus::AbstractFunction {
		protected:
			static const ParamType m_returnType;
			static const ParamType m_paramType[sizeof...(ZEUS_TYPES)];
		public:
			using functionType = ZEUS_RETURN (ZEUS_CLASS_TYPE::*)(ZEUS_TYPES...);
			functionType m_function;
			/**
			 * @brief Constructor
			 * @param[in] _name Name of the function
			 * @param[in] _fptr Pointer on the function
			 */
			AbstractFunctionTypeClass(const std::string& _name, functionType _fptr):
			  AbstractFunction(_name),
			  m_function(_fptr) {
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
			void execute(ememory::SharedPtr<zeus::WebServer> _interfaceClient,
			             ememory::SharedPtr<zeus::BufferCall> _obj,
			             void* _class) override {
				if (_obj == nullptr) {
					return;
				}
				ZEUS_CLASS_TYPE* tmpClass = nullptr;
				if (_class != nullptr) {
					tmpClass = (ZEUS_CLASS_TYPE*)_class;
				}
				
				// check parameter number
				if (_obj->getNumberParameter() != sizeof...(ZEUS_TYPES)) {
					ZEUS_ERROR("Wrong number of Parameters ...");
					std::string help = "request ";
					help += etk::to_string(_obj->getNumberParameter());
					help += " parameters and need ";
					help += etk::to_string(sizeof...(ZEUS_TYPES));
					help += " parameters. prototype function:";
					help += getPrototype();
					_interfaceClient->answerError(_obj->getTransactionId(),
					                              "WRONG-PARAMETER-NUMBER",
					                              help,
					                              _obj->getClientId());
					return;
				}
				// check parameter compatibility
				for (size_t iii=0; iii<sizeof...(ZEUS_TYPES); ++iii) {
					if (checkCompatibility(m_paramType[iii], _obj->getParameterType(iii)) == false) {
						_interfaceClient->answerError(_obj->getTransactionId(),
						                              "WRONG-PARAMETER-TYPE",
						                              std::string("Parameter id ") + etk::to_string(iii) + " not compatible with type: '" + m_paramType[iii].getName() + "'",
						                              _obj->getClientId());
						return;
					}
				}
				// execute cmd:
				zeus::executeClassCall(_interfaceClient, _obj, tmpClass, m_function);
			}
	};
	// specialization
	template <class ZEUS_RETURN, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	const ParamType AbstractFunctionTypeClass<ZEUS_RETURN, ZEUS_CLASS_TYPE, ZEUS_TYPES...>::m_returnType = createType<ZEUS_RETURN>();
	// specialization
	template <class ZEUS_RETURN, class ZEUS_CLASS_TYPE, class... ZEUS_TYPES>
	const ParamType AbstractFunctionTypeClass<ZEUS_RETURN, ZEUS_CLASS_TYPE, ZEUS_TYPES...>::m_paramType[sizeof...(ZEUS_TYPES)] = {createType<ZEUS_TYPES>()...};
	/**
	 * @brief Create a function information with the function type
	 * @param[in] _name Name of the function
	 * @param[in] _fffp Pointer of the function
	 * @return Abstract type of the function
	 */
	template <typename ZEUS_RETURN, class ZEUS_CLASS_TYPE, typename... ZEUS_TYPES>
	AbstractFunction* createAbstractFunctionClass(const std::string& _name, ZEUS_RETURN (ZEUS_CLASS_TYPE::*_fffp)(ZEUS_TYPES...)) {
		return new AbstractFunctionTypeClass<ZEUS_RETURN, ZEUS_CLASS_TYPE, ZEUS_TYPES...>(_name, _fffp);
	}
}

