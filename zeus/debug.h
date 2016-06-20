/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <elog/log.h>

namespace jus {
	int32_t getLogId();
};

#define JUS_BASE(info,data) ELOG_BASE(jus::getLogId(),info,data)

#define JUS_PRINT(data)         JUS_BASE(-1, data)
#define JUS_CRITICAL(data)      JUS_BASE(1, data)
#define JUS_ERROR(data)         JUS_BASE(2, data)
#define JUS_WARNING(data)       JUS_BASE(3, data)
#ifdef DEBUG
	#define JUS_INFO(data)          JUS_BASE(4, data)
	#define JUS_DEBUG(data)         JUS_BASE(5, data)
	#define JUS_VERBOSE(data)       JUS_BASE(6, data)
	#define JUS_TODO(data)          JUS_BASE(4, "TODO : " << data)
#else
	#define JUS_INFO(data)          do { } while(false)
	#define JUS_DEBUG(data)         do { } while(false)
	#define JUS_VERBOSE(data)       do { } while(false)
	#define JUS_TODO(data)          do { } while(false)
#endif

#define JUS_ASSERT(cond,data) \
	do { \
		if (!(cond)) { \
			JUS_CRITICAL(data); \
			assert(!#cond); \
		} \
	} while (0)


