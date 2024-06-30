//======================================================================================================
// error_define.h
// ����Ŷ���ͷ�ļ�
// �汾��v1.0 ���Ӵ�����ע��
// lujun
//======================================================================================================

#ifndef HTSC_MD_GATEWAY_CLIENT_ERROR_DEFINE_H
#define HTSC_MD_GATEWAY_CLIENT_ERROR_DEFINE_H

namespace com {
namespace htsc {
namespace mdc {
namespace gateway {

//============================================================
//����Ŷ���
//============================================================
const int CONNECT_TO_SERVER_FAILED = -1000;                 //���ӷ�����ʧ�ܣ����粻ͨ
const int SERVER_REJECT_LOGIN = -1001;                      //�������ܾ���¼
const int INVALID_MESSAGE_HANDLE = -1002;                   //��Ч����Ϣ���������Ҫ�ڵ�¼ǰ����
const int STREAM_SEND_ERROR = -1003;                        //��·���ϣ����ͱ���ʧ��
const int STREAM_RECV_ERROR = -1004;                        //��·���ϣ����ձ���ʧ��
const int LOGIN_WAIT_TIMEOUT = -1005;                       //��¼��ʱ
const int INVALID_LOGIN_RESPONSE_MESSAGE = -1006;           //��Ч�ĵ�¼Ӧ����Ϣ���ڲ�����
const int LOGIN_ALL_SERVERS_FAILED = -1007;                 //���з�������¼ʧ��
const int SERVICE_DISCOVERY_RESPONSE_INVALID = -1008;       //��Ч�ķ�����Ӧ����Ϣ
const int SERVICE_DISCOVERY_RESPONSE_FAILURE = -1009;       //������Ӧ����Ϊ��ʧ��
const int VALID_SERVER_NOT_EXIST = -1010;                   //�����ڿɵ�¼�ķ�����
const int STREAM_INVALID = -1011;                           //������Ч����������
const int SEND_WAIT_TIMEOUT_ERROR = -1012;                  //�����ͳ�ʱ
const int RECEIVE_WAIT_TIMEOUT_ERROR = -1013;               //���ճ�ʱ

const int INVALID_INPUT_PORT = -1020;                       //��Ч������˿ںţ�������ʱ����
const int INVALID_INPUT_USER = -1021;                       //��Ч�������û�������������ʱ����
const int INVALID_INPUT_IP = -1022;                         //��Ч��ip����������ʱ����
const int INVALID_CLIENT = -1023;                           //��Ч�Ŀͻ���

const int START_MAINTAIN_THREAD_FAILED = -2100;             //�������Ӷ���ά�����ʧ��
const int START_HEARTBEAT_THREAD_FAILED = -2101;            //���������߳�ʧ��
const int START_MESSAGE_THREAD_FAILED = -2102;              //������Ϣ�����߳�ʧ��
const int START_HANDLE_THREAD_FAILED = -2103;               //����handle�����߳�ʧ��
const int STOP_LOGIN_FOR_QUIT = -2104;                      //�ͻ����˳���ֹͣ��¼
const int ACQUIRE_QUIT_MUTEX_FAILED = -2105;                //������ʧ�ܣ��ڲ�����
const int GET_MESSAGE_FROM_QUEUE_TIMEOUT = -2106;           //��ȡ��Ϣ��ʱ
const int SUBSCRIBE_RESPONSE_REJECT = -2107;                //����Ӧ����Ϊ�ܾ�
const int SUBSCRIBE_RESPONSE_HEADER_ID_NOT_EQUAL = -2108;   //����Ӧ����Ϣͷ��һ�£���λ�Ķ���Ӧ�𣬶���
const int INVALID_EMDC_MESSAGE_TYPE = -2109;                //��Ч��EMDC��Ϣ����
const int INVALID_PACKAGE_SIZE = -2110;                     //��Ч�İ�����
const int NULL_MESSAGE_POINTER = -2111;                     //��Ϣָ��ΪNULL���ڲ��쳣
const int INVALID_INSIGHT_MESSAGE_HEADER = -2112;           //��Ч����Ϣͷ
const int INVALID_INSIGHT_MESSAGE_BODY = -2113;             //��Ч����Ϣ�� 
const int SERIALIZE_MESSAGE_HEADER_TO_ARRAY_FAILED = -2114; //���л���Ϣͷʧ��
const int SERIALIZE_MESSAGE_BODY_TO_ARRAY_FAILED = -2115;   //���л���Ϣ��ʧ��
const int PARSE_MESSAGE_HEADER_FROM_ARRAY_FAILED = -2116;   //�������н�����Ϣͷʧ��
const int PARSE_MESSAGE_BODY_FROM_ARRAY_FAILED = -2117;     //�������н�����Ϣ��ʧ��
const int INVALID_INSIGHT_MESSAGE_BUF = -2118;              //��Ч����Ϣ����
const int OUT_OF_MEMORY = -2120;                            //�ڴ治��
const int INVALID_SUBSCRIBE_INPUT = -2121;                  //��Ч�Ķ�������
const int SUBSCRIBE_FAILED = -2122;                         //����ʧ��
const int CLIENT_IS_ALREADY_LOGIN = -2300;                  //�ѵ�¼�����ظ���¼
const int WAIT_SUBSCRIBE_RESPONSE_TIMEOUT = -2301;          //�ȴ�����Ӧ��ʱ
const int INVALID_MDQUERY_RESPONSE_MESSAGE = -2302;         //��Ч�Ĳ�ѯӦ����Ϣ
const int WAIT_PLAYBACK_RESPONSE_TIMEOUT = -2303;           //�ȴ��ط�Ӧ��ʱ
const int WAIT_MDQUERY_RESPONSE_TIMEOUT = -2304;            //�ȴ���ѯӦ��ʧ��
const int PLAYBACK_RESPONSE_REJECT = -2305;                 //�ط�Ӧ����Ϊ���ܾ�
const int MDQUERY_RESPONSE_ERROR = -2306;                   //��ѯӦ�����
const int MDQUERY_RESPONSE_FAILURE = -2307;                 //��ѯ���Ϊʧ��
const int ACQUIRE_SUBSCRIBE_MUTEX_FAILED = -2308;           //��������ʧ�ܣ��ڲ�����
const int INVALID_SUBSCRIBE_MESSAGE_BODY = -2309;           //������Ϣ����Ч

const int INIT_SSL_CONTEXT_FAILED = -2400;                  //��ʼ��SSL��������
const int SSL_VERIFY_PRIVATE_KEY_FAILED = -2401;            //SSL��֤keyʧ��
const int PLAYBACK_CONTROL_RESPONSE_REJECT = -2402;         //�طſ���Ӧ����Ϊ���ܾ�
const int QUERY_RESPONSE_CITATION_REJECT = -2403;           //ѡ��ص����������ѯ�ظ���ȴʹ�����÷��ػظ�
const int QUERY_RESPONSE_CALLBACK_REJECT = -2404;           //δѡ��ص����������ѯ�ظ���ȴδʹ��Ӧ�÷��ػظ�

const int UNKNOWN_PROPERTY_NAME = -2501;                    //δ֪�Ĳ�����

} //gateway
} //mdc
} //htsc
} //com
#endif //HTSC_MD_GATEWAY_CLIENT_ERROR_DEFINE_H