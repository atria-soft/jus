/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/types.hpp>
#include <etk/Color.hpp>
#include <appl/debug.hpp>
#include <ewol/widget/Widget.hpp>
#include <ewol/compositing/Drawing.hpp>
#include <echrono/Time.hpp>
#include <echrono/Duration.hpp>
#include <esignal/Signal.hpp>

namespace appl {
	namespace widget {
		class VolumeBar;
		using VolumeBarShared = ememory::SharedPtr<appl::widget::VolumeBar>;
		using VolumeBarWeak = ememory::WeakPtr<appl::widget::VolumeBar>;
		/**
		 * @ingroup ewolWidgetGroup
		 */
		class VolumeBar : public ewol::Widget {
			public: // signals
				esignal::Signal<float> signalChange;
				esignal::Signal<float> signalHide; //!< Hide value [0..1] ==> 0 is hidden
			public:
				//eproperty::Value<etk::String> propertyShape; //!< name of the shape used
				eproperty::Value<float> propertyValue; //!< current value of the VolumeBar
				eproperty::Value<float> propertyStep; //!< Up and down step value
				eproperty::Value<float> propertyMinimum; //!< minimum value of the VolumeBar
				eproperty::Value<float> propertyMaximum; //!< maximum value of the VolumeBar
				eproperty::Value<float> propertyDanger; //!< change color value
			private:
				bool m_isHidden;
				echrono::Time m_lastEventTime;
				esignal::Connection m_PCH; //!< Periodic Call Handle to remove it when needed
				/**
				 * @brief Periodic call to update grapgic display
				 * @param[in] _event Time generic event
				 */
				void periodicCall(const ewol::event::Time& _event);
			protected:
				VolumeBar();
			public:
				DECLARE_WIDGET_FACTORY(VolumeBar, "appl_VolumeBar");
				virtual ~VolumeBar();
			public:
				// TODO : Rewoek the color in the theme ...
				void setColor(etk::Color<> _newColor) {
					m_textColorFg = _newColor;
				};
			protected:
				ewol::compositing::Drawing m_draw; //!< drawing tool.
				etk::Color<> m_textColorFg; //!< Text color
				etk::Color<> m_textColorBg; //!< Background color
				etk::Color<> m_textColorDone;
				etk::Color<> m_textColorLoaded;
				void updateValue(float _newValue);
			public: // Derived function
				void onDraw() override;
				void calculateMinMaxSize() override;
				void onRegenerateDisplay() override;
				bool onEventInput(const ewol::event::Input& _event) override;
			private:
				etk::Vector<etk::Pair<float,float>> m_listAvaillable;
			public:
				void setRangeAvaillable(etk::Vector<etk::Pair<float,float>>& _values) {
					m_listAvaillable = _values;
				}
			protected:
				virtual void onChangePropertyValue();
				virtual void onChangePropertyMinimum();
				virtual void onChangePropertyMaximum();
		};
	}
}

