/** @file
 * @author Edouard DUPIN
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */


#include <appl/widget/Connection.hpp>
#include <ewol/widget/Sizer.hpp>
#include <ewol/widget/List.hpp>
#include <ewol/widget/Button.hpp>
#include <ewol/widget/CheckBox.hpp>
#include <ewol/widget/ListFileSystem.hpp>
#include <ewol/widget/Entry.hpp>
#include <ewol/widget/Spacer.hpp>
#include <ewol/widget/Image.hpp>
#include <ewol/widget/Composer.hpp>
#include <ewol/widget/Manager.hpp>
//#include <etk/Vector.hpp>
#include <etk/Vector.hpp>
#include <etk/tool.hpp>
#include <appl/debug.hpp>

#include <ewol/ewol.hpp>
#include <ewol/tools/message.hpp>

appl::widget::Connection::Connection() :
  signalCancel(this, "cancel", ""),
  signalValidate(this, "validate", "") {
	addObjectType("appl::widget::Connection");
}

void appl::widget::Connection::init() {
	ewol::widget::Composer::init();
	loadFromFile("DATA:///gui-connection.xml", getId());
	subBind(ewol::widget::Entry, "[" + etk::toString(getId()) + "]connect-login", signalModify, sharedFromThis(), &appl::widget::Connection::onCallbackEntryLoginChangeValue);
	subBind(ewol::widget::Entry, "[" + etk::toString(getId()) + "]connect-password", signalModify, sharedFromThis(), &appl::widget::Connection::onCallbackEntryPasswordChangeValue);
	subBind(ewol::widget::Button, "[" + etk::toString(getId()) + "]connect-bt", signalPressed, sharedFromThis(), &appl::widget::Connection::onCallbackButtonValidate);
	subBind(ewol::widget::Button, "[" + etk::toString(getId()) + "]cancel-bt", signalPressed, sharedFromThis(), &appl::widget::Connection::onCallbackButtonCancel);
	setProperty(null);
	propertyCanFocus.set(true);
}


void appl::widget::Connection::setProperty(ememory::SharedPtr<appl::ClientProperty> _baseProperty) {
	m_baseProperty = _baseProperty;
	if (m_baseProperty == null) {
		m_baseProperty = ememory::makeShared<appl::ClientProperty>();
		if (m_baseProperty == null) {
			APPL_ERROR(" can not allocate the pointer of data ==> must auto kill");
			autoDestroy();
			return;
		}
	}
	m_login = m_baseProperty->getLogin();
	m_password = m_baseProperty->getPassword();
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]connect-login", "value", m_login);
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]connect-password", "value", m_password);
}

void appl::widget::Connection::onGetFocus() {
	// transfert focus on a specific widget...
	propertySetOnWidgetNamed("[" + etk::toString(getId()) + "]connect-login", "focus", "true");
}

appl::widget::Connection::~Connection() {
	
}

void appl::widget::Connection::onCallbackEntryLoginChangeValue(const etk::String& _value) {
	m_login = _value;
}

void appl::widget::Connection::onCallbackEntryPasswordChangeValue(const etk::String& _value) {
	m_password = _value;
}

void appl::widget::Connection::onCallbackButtonValidate() {
	// ckeck if connection is valid ...
	APPL_INFO("Connect with : '" << m_login << "' ... '" << m_password << "'");
	m_baseProperty->setLogin(m_login);
	m_baseProperty->setPassword(m_password);
	m_baseProperty->connect();
	if (m_baseProperty->connection.isAlive() == false) {
		APPL_ERROR("    ==> NOT Authentify to '" << m_baseProperty->getLogin() << "'");
		ewol::tools::message::displayError("Can not connect the server with <br/>'" + m_baseProperty->getLogin() + "'");
	} else {
		APPL_INFO("    ==> Authentify with '" << m_baseProperty->getLogin() << "'");
		signalValidate.emit(m_baseProperty);
		autoDestroy();
	}
}

void appl::widget::Connection::onCallbackButtonCancel() {
	signalCancel.emit();
	autoDestroy();
}

