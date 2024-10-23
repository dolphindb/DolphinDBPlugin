#include <Exceptions.h>
#include <ScalarImp.h>
#include <openssl/ssl.h>
#include <Util.h>
#include "email.h"
#include <curl/curl.h>
#include<list>
#include<vector>
#include<string>
#include<unordered_map>
using namespace std;

class SMTP_STATUS{
public:
    static unordered_map<string, string> smtpHost;
    static unordered_map<string, int> smtpPost;
    static Mutex mutex;
};


unordered_map<string, string> SMTP_STATUS::smtpHost = {
{"126.com", "smtp.126.com"},
{"163.com","smtp.163.com"},
{"yeah.net","smtp.yeah.net"},
{"sohu.com","smtp.sohu.com"},
{"dolphindb.com", "smtp.office365.com"},
{"qq.com", "smtp.qq.com"}};
unordered_map<string, int> SMTP_STATUS::smtpPost = {{"dolphindb.com", 587}};

Mutex SMTP_STATUS::mutex;


size_t curlWriteData(void *ptr, size_t size, size_t nmemb, string *data) {
    data->append((char *) ptr, size * nmemb);
    return size * nmemb;
}

CSendMail::CSendMail(
        const string &strUser,
        const string &strPsw
) {
    m_strUser_ = strUser;
    m_strPsw_ = strPsw;
    m_RecipientList_.clear();
    m_MailContent_.clear();
    m_iMailContentPos_ = 0;
    m_offset_ = 0;
}

size_t CSendMail::read_callback(void *ptr, size_t size, size_t nmemb, void *userp) {
    CSendMail *pSm = (CSendMail *) userp;
    size_t maxSize = size * nmemb;
    if (maxSize < 1)
        return 0;
    if (pSm->m_iMailContentPos_ < (int) pSm->m_MailContent_.size()) {
        size_t remainLen = pSm->m_MailContent_[pSm->m_iMailContentPos_].length() - pSm->m_offset_;
        size_t writeLen = std::min(remainLen, maxSize);
        memcpy(ptr, pSm->m_MailContent_[pSm->m_iMailContentPos_].data() + pSm->m_offset_, writeLen);
        if(writeLen == remainLen){
            pSm->m_iMailContentPos_++;
            pSm->m_offset_ = 0;
        }
        else{
            pSm->m_offset_ += writeLen;
        }
        return writeLen;
    }
    return 0;
}

string CSendMail::ConstructMsg(const string &strSubject, const string &strContent) {
    string msg;
    msg += "MIME-Version: 1.0\r\n";
    msg += "To: ";
    for (list<string>::iterator it = m_RecipientList_.begin(); it != m_RecipientList_.end();) {
        msg += *it;
        it++;
        if (it != m_RecipientList_.end())
            msg += ",";
    }
    msg += "\r\n";
    msg += "From: ";
    msg += m_strUser_;
    msg += "\r\n";
    msg += "Subject: ";
    msg += strSubject;
    msg += "\r\n";
    msg += "Content-Type: text/html;Charset=\"UTF-8\"\r\n\r\n";
    msg += strContent;
    return msg;
}

