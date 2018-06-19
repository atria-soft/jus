/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <etk/types.hpp>
#include <zeus/message/Message.hpp>
#include <zeus/debug.hpp>
#include <zeus/message/ParamType.hpp>
#include <etk/stdTools.hpp>
#include <zeus/AbstractFunction.hpp>
#include <zeus/ObjectRemote.hpp>
#include <zeus/Raw.hpp>
#include <climits>
namespace zeus {
	namespace message {
		template<>
		bool Parameter::getParameter<bool>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			if (createType<bool>() != type) {
				return 0;
			}
			const char* pointer2 = reinterpret_cast<const char*>(pointer);
			if (    *pointer2 == 'T'
			     || *pointer2 == '1'
			     || *pointer2 == 1) {
				return true;
			}
			return false;
		}
		
		template<>
		etk::String Parameter::getParameter<etk::String>(int32_t _id) const {
			etk::String out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			out.resize(dataSize, 0);
			memcpy(&out[0], pointer, out.size());
			return out;
		}
		
		// NOTE: The use of reinterpret cast is correct for all the generic machine but not for 
		//       the small ex: ARM, whe the pointer need to be alligned ex: int64_t have all 
		//       time an address where @%4 = 0.
		//       The it is better to use the copy of valu at the correct positions ...
		// TODO: Do it work for CPU in BIG endien ...
		
