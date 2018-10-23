/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <zeus/Object.hpp>
#include <zeus/File.hpp>
#include <etk/etk.hpp>
#include <zeus/zeus.hpp>
#include <echrono/Time.hpp>

#include <ethread/Mutex.hpp>
#include <ejson/ejson.hpp>
#include <sstream>

#include <etk/stdTools.hpp>

#include <zeus/service/test/Service1.hpp>
#include <zeus/service/test/registerService1.hpp>
#include <zeus/File.hpp>
#include <zeus/ProxyFile.hpp>
#include <zeus/zeus-Media.impl.hpp>

static ethread::Mutex g_mutex;
static etk::Uri g_basePath;
static etk::String g_baseDBName = etk::String(SERVICE_NAME) + "-database.json";

static etk::Vector<ememory::SharedPtr<zeus::MediaImpl>> m_listFile;

static uint64_t m_lastMaxId = 0;
static bool g_needToStore = false;

static uint64_t createUniqueID() {
	m_lastMaxId++;
	return m_lastMaxId;
}
namespace appl {
	class TestService1 : public zeus::service::test::Service1  {
		private:
			etk::String m_userName;
		public:
			TestService1(uint16_t _clientId) {
				APPL_VERBOSE("New TestService1 ... for user: " << _clientId);
			}
			~TestService1() {
				APPL_VERBOSE("delete TestService1 ...");
			}
		public:
			uint32_t getU32(uint32_t _value) override {
				return _value*2;
			}
			
			etk::Vector<uint32_t> getVU32(etk::Vector<uint32_t> _value) override {
				for (auto &it: _value) {
					it *= 2;
				}
				return _value;
			}
			
			etk::String getString(etk::String _value) override {
				return "get:'" + _value + "'";
			}
			void getVoid() override {
				// nothing to do ...;
			}
			void doSomething(zeus::ActionNotification<int32_t>& _notifs, int32_t _value) override {
				for (int32_t iii=1; iii<=_value; ++iii) {
					//ethread::sleepMilliSeconds((500));
					_notifs.emit(iii);
				}
			}
	};
}

static void store_db() {
	APPL_INFO("Store database [START]");
	ejson::Document database;
	ejson::Array listFilesArray;
	database.add("baseValue", ejson::String("plop"));
	bool retGenerate = database.storeSafe(g_basePath / g_baseDBName);
	APPL_INFO("Store database [STOP] : " << (g_basePath / g_baseDBName) << " ret = " << retGenerate);
	g_needToStore = false;
}

static void load_db() {
	ejson::Document database;
	bool ret = database.load(g_basePath / g_baseDBName);
	if (ret == false) {
		APPL_WARNING("    ==> LOAD error");
	}
	g_needToStore = false;
}

ETK_EXPORT_API bool SERVICE_IO_init(int _argc, const char *_argv[], etk::Uri _basePath) {
	g_basePath = _basePath;
	ethread::UniqueLock lock(g_mutex);
	APPL_INFO("Load USER: " << g_basePath);
	load_db();
	APPL_INFO("new USER: [STOP]");
	return true;
}

ETK_EXPORT_API bool SERVICE_IO_uninit() {
	ethread::UniqueLock lock(g_mutex);
	store_db();
	APPL_INFO("delete USER [STOP]");
	return true;
}

ETK_EXPORT_API void SERVICE_IO_peridic_call() {
	if (g_needToStore == false) {
		return;
	}
	// try lock mutex:
	if (g_mutex.tryLock() == false) {
		return;
	}
	store_db();
	g_mutex.unLock();
}


ZEUS_SERVICE_TEST_SERVICE1_DECLARE(appl::TestService1);

