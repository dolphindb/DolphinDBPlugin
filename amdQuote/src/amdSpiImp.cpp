#include "amdSpiImp.h"
#include "ama_datatype.h"
#include "ama_tools.h"
#include "CoreConcept.h"
#include "Logger.h"

using amd::ama::LogLevel;

void AMDSpiImp::OnLog(const int32_t& level, const char* log, uint32_t len) {
    switch (level) {
        case LogLevel::kTrace:
        case LogLevel::kDebug:
            PLUGIN_LOG(AMDQUOTE_PREFIX, "AMA log: ", log);
            break;
        case LogLevel::kInfo:
            PLUGIN_LOG_INFO(AMDQUOTE_PREFIX, "AMA log: ", log);
            break;
        case LogLevel::kWarn:
            PLUGIN_LOG_WARN(AMDQUOTE_PREFIX, "AMA log: ", log);
            break;
        case LogLevel::kError:
        case LogLevel::kFatal:
            PLUGIN_LOG_ERR(AMDQUOTE_PREFIX, "AMA log: ", log);
            break;
        default:
            PLUGIN_LOG_INFO(AMDQUOTE_PREFIX, "AMA log: ", log);
    }
}

void AMDSpiImp::OnIndicator(const char* indicator, uint32_t len) {
    PLUGIN_LOG(AMDQUOTE_PREFIX, "AMA indicator: ", indicator);
}

void AMDSpiImp::OnEvent(uint32_t level, uint32_t code, const char* event_msg, uint32_t len) {
    static LocklessHashmap<string, int> errorMap;
    string codeString = amd::ama::Tools::GetEventCodeString(code);
    // LockGuard<Mutex> amdLock_(&AmdQuote::amdMutex_);
    if(codeString.find("Failed") != string::npos){
        if(ERROR_LOG) {
            PLUGIN_LOG_ERR(AMDQUOTE_PREFIX + "AMA event: " + codeString);
            PLUGIN_LOG_INFO(AMDQUOTE_PREFIX + "AMA event: ", std::string(event_msg));
        } else {
            int val = 1;
            if(errorMap.find(event_msg, val) == 0){
                errorMap.insert(event_msg, val);
                PLUGIN_LOG_ERR(AMDQUOTE_PREFIX + "AMA event: " + codeString);
                PLUGIN_LOG_INFO(AMDQUOTE_PREFIX + "AMA event: ", std::string(event_msg));
            }
        }
    }else{
        codeString.clear();
        PLUGIN_LOG_INFO(AMDQUOTE_PREFIX + "AMA event: ", std::string(event_msg));
    }
}