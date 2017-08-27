/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/types.hpp>
#include <ewol/debug.hpp>
#include <ewol/widget/Composer.hpp>
#include <esignal/Signal.hpp>
#include <appl/widget/ProgressBar.hpp>
#include <appl/widget/VolumeBar.hpp>
#include <appl/ClientProperty.hpp>

namespace appl {
	namespace widget {
		class UpBar;
		using UpBarShared = ememory::SharedPtr<appl::widget::UpBar>;
		using UpBarWeak = ememory::WeakPtr<appl::widget::UpBar>;
		class UpBar : public ewol::widget::Composer {
			public: // properties
				eproperty::Value<etk::String> propertyType; //!< Type of the bar
				eproperty::Value<float> propertyValue; //!< value of the bar ...
			public: // signals
				esignal::Signal<float> signalChange; //!< Value of the bar
			protected:
				UpBar();
				void init() override;
			public:
				DECLARE_WIDGET_FACTORY(UpBar, "appl_UpBar");
				virtual ~UpBar();
			private:
				// callback functions:
				void onCallbackLightRequest(const float& _value);
				void onCallbackVolumeRequest(const float& _value);
				void onCallbackHide(const float& _value);
				void onChangePropertyType();
				void onChangePropertyValue();
		};
	};
};
