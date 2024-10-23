#include <CoreConcept.h>
#include <string>
#include <list>
#include <vector>
#include <curl/curl.h>

extern "C" ConstantSP sendEmail(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP emailSmtpConfig(Heap *heap, vector<ConstantSP> &args);
extern "C" ConstantSP sendSMTPEmail(Heap *heap, vector<ConstantSP> &args);

static const string PLUGIN_HTTPCLIENT_PREFIX = "[PLUGIN:HTTPCLIENT]: ";

class CSendMail {
public:
    CSendMail(
            const std::string &strUser,
            const std::string &strPsw
    );

private:
    std::string m_strUser_;
    std::string m_strPsw_;
    std::list<std::string> m_RecipientList_;
    std::string m_strMailFrom_;
    std::vector<std::string> m_MailContent_;
    int m_iMailContentPos_;
    size_t m_offset_;

private:
    static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp);

public:
    ConstantSP SendMail(const string &content);
    void AddRecipient(const std::string &strMailTo) { m_RecipientList_.push_back(strMailTo); }
    string ConstructMsg(const string &strSubject, const string &strContent);
    void checkSMTPMsg(const string& msg, const string& userId, const vector<string>& recipient);
};
