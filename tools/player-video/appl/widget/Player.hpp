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
#include <appl/widget/VideoPlayer.hpp>
#include <appl/ClientProperty.hpp>

namespace appl {
	namespace widget {
		class Player;
		using PlayerShared = ememory::SharedPtr<appl::widget::Player>;
		using PlayerWeak = ememory::WeakPtr<appl::widget::Player>;
		class Player : public ewol::widget::Composer {
			public: // properties
				
			public: // signals
				esignal::Signal<> signalFinished; //!< the play is finished
				esignal::Signal<> signalNext; //!< Next file is requested
				esignal::Signal<> signalPrevious; //!< Previous file is requested
				
			protected:
				Player();
				void init() override;
			public:
				DECLARE_WIDGET_FACTORY(Player, "Player");
				virtual ~Player();
			protected:
				ememory::SharedPtr<appl::widget::VideoDisplay> m_display; //!< Display widget
			public:
				void playStream(ememory::SharedPtr<appl::ClientProperty> _property, uint32_t _mediaId);
				void suspend();
			public:
				void onGetFocus() override;
			private:
				// callback functions:
				void onCallbackButtonPrevious();
				void onCallbackButtonNext();
				void onCallbackButtonPlay(const bool& _value);
				void onCallbackPosition(const echrono::Duration& _value);
				void onCallbackDuration(const echrono::Duration& _value);
				void onCallbackSeekRequest(const float& _value);
				void onCallbackFPS(const int32_t& _fps);
		};
	};
};
