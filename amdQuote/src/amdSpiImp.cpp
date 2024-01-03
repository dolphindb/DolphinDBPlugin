
#include "amdQuote.h"
#include "amdSpiImp.h"
#include "CoreConcept.h"

inline string AMDSpiImp::transMarket(int type) {
    if(type == 101) {
        return "XSHG";
    } else if(type == 102) {
        return "XSHE";
    }
    return "";
}

void AMDSpiImp::OnEvent(uint32_t level, uint32_t code, const char* event_msg, uint32_t len) {
    static LocklessHashmap<string, int> errorMap;
    string codeString = amd::ama::Tools::GetEventCodeString(code);
    // LockGuard<Mutex> amdLock_(&AmdQuote::amdMutex_);
    if(codeString.find("Failed") != string::npos){
        if(ERROR_LOG) {
            LOG_ERR(AMDQUOTE_PREFIX + "AMA event: " + codeString);
            LOG_INFO(AMDQUOTE_PREFIX + "AMA event: ", std::string(event_msg));
        } else {
            int val = 1;
            if(errorMap.find(event_msg, val) == 0){
                errorMap.insert(event_msg, val);
                LOG_ERR(AMDQUOTE_PREFIX + "AMA event: " + codeString);
                LOG_INFO(AMDQUOTE_PREFIX + "AMA event: ", std::string(event_msg));
            }
        }
    }else{
        codeString.clear();
        LOG_INFO(AMDQUOTE_PREFIX + "AMA event: ", std::string(event_msg));
    }
}