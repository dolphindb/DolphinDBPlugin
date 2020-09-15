#include <CoreConcept.h>
#include <string>
#include <list>
#include <vector>
#include <curl/curl.h>
extern "C" ConstantSP sendEmail(Heap *heap, vector<ConstantSP> &args);

class CSendMail {
public:
    CSendMail( 
                const std::string & strUser,
                const std::string & strPsw
            ); 
private:
    std::string m_strUser;   
    std::string m_strPsw; 
    std::list<std::string> m_RecipientList;   
    std::string m_strMailFrom;   
    std::vector<std::string> m_MailContent; 
    int m_iMailContentPos;  
   
private:
    static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp);
    bool ConstructHead(const std::string & strSubject, const std::string & strContent);
public:
    ConstantSP SendMail(const std::string & strSubject, const std::string & strMailBody);
    void AddRecipient(const std::string & strMailTo) { m_RecipientList.push_back(strMailTo); }
};