		template<>
		uint8_t Parameter::getParameter<uint8_t>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (createType<uint8_t>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				return *tmp;
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint16_t(UCHAR_MAX));
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint32_t(UCHAR_MAX));
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint64_t(UCHAR_MAX));
			} else if (createType<int8_t>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				return etk::max(int8_t(0), *tmp);
			} else if (createType<int16_t>() == type) {
				int16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int16_t(0), tmp, int16_t(UCHAR_MAX));
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int32_t(0), tmp, int32_t(UCHAR_MAX));
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int64_t(0), tmp, int64_t(UCHAR_MAX));
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return uint8_t(etk::avg(float(0), tmp, float(UCHAR_MAX)));
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return uint8_t(etk::avg(double(0), tmp, double(UCHAR_MAX)));
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0;
		}
		template<>
		uint16_t Parameter::getParameter<uint16_t>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (createType<uint8_t>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				return *tmp;
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint32_t(USHRT_MAX));
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint64_t(USHRT_MAX));
			} else if (createType<int8_t>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				return etk::max(int8_t(0), *tmp);
			} else if (createType<int16_t>() == type) {
				int16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::max(int16_t(0), tmp);
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int32_t(0), tmp, int32_t(USHRT_MAX));
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int64_t(0), tmp, int64_t(USHRT_MAX));
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return uint16_t(etk::avg(float(0), tmp, float(USHRT_MAX)));
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return uint16_t(etk::avg(double(0), tmp, double(USHRT_MAX)));
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0;
		}
		
		template<>
		uint32_t Parameter::getParameter<uint32_t>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			//ZEUS_WARNING("get type " << type << "   with size=" << dataSize);
			if (createType<uint8_t>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				return *tmp;
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint64_t(ULONG_MAX));
			} else if (createType<int8_t>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				return etk::max(int8_t(0), *tmp);
			} else if (createType<int16_t>() == type) {
				int16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::max(int16_t(0), tmp);
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::max(int32_t(0), tmp);
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int64_t(0), tmp, int64_t(ULONG_MAX));
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return uint32_t(etk::avg(float(0), tmp, float(ULONG_MAX)));
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return uint32_t(etk::avg(double(0), tmp, double(ULONG_MAX)));
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0;
		}
		
		template<>
		uint64_t Parameter::getParameter<uint64_t>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			uint32_t dataSize = getParameterSize(_id);
			uint8_t* pointer = const_cast<uint8_t*>(getParameterPointer(_id));
			//ZEUS_WARNING("get type " << type << "   with size=" << dataSize << " pointer=" << uint64_t(pointer) << " sizeof(uint64_t)=" << int32_t(sizeof(uint64_t)));
			// TODO : Check size ...
			if (createType<uint8_t>() == type) {
				uint8_t* tmp = reinterpret_cast<uint8_t*>(pointer);
				return *tmp;
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int8_t>() == type) {
				int8_t* tmp = reinterpret_cast<int8_t*>(pointer);
				return etk::max(int8_t(0), *tmp);
			} else if (createType<int16_t>() == type) {
				int16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::max(int16_t(0), tmp);
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::max(int32_t(0), tmp);
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::max(int64_t(0), tmp);
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return uint64_t(etk::avg(float(0), tmp, float(ULONG_MAX)));
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return uint64_t(etk::avg(double(0), tmp, double(ULONG_MAX)));
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0;
		}
		
		#if    defined(__TARGET_OS__MacOs) \
		    || defined(__TARGET_OS__IOs)
		template<>
		size_t Parameter::getParameter<size_t>(int32_t _id) const {
			return getParameter<uint64_t>(_id);
		}
		#endif
		template<>
		int8_t Parameter::getParameter<int8_t>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (createType<uint8_t>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				return etk::min(*tmp, uint8_t(SCHAR_MAX));
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint16_t(SCHAR_MAX));
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint32_t(SCHAR_MAX));
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint64_t(SCHAR_MAX));
			} else if (createType<int8_t>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				return *tmp;
			} else if (createType<int16_t>() == type) {
				int16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int16_t(SCHAR_MIN), tmp, int16_t(SCHAR_MAX));
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int32_t(SCHAR_MIN), tmp, int32_t(SCHAR_MAX));
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int64_t(SCHAR_MIN), tmp, int64_t(SCHAR_MAX));
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return int8_t(etk::avg(float(SCHAR_MIN), tmp, float(SCHAR_MAX)));
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return int8_t(etk::avg(double(SCHAR_MIN), tmp, double(SCHAR_MAX)));
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0;
		}
		
		template<>
		int16_t Parameter::getParameter<int16_t>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (createType<uint8_t>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				return *tmp;
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint16_t(SHRT_MAX));
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint32_t(SHRT_MAX));
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint64_t(SHRT_MAX));
			} else if (createType<int8_t>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				return *tmp;
			} else if (createType<int16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int32_t(SHRT_MIN), tmp, int32_t(SHRT_MAX));
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int64_t(SHRT_MIN), tmp, int64_t(SHRT_MAX));
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return int16_t(etk::avg(float(SHRT_MIN), tmp, float(SHRT_MAX)));
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return int16_t(etk::avg(double(SHRT_MIN), tmp, double(SHRT_MAX)));
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0;
		}
		
		template<>
		int32_t Parameter::getParameter<int32_t>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			//ZEUS_WARNING("get type " << type << "   with size=" << dataSize);
			if (createType<uint8_t>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				return *tmp;
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint32_t(LONG_MAX));
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint64_t(LONG_MAX));
			} else if (createType<int8_t>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				return *tmp;
			} else if (createType<int16_t>() == type) {
				int16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::avg(int64_t(LONG_MIN), tmp, int64_t(LONG_MAX));
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return int32_t(etk::avg(float(LONG_MIN), tmp, float(LONG_MAX)));
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return int32_t(etk::avg(double(LONG_MIN), tmp, double(LONG_MAX)));
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0;
		}
		
		template<>
		int64_t Parameter::getParameter<int64_t>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			//ZEUS_WARNING("get type " << type << "   with size=" << dataSize << " pointer=" << uint64_t(pointer));
			if (createType<uint8_t>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				return *tmp;
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return etk::min(tmp, uint64_t(LLONG_MAX));
			} else if (createType<int8_t>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				return *tmp;
			} else if (createType<int16_t>() == type) {
				int16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return int64_t(etk::avg(float(LLONG_MIN), tmp, float(LLONG_MAX)));
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return int64_t(etk::avg(double(LLONG_MIN), tmp, double(LLONG_MAX)));
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0;
		}
		
		template<>
		float Parameter::getParameter<float>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (createType<uint8_t>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				return *tmp;
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int8_t>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				return *tmp;
			} else if (createType<int16_t>() == type) {
				int16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0.0f;
		}
		template<>
		double Parameter::getParameter<double>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (createType<uint8_t>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				return *tmp;
			} else if (createType<uint16_t>() == type) {
				uint16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint32_t>() == type) {
				uint32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<uint64_t>() == type) {
				uint64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int8_t>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				return *tmp;
			} else if (createType<int16_t>() == type) {
				int16_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int32_t>() == type) {
				int32_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<int64_t>() == type) {
				int64_t tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<float>() == type) {
				float tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			} else if (createType<double>() == type) {
				double tmp;
				memcpy(&tmp, pointer, sizeof(tmp));
				return tmp;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return 0.0;
		}
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		template<>
		etk::Vector<uint8_t> Parameter::getParameter<etk::Vector<uint8_t>>(int32_t _id) const {
			etk::Vector<uint8_t> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(uint8_t));
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint16_t(UCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint32_t(UCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint64_t(UCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int8_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int16_t(0), tmp[iii], int16_t(UCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int32_t(0), tmp[iii], int32_t(UCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int64_t(0), tmp[iii], int64_t(UCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				const float* tmp = reinterpret_cast<const float*>(pointer);
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = uint8_t(etk::avg(float(0), tmp[iii], float(UCHAR_MAX)));
				}
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				const double* tmp = reinterpret_cast<const double*>(pointer);
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = uint8_t(etk::avg(double(0), tmp[iii], double(UCHAR_MAX)));
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		template<>
		etk::Vector<uint16_t> Parameter::getParameter<etk::Vector<uint16_t>>(int32_t _id) const {
			etk::Vector<uint16_t> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(uint16_t));
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint32_t(USHRT_MAX));
				}
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint64_t(USHRT_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int8_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int16_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int32_t(0), tmp[iii], int32_t(USHRT_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int64_t(0), tmp[iii], int64_t(USHRT_MAX));
				}
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				const float* tmp = reinterpret_cast<const float*>(pointer);
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = uint16_t(etk::avg(float(0), tmp[iii], float(USHRT_MAX)));
				}
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				const double* tmp = reinterpret_cast<const double*>(pointer);
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = uint16_t(etk::avg(double(0), tmp[iii], double(USHRT_MAX)));
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<uint32_t> Parameter::getParameter<etk::Vector<uint32_t>>(int32_t _id) const {
			etk::Vector<uint32_t> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(uint32_t));
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint64_t(ULONG_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int8_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int16_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int32_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int64_t(0), tmp[iii], int64_t(ULONG_MAX));
				}
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				const float* tmp = reinterpret_cast<const float*>(pointer);
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = uint32_t(etk::avg(float(0), tmp[iii], float(ULONG_MAX)));
				}
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				const double* tmp = reinterpret_cast<const double*>(pointer);
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = uint32_t(etk::avg(double(0), tmp[iii], double(ULONG_MAX)));
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<uint64_t> Parameter::getParameter<etk::Vector<uint64_t>>(int32_t _id) const {
			etk::Vector<uint64_t> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(uint64_t));
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int8_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int16_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int32_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::max(int64_t(0), tmp[iii]);
				}
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				const float* tmp = reinterpret_cast<const float*>(pointer);
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = uint64_t(etk::avg(float(0), tmp[iii], float(ULONG_MAX)));
				}
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				const double* tmp = reinterpret_cast<const double*>(pointer);
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = uint64_t(etk::avg(double(0), tmp[iii], double(ULONG_MAX)));
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<int8_t> Parameter::getParameter<etk::Vector<int8_t>>(int32_t _id) const {
			etk::Vector<int8_t> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint8_t(SCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint16_t(SCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint32_t(SCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint64_t(SCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(int8_t));
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int16_t(SCHAR_MIN), tmp[iii], int16_t(SCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int32_t(SCHAR_MIN), tmp[iii], int32_t(SCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int64_t(SCHAR_MIN), tmp[iii], int64_t(SCHAR_MAX));
				}
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				const float* tmp = reinterpret_cast<const float*>(pointer);
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = int8_t(etk::avg(float(SCHAR_MIN), tmp[iii], float(SCHAR_MAX)));
				}
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				const double* tmp = reinterpret_cast<const double*>(pointer);
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = int8_t(etk::avg(double(SCHAR_MIN), tmp[iii], double(SCHAR_MAX)));
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<int16_t> Parameter::getParameter<etk::Vector<int16_t>>(int32_t _id) const {
			etk::Vector<int16_t> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint16_t(SHRT_MAX));
				}
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint32_t(SHRT_MAX));
				}
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint64_t(SHRT_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(int16_t));
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int32_t(SHRT_MIN), tmp[iii], int32_t(SHRT_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int64_t(SHRT_MIN), tmp[iii], int64_t(SHRT_MAX));
				}
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				const float* tmp = reinterpret_cast<const float*>(pointer);
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = int16_t(etk::avg(float(SHRT_MIN), tmp[iii], float(SHRT_MAX)));
				}
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				const double* tmp = reinterpret_cast<const double*>(pointer);
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = int16_t(etk::avg(double(SHRT_MIN), tmp[iii], double(SHRT_MAX)));
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<int32_t> Parameter::getParameter<etk::Vector<int32_t>>(int32_t _id) const {
			etk::Vector<int32_t> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint32_t(LONG_MAX));
				}
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint64_t(LONG_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(int32_t));
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::avg(int64_t(LONG_MIN), tmp[iii], int64_t(LONG_MAX));
				}
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				const float* tmp = reinterpret_cast<const float*>(pointer);
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = int32_t(etk::avg(float(LONG_MIN), tmp[iii], float(LONG_MAX)));
				}
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				const double* tmp = reinterpret_cast<const double*>(pointer);
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = int32_t(etk::avg(double(LONG_MIN), tmp[iii], double(LONG_MAX)));
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<int64_t> Parameter::getParameter<etk::Vector<int64_t>>(int32_t _id) const {
			etk::Vector<int64_t> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = etk::min(tmp[iii], uint64_t(LLONG_MAX));
				}
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(int64_t));
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				const float* tmp = reinterpret_cast<const float*>(pointer);
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = int64_t(etk::avg(float(LLONG_MIN), tmp[iii], float(LLONG_MAX)));
				}
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				const double* tmp = reinterpret_cast<const double*>(pointer);
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = int64_t(etk::avg(double(LLONG_MIN), tmp[iii], double(LLONG_MAX)));
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<float> Parameter::getParameter<etk::Vector<float>>(int32_t _id) const {
			etk::Vector<float> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(float));
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				const double* tmp = reinterpret_cast<const double*>(pointer);
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<double> Parameter::getParameter<etk::Vector<double>>(int32_t _id) const {
			etk::Vector<double> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<uint8_t>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint16_t>>() == type) {
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint32_t>>() == type) {
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<uint64_t>>() == type) {
				const uint64_t* tmp = reinterpret_cast<const uint64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int8_t>>() == type) {
				const int8_t* tmp = reinterpret_cast<const int8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int16_t>>() == type) {
				const int16_t* tmp = reinterpret_cast<const int16_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int16_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int32_t>>() == type) {
				const int32_t* tmp = reinterpret_cast<const int32_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int32_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<int64_t>>() == type) {
				const int64_t* tmp = reinterpret_cast<const int64_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(int64_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<float>>() == type) {
				const float* tmp = reinterpret_cast<const float*>(pointer);
				int32_t nbElement = dataSize / sizeof(float);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii];
				}
				return out;
			} else if (createType<etk::Vector<double>>() == type) {
				int32_t nbElement = dataSize / sizeof(double);
				out.resize(nbElement);
				memcpy(&out[0], pointer, nbElement * sizeof(double));
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<bool> Parameter::getParameter<etk::Vector<bool>>(int32_t _id) const {
			etk::Vector<bool> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<bool>>() == type) {
				const uint8_t* tmp = reinterpret_cast<const uint8_t*>(pointer);
				int32_t nbElement = dataSize / sizeof(uint8_t);
				out.resize(nbElement);
				for (size_t iii=0; iii<nbElement; ++iii) {
					out[iii] = tmp[iii] == 'T';
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		
		template<>
		etk::Vector<etk::String> Parameter::getParameter<etk::Vector<etk::String>>(int32_t _id) const {
			etk::Vector<etk::String> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			if (type == createType<etk::Vector<void>>()) {
				return out;
			} else if (createType<etk::Vector<etk::String>>() == type) {
				// first element is the number of elements:
				const uint16_t* tmp = reinterpret_cast<const uint16_t*>(pointer);
				out.resize(*tmp);
				pointer += sizeof(uint16_t);
				//each string is separated with a \0:
				for (int32_t iii=0; iii<out.size(); ++iii) {
					const char* tmp2 = reinterpret_cast<const char*>(pointer);
					out[iii] = tmp2;
					pointer += out[iii].size() + 1;
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
		template<>
		zeus::Raw Parameter::getParameter<zeus::Raw>(int32_t _id) const {
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (createType<zeus::Raw>() == type) {
				return zeus::Raw(dataSize, pointer);
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return zeus::Raw();
		}
		template<>
		ememory::SharedPtr<zeus::ObjectRemoteBase> Parameter::getParameter<ememory::SharedPtr<zeus::ObjectRemoteBase>>(int32_t _id) const {
			ememory::SharedPtr<zeus::ObjectRemoteBase> out;
			zeus::message::ParamType type = getParameterType(_id);
			const uint8_t* pointer = getParameterPointer(_id);
			uint32_t dataSize = getParameterSize(_id);
			// TODO : Check size ...
			if (type.getId() == zeus::message::paramTypeObject) {
				// Get the type string of the parameter:
				ZEUS_VERBOSE("Get type : " << type.getName());
				ZEUS_VERBOSE("Get id : " << getSourceId() << "/" << getSourceObjectId());
				const uint32_t* tmp = reinterpret_cast<const uint32_t*>(pointer);
				uint32_t objectAdress = *tmp;
				ZEUS_VERBOSE("Get id : " << (*tmp>>16) << "/" << (*tmp&0xFFFF));
				
				// get new local ID:
				ememory::SharedPtr<zeus::WebServer> iface = m_iface;
				if (iface != null) {
					uint16_t id    = iface->getAddress();
					uint16_t idObj = iface->getNewObjectId();
					// Sent to the interface the change of links from the curent interface to the real object remote ... (the client interface can control that the user calle r is athorised to do it ...
					iface->call(getDestination(), objectAdress&0xFFFF0000, "movelink", objectAdress, (uint32_t(id)<<16)+idObj);
					out = ememory::makeShared<zeus::ObjectRemoteBase>(iface, id, idObj, objectAdress, type.getName());
					iface->addWebObjRemote(out);
				} else {
					ZEUS_ERROR("missing interface to crate object: '" << type << "'");
				}
				return out;
			}
			ZEUS_ERROR("Can not get type from '" << type << "'");
			return out;
		}
	}
}

