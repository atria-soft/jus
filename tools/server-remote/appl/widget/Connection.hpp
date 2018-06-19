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
#include <appl/ClientProperty.hpp>

namespace appl {
	namespace widget {
		class Connection;
		using ConnectionShared = ememory::SharedPtr<appl::widget::Connection>;
		using ConnectionWeak = ememory::WeakPtr<appl::widget::Connection>;
		/**
		 *  @brief File Chooser is a simple selector of file for opening, saving, and what you want ...
		 *  
		 *  As all other pop-up methode ( wost case we can have) the creating is simple , but event back is not all the time simple:
		 *  
		 *  Fist global static declaration and inclusion:
		 *  [code style=c++]
		 *  #include <appl/widget/Connection.hpp>
		 *  [/code]
		 *  
		 *  The first step is to create the file chooser pop-up : (never in the constructor!!!)
		 *  [code style=c++]
		 *  appl::widget::ConnectionShared tmpWidget = appl::widget::Connection::create();
		 *  if (tmpWidget == null) {
		 *  	APPL_ERROR("Can not open File chooser !!! ");
		 *  	return -1;
		 *  }
		 *  // register on the Validate event:
		 *  tmpWidget->signalValidate.connect(sharedFromThis(), &****::onCallbackConnectionValidate);
		 *  // no need of this event watching ...
		 *  tmpWidget->signalCancel.connect(sharedFromThis(), &****::onCallbackConnectionCancel);
		 *  // add the widget as windows pop-up ...
		 *  ewol::widget::WindowsShared tmpWindows = getWindows();
		 *  if (tmpWindows == null) {
		 *  	APPL_ERROR("Can not get the current windows !!! ");
		 *  	return -1;
		 *  }
		 *  tmpWindows->popUpWidgetPush(tmpWidget);
		 *  [/code]
		 *  
		 *  Now we just need to wait the the open event message.
		 *  
		 *  [code style=c++]
		 *  void ****::onCallbackConnectionValidate(const ememory::SharedPtr<appl::ClientProperty>& _prop) {
		 *  	APPL_INFO("New connection : '" << _value << "'");
		 *  }
		 *  void ****::onCallbackConnectionCancel() {
		 *  	APPL_INFO("cancel connection");
		 *  }
		 *  [/code]
		 *  This is the best example of a Meta-widget.
		 */
		class Connection : public ewol::widget::Composer {
			public: // signals
				esignal::Signal<> signalCancel; //!< abort the display of the pop-up or press cancel button
				esignal::Signal<ememory::SharedPtr<appl::ClientProperty>> signalValidate; //!< select file(s)
			protected:
				ememory::SharedPtr<appl::ClientProperty> m_baseProperty;
				etk::String m_login;
				etk::String m_password;
				Connection();
				void init() override;
			public:
				DECLARE_WIDGET_FACTORY(Connection, "Connection");
				virtual ~Connection();
				void setProperty(ememory::SharedPtr<appl::ClientProperty> _baseProperty=null);
			private:
				etk::String getCompleateFileName();
				void updateCurrentFolder();
			public:
				void onGetFocus() override;
			private:
				// callback functions:
				void onCallbackButtonValidate();
				void onCallbackButtonCancel();
				void onCallbackEntryLoginChangeValue(const etk::String& _value);
				void onCallbackEntryPasswordChangeValue(const etk::String& _value);
		};
	};
};
