/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <elog/log.h>

namespace zeus {
	int32_t getLogId();
};

#define ZEUS_BASE(info,data) ELOG_BASE(zeus::getLogId(),info,data)

#define ZEUS_PRINT(data)         ZEUS_BASE(-1, data)
#define ZEUS_CRITICAL(data)      ZEUS_BASE(1, data)
#define ZEUS_ERROR(data)         ZEUS_BASE(2, data)
#define ZEUS_WARNING(data)       ZEUS_BASE(3, data)
#ifdef DEBUG
	#define ZEUS_INFO(data)          ZEUS_BASE(4, data)
	#define ZEUS_DEBUG(data)         ZEUS_BASE(5, data)
	#define ZEUS_VERBOSE(data)       ZEUS_BASE(6, data)
	#define ZEUS_TODO(data)          ZEUS_BASE(4, "TODO : " << data)
#else
	#define ZEUS_INFO(data)          do { } while(false)
	#define ZEUS_DEBUG(data)         do { } while(false)
	#define ZEUS_VERBOSE(data)       do { } while(false)
	#define ZEUS_TODO(data)          do { } while(false)
#endif

#define ZEUS_ASSERT(cond,data) \
	do { \
		if (!(cond)) { \
			ZEUS_CRITICAL(data); \
			assert(!#cond); \
		} \
	} while (0)


