/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <zeus/Object.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>

#include <ethread/Mutex.hpp>
#include <ejson/ejson.hpp>

#include <etk/stdTools.hpp>

#include <zeus/service/User.hpp>
#include <zeus/service/registerUser.hpp>
#include <zeus/ProxyClientProperty.hpp>

static ethread::Mutex g_mutex;
static etk::Uri g_basePath;
static etk::String g_baseDBName = etk::String(SERVICE_NAME) + "-database.json";
static ejson::Document g_database;

namespace appl {
	class SystemService : public zeus::service::User {
		private:
			zeus::ProxyClientProperty m_client;
			etk::String m_userName;
		public:
			/*
			SystemService(ememory::SharedPtr<zeus::ClientProperty> _client, const etk::String& _userName) :
			  m_client(_client),
			  m_userName(_userName) {
				APPL_WARNING("New SystemService ... for user: ");
			}
			*/
			SystemService(uint16_t _clientId) {
				APPL_WARNING("New SystemService ... for user: " << _clientId);
			}
			~SystemService() {
				APPL_WARNING("Delete service-user interface.");
			}
		public:
			etk::Vector<etk::String> clientGroupsGet(etk::String _clientName) {
				APPL_WARNING("call clientGroupsGet : " << _clientName);
				etk::Vector<etk::String> out;
				/*
				if (m_client == null) {
					return out;
				}
				*/
				// TODO: check if basished ...
				/*
				if (m_client.getName().get() != "") {
					ethread::UniqueLock lock(g_mutex);
					etk::Vector<etk::String> out;
					ejson::Object clients = g_database["client"].toObject();
					if (clients.exist() == false) {
						// Section never created
						return out;
					}
					ejson::Object client = clients[m_client.getName().get()].toObject();
					if (clients.exist() == false) {
						// No specificity for this client (in case it have no special right)
						return out;
					}
					if (client["tocken"].toString().get() != "") {
						out.pushBack("connected");
					}
					// TODO: check banishing ...
					ejson::Array groups = client["group"].toArray();
					for (auto it : groups) {
						out.pushBack(it.toString().get());
					}
				}
				*/
				// TODO: Check default visibility ... (if user want to have default visibility at Noone ==> then public must be removed...
				if (true) {
					out.pushBack("public");
				}
				return out;
			}
			bool checkTocken(etk::String _clientName, etk::String _tocken) {
				ethread::UniqueLock lock(g_mutex);
				APPL_INFO("Check TOCKEN for : '" << _clientName << "' tocken='" << _clientName << "'");
				ejson::Object clients = g_database["client"].toObject();
				if (clients.exist() == false) {
					APPL_INFO("     ==> return false");
					// Section never created
					return false;
				}
				ejson::Object client = clients[_clientName].toObject();
				if (clients.exist() == false) {
					APPL_INFO("     ==> return false");
					// No specificity for this client (in case it have no special right)
					return false;
				}
				// TODO: check banishing ...
				// TODO: Do it better ...
				etk::String registerTocken = client["tocken"].toString().get();
				if (registerTocken == _tocken) {
					APPL_INFO("     ==> return true");
					return true;
				}
				APPL_INFO("     ==> return false");
				return false;
			}
			bool checkAuth(etk::String _password) {
				ethread::UniqueLock lock(g_mutex);
				APPL_INFO("Check AUTH for : '" << _password << "'");
				etk::String pass = g_database["password"].toString().get();
				if (pass == "") {
					// pb password
					return false;
				}
				if (pass == _password) {
					return true;
				}
				return false;
			}
			etk::Vector<etk::String> filterClientServices(etk::String _clientName, etk::Vector<etk::String> _currentList) {
				ethread::UniqueLock lock(g_mutex);
				APPL_INFO("Filter services : '" << _clientName << "' " << _currentList);
				// When connected to our session ==> we have no control access ...
				if (_clientName == m_userName) {
					APPL_INFO("    return all");
					return _currentList;
				}
				etk::Vector<etk::String> out;
				APPL_TODO("Filter service list ==> not implemented...");
				return out;
			}
	};
}

ETK_EXPORT_API bool SERVICE_IO_init(int _argc, const char *_argv[], etk::Uri _basePath) {
	g_basePath = _basePath;
	ethread::UniqueLock lock(g_mutex);
	APPL_WARNING("Load USER: " << g_basePath);
	etk::Uri db = g_basePath;
	db.setPath(g_basePath.getPath() / g_baseDBName);
	bool ret = g_database.load(db);
	if (ret == false) {
		APPL_WARNING("    ==> LOAD error");
	}
	return true;
}

ETK_EXPORT_API bool SERVICE_IO_uninit() {
	ethread::UniqueLock lock(g_mutex);
	APPL_DEBUG("Store User Info:");
	etk::Uri db = g_basePath;
	db.setPath(g_basePath.getPath() / g_baseDBName);
	bool ret = g_database.storeSafe(db);
	if (ret == false) {
		APPL_WARNING("    ==> Store error");
		return false;
	}
	APPL_WARNING("delete USER [STOP]");
	return true;
}

ZEUS_SERVICE_USER_DECLARE(appl::SystemService);
