/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <etk/types.hpp>
#include <zeus/message/Message.hpp>
#include <zeus/message/Data.hpp>
#include <zeus/debug.hpp>
#include <zeus/message/ParamType.hpp>
#include <etk/stdTools.hpp>
#include <zeus/AbstractFunction.hpp>
#include <climits>
#include <zeus/mineType.hpp>
#include <zeus/WebServer.hpp>


void zeus::message::addType(etk::Vector<uint8_t>& _data, zeus::message::ParamType _type) {
	_data.pushBack(uint8_t(_type.getId()>>8));
	_data.pushBack(uint8_t(_type.getId()));
}

void zeus::message::addTypeObject(etk::Vector<uint8_t>& _data, const etk::String _type) {
	_data.pushBack(uint8_t(zeus::message::paramTypeObject>>8));
	_data.pushBack(uint8_t(zeus::message::paramTypeObject));
	for (auto &it : _type) {
		_data.pushBack(uint8_t(it));
	}
	_data.pushBack(0);
}


void zeus::message::Parameter::addParameter() {
	etk::Vector<uint8_t> data;
	addType(data, createType<void>());
	m_parameter.pushBack(etk::makePair(2,data));
}
void zeus::message::Parameter::addParameterEmptyVector() {
	// special case of json change mode
	etk::Vector<uint8_t> data;
	addType(data, createType<etk::Vector<void>>());
	m_parameter.pushBack(etk::makePair(2,data));
}
namespace zeus {
	namespace message {
		template<>
		void Parameter::addParameter<etk::String>(uint16_t _paramId, const etk::String& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::String>());
			int32_t currentOffset = data.size();
			data.resize(data.size()+_value.size());
			memcpy(&data[currentOffset], &_value[0], _value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<etk::Vector<etk::String>>(uint16_t _paramId, const etk::Vector<etk::String>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<etk::String>>());
			// count all datas:
			uint32_t size = 0;
			for (auto &it : _value) {
				size+=it.size()+1;
			}
			uint16_t nb = _value.size();
			int32_t currentOffset = data.size();
			data.resize(data.size()+size+2);
			memcpy(&data[currentOffset], &nb, sizeof(uint16_t));
			currentOffset += sizeof(uint16_t);
			for (auto &it : _value) {
				memcpy(&data[currentOffset], &it[0], it.size());
				currentOffset += it.size();
				data[currentOffset] = '\0';
				currentOffset++;
			}
			m_parameter.pushBack(etk::makePair(2,data));
		}
		
		template<>
		void Parameter::addParameter<etk::Vector<bool>>(uint16_t _paramId, const etk::Vector<bool>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<bool>>());
			// add size:
			uint16_t nb = _value.size();
			int32_t currentOffset = data.size();
			data.resize(data.size()+_value.size());
			for (const auto &it : _value) {
				if (it == true) {
					data[currentOffset] = 'T';
				} else {
					data[currentOffset] = 'F';
				}
				currentOffset++;
			}
			m_parameter.pushBack(etk::makePair(2,data));
		}
		
