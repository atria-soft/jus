/** @file
 * @author Edouard DUPIN
 * @copyright 2016, Edouard DUPIN, all right reserved
 * @license GPL v3 (see license file)
 */

#pragma once

#include <ewol/widget/Widget.hpp>
#include <ewol/compositing/Image.hpp>
#include <ewol/compositing/Text.hpp>
#include <ewol/compositing/Drawing.hpp>
#include <ewol/widget/Manager.hpp>
#include <esignal/Signal.hpp>

namespace appl {
	class ClientProperty {
		public:
			std::string login;
			std::string pass;
			std::string address;
			uint16_t port;
	};
	namespace widget {
		class ListViewer : public ewol::Widget {
			protected:
				ewol::compositing::Image m_compImageVideo;
				ewol::compositing::Image m_compImageAudio;
				ewol::compositing::Text m_text;
				ewol::compositing::Drawing m_draw;
				ememory::SharedPtr<ClientProperty> m_clientProp;
			protected:
				//! @brief constructor
				ListViewer();
				void init() override;
			public:
				DECLARE_WIDGET_FACTORY(ListViewer, "ListViewer");
				//! @brief destructor
				virtual ~ListViewer();
			public:
				void onDraw() override;
				void onRegenerateDisplay() override;
			public:
				void setClientProperty(ememory::SharedPtr<ClientProperty> _prop) {
					m_clientProp = _prop;
				}
		};
	}
}




