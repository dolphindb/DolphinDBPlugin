//======================================================================================================
// base_define.h
// �������塢��־�����ȳ��ú�������ͷ�ļ�
// �汾��v1.0  
// lujun
//======================================================================================================

#ifndef HTSC_MD_GATEWAY_CLIENT_BASE_DEFINE_H
#define HTSC_MD_GATEWAY_CLIENT_BASE_DEFINE_H

#include <string>
#include <stdint.h>

#ifdef USE_STATIC_LIB
#define LIB_EXPORT
#elif defined(WIN32) || defined(WIN64) || defined(_WINDOWS)
#ifdef MD_GATEWAY_CLIENT_EXPORT_FLAG
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT __declspec(dllimport)
#endif
#else
#define LIB_EXPORT
#endif

#define NAMESPACE_BEGIN \
namespace com { \
namespace htsc {  \
namespace mdc { \
namespace gateway {

#define NAMESPACE_END \
} \
} \
} \
} 

//���ÿ������캯���͸�ֵ����
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);                \
    TypeName& operator=(const TypeName&)

//���嵥�����󼰶��������ͷź�
#define DELETE_P(buf) if (buf) { delete buf; buf = NULL;}
#define DELETE_P_ARRAY(buf) if (buf) { delete[] buf; buf = NULL;}

NAMESPACE_BEGIN
//messageClassification��Ϣ���� 
const int MESSAGE_CLASSIFICATION_SYSTEM = 1001;	   //ϵͳ��Ϣ����	������Ϣ����¼��Ϣ��ͨ�þ�����Ϣ
const int MESSAGE_CLASSIFICATION_REALTIME = 2001;	   //ʵʱ������Ϣ����	ʵʱ������ص���Ϣ
const int MESSAGE_CLASSIFICATION_PLAYBACK = 2002;	   //�ط�������Ϣ����	�ط���ص���Ϣ
const int MESSAGE_CLASSIFICATION_QUERY = 2003;	   //��ѯ��Ϣ����	��ѯ�����Ϣ

//queryType˵��
const int QUERY_TYPE_BASE_INFORMATION = 1011;        //��ѯ֤ȯ������Ϣ	����������Ϊֹ�������Ķ�Ӧ���͵����Ի�����Ϣ������Ҫ�ر��QueryParam
const int QUERY_TYPE_LATEST_BASE_INFORMATION = 1012; //��ѯ����֤ȯ������Ϣ	�����������е�֤ȯ������Ϣ������Ҫ�ر��QueryParam
const int QUERY_TYPE_ETF_BASE_INFORMATION = 1013;    //��ѯETF�Ļ�����Ϣ
const int QUERY_TYPE_STATUS = 1021;	                 //��ѯ֤ȯ����״̬	����Ҫ�ر��QueryParam
const int QUERY_TYPE_SUBSCRIBE_ANSWER = 1031;        //���ĵķ���ֵ

//appType˵��
const int APP_TYPE_INSIGHT = 101;	//insight�����û�
const int APP_TYPE_XTRADER = 201;	//XTraderϵͳ

/**
* ������ʼ�����رպ���
* ����MFC�û�(��main�������)����Ҫ�������ʼ�ͽ�β������InitEnv��FiniEnv�����ڳ�ʼ��ACE����
*/
LIB_EXPORT void init_env();
LIB_EXPORT void fini_env();

/**
* �򿪹ر�g_trace��־
*/
LIB_EXPORT void open_trace();
LIB_EXPORT void close_trace();
LIB_EXPORT bool is_trace();

/**
* �򿪹ر�g_heartbeat_trace��־
*/
LIB_EXPORT void open_heartbeat_trace();
LIB_EXPORT void close_heartbeat_trace();
LIB_EXPORT bool is_heartbeat_trace();

/**
* �����Ƿ�Ϊѹ������ѡ���߳�
*/
LIB_EXPORT void open_compress();
LIB_EXPORT void close_compress();
LIB_EXPORT bool is_compress();

/**
* �򿪹ر�g_response_callback����
* ��g_response_callback���غ���Ҫ���д���response��Ϣ
*/
LIB_EXPORT void open_response_callback();
LIB_EXPORT void close_response_callback();
LIB_EXPORT bool is_response_callback();

/**
* �������ص�ַ�Զ�ѡ�񿪹أ�open:���ݷ��������ѡ��close:���ݿͻ�������ѡ��
*/
LIB_EXPORT void open_node_auto();
LIB_EXPORT void close_node_auto();
LIB_EXPORT bool is_node_auto();

