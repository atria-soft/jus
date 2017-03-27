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
#include <esignal/Signal.hpp>

namespace appl {
	namespace widget {
		class ProgressBar;
		using ProgressBarShared = ememory::SharedPtr<appl::widget::ProgressBar>;
		using ProgressBarWeak = ememory::WeakPtr<appl::widget::ProgressBar>;
		/**
		 * @ingroup ewolWidgetGroup
		 */
		class ProgressBar : public ewol::Widget {
			public: // signals
				esignal::Signal<float> signalChange;
			public:
				//eproperty::Value<std::string> propertyShape; //!< name of the shape used
				eproperty::Value<float> propertyValue; //!< current value of the ProgressBar
				eproperty::Value<float> propertyMaximum; //!< maximum value of the ProgressBar
			protected:
				ProgressBar();
			public:
				DECLARE_WIDGET_FACTORY(ProgressBar, "appl_ProgressBar");
				virtual ~ProgressBar();
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
				std::vector<std::pair<float,float>> m_listAvaillable;
			public:
				void setRangeAvaillable(std::vector<std::pair<float,float>>& _values) {
					m_listAvaillable = _values;
				}
			protected:
				virtual void onChangePropertyValue();
				virtual void onChangePropertyMaximum();
		};
	}
}

