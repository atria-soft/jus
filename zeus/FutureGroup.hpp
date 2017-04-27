/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <zeus/FutureBase.hpp>
#include <vector>

namespace zeus {
	/**
	 * @brief Class that permit to add all waiting feture inside this group and wait only one time
	 */
	// TODO : Add the posiboilities to have a andFinished()
	class FutureGroup {
		private:
			std::vector<zeus::FutureBase> m_listFuture;
		public:
			void add(const zeus::FutureBase& _fut);
			/**
			 * @brief Wait the Future receive data
			 * @return reference on the current futur
			 */
			void wait() const;
			/**
			 * @brief Wait the Future receive data
			 * @param[in] _delta delay to wait the data arrive
			 * @return reference on the current futur
			 */
			void waitFor(echrono::Duration _delta = echrono::seconds(30)) const;
			/**
			 * @brief Wait the Future receive data
			 * @param[in] _endTime tiem to wait the data
			 * @return reference on the current futur
			 */
			void waitUntil(echrono::Steady _endTime) const;
	};
}