/**
* �ж��Ƿ��ֹ����
*/
LIB_EXPORT bool is_forbid_destruct();


/**
* �ж��Ƿ���ֽ��ճ�ʱ������
*/
LIB_EXPORT bool is_etime_reconnect();

LIB_EXPORT void open_bind_port(uint32_t start_port, uint32_t end_port);
LIB_EXPORT void close_bind_port();
LIB_EXPORT bool is_bind_port();
LIB_EXPORT uint32_t get_start_port();
LIB_EXPORT uint32_t get_end_port();

/**
* ��־������
*/
typedef enum mdc_log_severity {
	MDC_LOG_SEVERITY_DEBUG,
	MDC_LOG_SEVERITY_WARNING,
	MDC_LOG_SEVERITY_ERROR
} mdc_log_severity;

//�򿪡��ر���־���ļ�����־�ļ��������������Ŀ¼����ʽΪ��pid_����id.log
LIB_EXPORT void open_file_log();
LIB_EXPORT void close_file_log();

//�򿪡��ر���־����׼���
LIB_EXPORT void open_cout_log();
LIB_EXPORT void close_cout_log();

/**
* ��ӡ��־��Ĭ�ϴ�ӡ����־�ͱ�׼�����
* �����Ҫ���ƣ�ʹ������open/close�ӿ�
* ʹ�÷�ʽ����printf���ƣ���֧��%s��%d�ȳ�������
*/
LIB_EXPORT void debug_print(const char* format, ...);
LIB_EXPORT void warning_print(const char* format, ...);
LIB_EXPORT void error_print(const char* format, ...);

/**
* ��ȡ����ź���
* @param[in] code �����
* @return ������
*/
LIB_EXPORT std::string get_error_code_value(int code);

/**
* ��ȡ��̬��汾���15���ַ�
*/
LIB_EXPORT const char* get_dll_version();

/**
* ���ö�̬��汾
*/
LIB_EXPORT void set_dll_version(const char* version);

/**
* ��ȡtaskid������Ϊ���ڵ���+pid+��ǰʱ�䣨��ȷ��΢�룩+�Զ������ݺ�׺
* �磺hostname_13323_20170728T1500.0132_suffix
* @param[in] suffix ��׺
* @return ����id�ַ���
*/
LIB_EXPORT std::string get_task_id(const std::string& suffix = "");

/**
* ��utf8������ַ���ת��Ϊgb2312���ַ���
* @param[in] str utf8������ַ���
* @return gb2312���ַ���
*/
LIB_EXPORT std::string utf8_to_gb2312(const std::string& str);
/**
* ��gb2312�����bufת��Ϊutf8���ַ���
* @param[in] str gb2312������ַ���
* @return utf8���ַ���
*/
LIB_EXPORT std::string gb2312_to_utf8(const std::string& str);

//���Ի�ȡ�����ã��������init_env��ʼ��
LIB_EXPORT int get_int_property_value(const char* key);
LIB_EXPORT int set_int_property_value(const char* key, int value);

/**
* AES���ܣ����С����Ϊ128λ��16�ֽڣ�
* ������ǣ���Ҫ���룬��Կ���ȿ���ѡ��128λ��192λ��256λ��
* @param[in] key ��Կ
* @param[in] data ����
*/
LIB_EXPORT std::string encode(const std::string& key, const std::string& data);

/**
* AES���ܣ���Կ���ȿ���ѡ��128λ��192λ��256λ��
* @param[in] key ��Կ
* @param[in] encode_data ����
*/
LIB_EXPORT std::string decode(const std::string& key, const std::string& encode_data);

/**
* ɾ����ַӳ����е�ĳ����ַ
* @param[in] original_ip ��ɾ����ԭ��ַ
*/
LIB_EXPORT void delete_ip_map(std::string original_ip);

/**
* ������ַӳ����е�ӳ���߼�
* @param[in] original_ip ԭ��ַ
* @param[in] mapped_ip   ӳ���ַ
*/
LIB_EXPORT void add_ip_map(std::string original_ip, std::string mapped_ip);

/**
* ɾ���˿�ӳ����е�ĳ���˿�
* @param[in] original_port ��ɾ����ԭ�˿�
*/
LIB_EXPORT void delete_port_map(int original_port);

/**
* �����˿�ӳ����е�ӳ���߼�
* @param[in] original_port ԭ�˿�
* @param[in] mapped_port   ӳ��˿�
*/
LIB_EXPORT void add_port_map(int original_port, int mapped_port);

NAMESPACE_END

#endif //HTSC_MD_GATEWAY_CLIENT_BASE_DEFINE_H