ConstantSP CSendMail::SendMail(const string &content) {
    ConstantSP res = Util::createDictionary(DT_STRING, nullptr, DT_ANY, nullptr);
    char errorBuf[CURL_ERROR_SIZE];
    m_MailContent_.clear();
    m_iMailContentPos_ = 0;
    m_offset_ = 0;
    m_MailContent_.push_back(content);
    CURL *curl;
    struct curl_slist *rcpt_list = NULL;
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (!curl) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        throw IllegalArgumentException(__FUNCTION__, "Init curl failed");
    }
    string strListMailTo;
    list<string>::iterator lastIter = --m_RecipientList_.end();
    for (list<string>::iterator it = m_RecipientList_.begin(); it != m_RecipientList_.end(); it++) {
        strListMailTo += it->c_str();
        if(lastIter != it)
            strListMailTo += ' ';
    }
    for (list<string>::iterator it = m_RecipientList_.begin(); it != m_RecipientList_.end(); it++) {
        rcpt_list = curl_slist_append(rcpt_list, it->c_str());
    }
    string strSmtpServer;
    int port = 25;
    int pos = m_strUser_.find_first_of('@');
    string tmp = m_strUser_.substr(pos + 1);
    {
        LockGuard<Mutex> lockGuard(&SMTP_STATUS::mutex);
        if (pos > 1 && SMTP_STATUS::smtpHost.count(tmp) != 0)
            strSmtpServer = SMTP_STATUS::smtpHost[tmp];
        else {
            curl_slist_free_all(rcpt_list);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            throw IllegalArgumentException(__FUNCTION__,
                                        "Enter @126.com,@163.com,@yeah.net,@sohu.com,@dolphindb.com,@qq.com or config the email smtpServer");
        }
        if (SMTP_STATUS::smtpPost.count(tmp) != 0)
            port = SMTP_STATUS::smtpPost[tmp];
    }

    string strUrl = "smtp://" + strSmtpServer;
    strUrl += ":";
    strUrl += to_string(port);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuf);
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, m_strUser_.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, m_strPsw_.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, &read_callback);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, m_strUser_.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, rcpt_list);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
    curl_easy_setopt(curl, CURLOPT_READDATA, this);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_SESSIONID_CACHE, 0L);
    string responseString;
    string headerString;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerString);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    CURLcode ret = curl_easy_perform(curl);

    long responseCode;
    double elapsed;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);

    if (ret != 0) {
        string errorMsg(errorBuf);
        curl_slist_free_all(rcpt_list);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        if (responseCode == 535)
            throw RuntimeException("UserId or password is incorrect");
        else
            throw RuntimeException("Failed to send the email with code :" + std::to_string(ret) + ", msg :" + errorMsg + ". ");
    }

    res->set(new String("responseCode"), new Int(responseCode));
    res->set(new String("elapsed"), new Double(elapsed));
    res->set(new String("headers"), new String(headerString));
    res->set(new String("text"), new String(responseString));
    res->set(new String("userId"), new String(m_strUser_));
    res->set(new String("recipient"), new String(strListMailTo));
    curl_slist_free_all(rcpt_list);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return res;
}

void CSendMail::checkSMTPMsg(const string& msg, const string& userId, const vector<string>& recipient) {
    vector<string> lines = Util::split(Util::replace(msg, "\r\n", "\n"), '\n');
    bool setUserId = false, setRecipient = false;
    for(string& line : lines){
        if(line.empty()){
            break;
        }
        vector<string> words = Util::split(line, ':');
        for(string& word : words){
            word = Util::trim(word);
        }
        if(words.size() < 2) continue;
        if (words[0] == "From") {
            if(words[1] != userId){
                throw IllegalArgumentException(__FUNCTION__, PLUGIN_HTTPCLIENT_PREFIX + "Sender email specified in the 'From:' field of msg must match userId. ");
            }
            setUserId = true;
        }else if(words[0] == "To"){
            vector<string> toList = Util::split(words[1], ',');
            for(string& to : toList){
                to = Util::trim(to);
            }
            if(toList.size() != recipient.size()){
                throw IllegalArgumentException(__FUNCTION__, PLUGIN_HTTPCLIENT_PREFIX + "Receiver email specified in the 'To:' field of msg must match recipient. ");
            }
            for(size_t i = 0; i < toList.size(); ++i){
                if(toList[i] != recipient[i]){
                    throw IllegalArgumentException(__FUNCTION__, PLUGIN_HTTPCLIENT_PREFIX + "Receiver email specified in the 'To:' field of msg must match recipient. ");
                }
            }
            setRecipient = true;
        }
    }
    if(!setUserId){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HTTPCLIENT_PREFIX + "The 'From:' field of msg is required. ");
    }
    if(!setRecipient){
        throw IllegalArgumentException(__FUNCTION__, PLUGIN_HTTPCLIENT_PREFIX + "The 'To:' field of msg is required. ");
    }
}