		template<>
		void Parameter::addParameter<etk::Vector<int8_t>>(uint16_t _paramId, const etk::Vector<int8_t>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<int8_t>>());
			// add size:
			int32_t currentOffset = data.size();
			data.resize(data.size()+sizeof(int8_t)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(int8_t)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<etk::Vector<int16_t>>(uint16_t _paramId, const etk::Vector<int16_t>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<int16_t>>());
			// add size:
			int32_t currentOffset = data.size();
			data.resize(data.size()+sizeof(int16_t)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(int16_t)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<etk::Vector<int32_t>>(uint16_t _paramId, const etk::Vector<int32_t>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<int32_t>>());
			// add size:
			int32_t currentOffset = data.size();
			data.resize(data.size()+sizeof(int32_t)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(int32_t)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<etk::Vector<int64_t>>(uint16_t _paramId, const etk::Vector<int64_t>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<int64_t>>());
			// add size:
			int32_t currentOffset = data.size();
			data.resize(data.size()+sizeof(int64_t)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(int64_t)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<etk::Vector<uint8_t>>(uint16_t _paramId, const etk::Vector<uint8_t>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<uint8_t>>());
			// add size:
			int32_t currentOffset = data.size();
			data.resize(data.size()+sizeof(uint8_t)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(uint8_t)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<etk::Vector<uint16_t>>(uint16_t _paramId, const etk::Vector<uint16_t>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<uint16_t>>());
			// add size:
			int32_t currentOffset = data.size();
			ZEUS_INFO("add " << _value.size() << " elements");
			data.resize(data.size()+sizeof(uint16_t)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(uint16_t)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<etk::Vector<uint32_t>>(uint16_t _paramId, const etk::Vector<uint32_t>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<uint32_t>>());
			// add size:
			int32_t currentOffset = data.size();
			data.resize(data.size()+sizeof(uint32_t)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(uint32_t)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<etk::Vector<uint64_t>>(uint16_t _paramId, const etk::Vector<uint64_t>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<uint64_t>>());
			// add size:
			int32_t currentOffset = data.size();
			data.resize(data.size()+sizeof(uint64_t)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(uint64_t)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		
		template<>
		void Parameter::addParameter<etk::Vector<float>>(uint16_t _paramId, const etk::Vector<float>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<float>>());
			// add size:
			int32_t currentOffset = data.size();
			data.resize(data.size()+sizeof(float)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(float)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		
		template<>
		void Parameter::addParameter<etk::Vector<double>>(uint16_t _paramId, const etk::Vector<double>& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<etk::Vector<double>>());
			// add size:
			int32_t currentOffset = data.size();
			data.resize(data.size()+sizeof(double)*_value.size());
			memcpy(&data[currentOffset], &_value[0], sizeof(double)*_value.size());
			m_parameter.pushBack(etk::makePair(2,data));
		}
		
		template<>
		void Parameter::addParameter<int8_t>(uint16_t _paramId, const int8_t& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<int8_t>());
			data.pushBack(uint8_t(_value));
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<uint8_t>(uint16_t _paramId, const uint8_t& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<uint8_t>());
			data.pushBack(_value);
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<int16_t>(uint16_t _paramId, const int16_t& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<int16_t>());
			int32_t currentOffset = data.size();
			data.resize(data.size()+2);
			memcpy(&data[currentOffset], &_value, 2);
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<uint16_t>(uint16_t _paramId, const uint16_t& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<uint16_t>());
			int32_t currentOffset = data.size();
			data.resize(data.size()+2);
			memcpy(&data[currentOffset], &_value, 2);
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<int32_t>(uint16_t _paramId, const int32_t& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<int32_t>());
			int32_t currentOffset = data.size();
			data.resize(data.size()+4);
			memcpy(&data[currentOffset], &_value, 4);
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<uint32_t>(uint16_t _paramId, const uint32_t& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<uint32_t>());
			int32_t currentOffset = data.size();
			data.resize(data.size()+4);
			memcpy(&data[currentOffset], &_value, 4);
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<int64_t>(uint16_t _paramId, const int64_t& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<int64_t>());
			int32_t currentOffset = data.size();
			data.resize(data.size()+8);
			memcpy(&data[currentOffset], &_value, 8);
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<uint64_t>(uint16_t _paramId, const uint64_t& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<uint64_t>());
			int32_t currentOffset = data.size();
			data.resize(data.size()+8);
			memcpy(&data[currentOffset], &_value, 8);
			m_parameter.pushBack(etk::makePair(2,data));
		}
		#if    defined(__TARGET_OS__MacOs) \
		    || defined(__TARGET_OS__IOs)
		template<>
		void Parameter::addParameter<size_t>(uint16_t _paramId, const size_t& _value) {
			addParameter(_paramId, uint64_t(_value));
		}
		#endif
		template<>
		void Parameter::addParameter<float>(uint16_t _paramId, const float& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<float>());
			int32_t currentOffset = data.size();
			data.resize(data.size()+4);
			memcpy(&data[currentOffset], &_value, 4);
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<double>(uint16_t _paramId, const double& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<double>());
			int32_t currentOffset = data.size();
			data.resize(data.size()+8);
			memcpy(&data[currentOffset], &_value, 8);
			m_parameter.pushBack(etk::makePair(2,data));
		}
		template<>
		void Parameter::addParameter<bool>(uint16_t _paramId, const bool& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<bool>());
			if (_value == true) {
				data.pushBack('T');
			} else {
				data.pushBack('F');
			}
			m_parameter.pushBack(etk::makePair(2,data));
		}
		#define ZEUS_MINIMUM_SIZE_MULTIPLE (1024*50)
		class SendData {
			private:
				zeus::Raw m_data;
				uint16_t m_parameterId;
				int64_t m_size;
				uint64_t m_offset;
			public:
				SendData(const zeus::Raw& _data, uint16_t _parameterId) :
				  m_data(_data),
				  m_parameterId(_parameterId),
				  m_size(_data.size()),
				  m_offset(0) {
					
				}
				~SendData() {
					
				}
				// TODO : Set it with a sharedPtr instaed of pointer ...
				bool operator() (zeus::WebServer* _interface,
				                 uint32_t _source,
				                 uint32_t _destination,
				                 uint32_t _transactionId,
				                 uint32_t _partId) {
					ememory::SharedPtr<zeus::message::Data> answer = zeus::message::Data::create(_interface->sharedFromThis());
					answer->setTransactionId(_transactionId);
					answer->setSource(_source);
					answer->setDestination(_destination);
					answer->setPartId(_partId);
					answer->setPartFinish(false);
					int32_t tmpSize = ZEUS_MINIMUM_SIZE_MULTIPLE;
					if (m_size < ZEUS_MINIMUM_SIZE_MULTIPLE) {
						tmpSize = m_size;
					}
					answer->addData(m_parameterId, (void *)(&m_data.data()[m_offset]), tmpSize);
					m_size -= tmpSize;
					m_offset += tmpSize;
					_interface->writeBinary(answer);
					if (m_size <= 0) {
						return true;
					}
					return false;
				}
		};
		template<>
		void Parameter::addParameter<zeus::Raw>(uint16_t _paramId, const zeus::Raw& _value) {
			etk::Vector<uint8_t> data;
			addType(data, createType<zeus::Raw>());
			// set mine type in string:
			int32_t currentOffset = data.size();
			if (_value.size() != 0) {
				if (_value.size() < ZEUS_MINIMUM_SIZE_MULTIPLE) {
					data.resize(data.size()+_value.size());
					memcpy(&data[currentOffset], _value.data(), _value.size());
				} else {
					m_multipleSend.pushBack(zeus::message::SendData(_value, _paramId));
				}
			}
			m_parameter.pushBack(etk::makePair(2,data));
		}
	}
}

