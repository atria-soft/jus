/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <zeus/Raw.hpp>
#include "debug.hpp"


zeus::Raw::Raw() :
  m_size(0),
  m_dataExternal(nullptr),
  m_dataInternal() {
	
}

zeus::Raw::Raw(uint32_t _size) :
  m_size(_size),
  m_dataExternal(nullptr),
  m_dataInternal() {
	m_dataInternal.resize(_size);
}

zeus::Raw::Raw(uint32_t _size, const uint8_t* _data) :
  m_size(_size),
  m_dataExternal(_data),
  m_dataInternal() {
	
}

zeus::Raw::~Raw() {
	
}


uint32_t zeus::Raw::size() const {
	return m_size;
}

const uint8_t* zeus::Raw::data() const {
	if (m_dataExternal != nullptr) {
		return m_dataExternal;
	}
	return &m_dataInternal[0];
}

uint8_t* zeus::Raw::writeData() {
	if (m_dataExternal != nullptr) {
		ZEUS_ERROR("Try to write on Data that is not allowed ...");
		return nullptr;
	}
	return &m_dataInternal[0];
}