ConstantSP sendEmail(Heap *heap, vector<ConstantSP> &args) {
    string syntax = "Usage: httpClient::sendEmail(userId, pwd, recipient, [subject], [body], [msg]). ";
    string userId, pwd;
    ConstantSP recipient;
    std::string subject, body, msg;
    bool setSubject = false, setBody = false, setMsg = false;
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "userId must be a string. ");
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR)
        throw IllegalArgumentException(__FUNCTION__, syntax + "pwd must be a string. ");
    recipient = args[2];
    if (recipient->getType() != DT_STRING || (recipient->getForm() != DF_SCALAR && recipient->getForm() != DF_VECTOR))
        throw IllegalArgumentException(__FUNCTION__, syntax + "recipient must be a string or a string vector. ");

    if(!args[3]->isNothing()){
        if (args[3]->getType() != DT_STRING || args[3]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, syntax + "subject must be a string. ");
        setSubject = true;
        subject = args[3]->getString();
    }
    if(!args[4]->isNothing()){
        if (args[4]->getType() != DT_STRING || args[4]->getForm() != DF_SCALAR){
            throw IllegalArgumentException(__FUNCTION__, syntax + "body must be a string. ");
        }
        setBody = true;
        body = args[4]->getString();
    }
    if(args.size() >= 6 && !args[5]->isNothing()){
        if (args[5]->getType() != DT_STRING || args[5]->getForm() != DF_SCALAR)
            throw IllegalArgumentException(__FUNCTION__, syntax + "msg must be a string. ");
        setMsg = true;
        msg = args[5]->getString();
    }
    
    userId = args[0]->getString();
    if(userId == "")
        throw IllegalArgumentException(__FUNCTION__, syntax + "userId must be a non-empty string. ");
    pwd = args[1]->getString();
    CSendMail mail(userId, pwd);
    vector<string> recipientList;
    if (recipient->getForm() == DF_VECTOR) {
        VectorSP veMailTo = ((VectorSP) recipient);
        for (int i = 0; i < veMailTo->size(); ++i) {
            mail.AddRecipient(veMailTo->get(i)->getString());
            recipientList.push_back(veMailTo->get(i)->getString());
        }
    } else{
        mail.AddRecipient(recipient->getString());
        recipientList.push_back(recipient->getString());
    }
    if(setSubject ^ setBody){
        throw IllegalArgumentException(__FUNCTION__, syntax + "Subject and body must be set together. ");
    }
    if(!setSubject && !setMsg){
        throw IllegalArgumentException(__FUNCTION__, syntax + "Set either subject or msg. At least one is required. ");
    }
    if(setSubject && setMsg){
        throw IllegalArgumentException(__FUNCTION__, syntax + "Set either subject or msg, not both. ");
    }
    if(setBody){
        msg = mail.ConstructMsg(subject, body);
    }else{
        mail.checkSMTPMsg(msg, userId, recipientList);
    }
    ConstantSP res = mail.SendMail(msg);
    return res;
}

ConstantSP emailSmtpConfig(Heap *heap, vector<ConstantSP> &args) {
    if (args[0]->getType() != DT_STRING || args[0]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "EmailName must be a string scalar");
    }
    if (args[1]->getType() != DT_STRING || args[1]->getForm() != DF_SCALAR) {
        throw IllegalArgumentException(__FUNCTION__, "Host must be a string scalar");
    }
    if (args.size() == 3) {
        if (args[2]->getType() != DT_INT || args[2]->getForm() != DF_SCALAR) {
            throw IllegalArgumentException(__FUNCTION__, "Post must be a int scalar");
        }
    }
    if (args[0]->getString() == "")
        throw IllegalArgumentException(__FUNCTION__, "EmailName can't be empty");
    if (args[1]->getString() == "")
        throw IllegalArgumentException(__FUNCTION__, "Host can't be empty");
    
    LockGuard<Mutex> lockGuard(&SMTP_STATUS::mutex);
    SMTP_STATUS::smtpHost[args[0]->getString()] = args[1]->getString();
    if (args.size() == 3)
        SMTP_STATUS::smtpPost[args[0]->getString()] = args[2]->getInt();
    return new Bool(true);
}
