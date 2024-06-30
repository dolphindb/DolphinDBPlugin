//======================================================================================================
// parameter_define.h
// ��������ͷ�ļ�
// �汾��v1.0
// lujun 
//======================================================================================================

#ifndef HTSC_MD_GATEWAY_CLIENT_PARAMETER_DEFINE_H
#define HTSC_MD_GATEWAY_CLIENT_PARAMETER_DEFINE_H

namespace com {
namespace htsc {
namespace mdc {
namespace gateway {

//=======================================================================================================
//  �������ݶ���
//=======================================================================================================
//�����������ļ�·��
const int HTSC_INSIGHT_PATH_MAX_LEN = 4096;
//����������
const int HTSC_INSIGHT_BUF_LEN = 2048;
//�����ļ���key��󳤶�
const int HTSC_INSIGHT_PROPERTY_KEY_MAX_LEN = 100;
//�����ļ���value��󳤶�
const int HTSC_INSIGHT_PROPERTY_VALUE_MAX_LEN = 1024;
//Ĭ�ϵ������ļ���
const char* const HTSC_INSIGHT_PROPERTIES_FILENAME = "htsc-insight-cpp-config.conf";
//�����ļ�Ŀ¼ϵͳ����������
const char* const HTSC_INSIGHT_ENV_CONFIG_FOLDER = "HTSC_INSIGHT_ENV_CONFIG_FOLDER";

#define HTSC_INSIGHT_COMMENT_LETTER '#'
#define HTSC_INSIGHT_ASSAN_OPERATOR '='

//=======================================================================================================
//  ����������
//=======================================================================================================
//���ӵȴ���ʱʱ�䣬��λ����
const char* const HTSC_INSIGHT_CONFIG_CONNECT_WAIT_TIME = "connect_wait_time";
//ÿ�η���(send)�ȴ���ʱʱ�䣬��λ:��
const char* const HTSC_INSIGHT_CONFIG_SEND_WAIT_TIMEOUT = "send_wait_timeout";
//ÿ�ν���(reveive)�ȴ���ʱʱ�䣬��λ����
const char* const HTSC_INSIGHT_CONFIG_RECV_WAIT_TIMEOUT = "recv_wait_timeout";
//���ν���(reveive)�����������ȴ�ʱ�䣬��λ����
const char* const HTSC_INSIGHT_CONFIG_RECV_N_TIMEOUT = "recv_n_timeout";
//��������·�ȴ���ʱʱ�䣬��λ����
const char* const HTSC_INSIGHT_CONFIG_DISCOVERY_SERVER_WAIT_TIMEOUT = "discovery_server_wait_timeout";
//�����ȴ�ʱ�䣬��λ:��
const char* const HTSC_INSIGHT_CONFIG_HEARTBEAT_SLEEP_TIME = "heartbeat_sleep_time";
//���������ʧЧʱ�䣬��λ:��
const char* const HTSC_INSIGHT_CONFIG_SERVER_HEARTBEAT_TIMEOUT = "server_heartbeat_timeout";
//���ĵȴ�Ӧ��ʱ�䣬��λ����
const char* const HTSC_INSIGHT_CONFIG_SUBSCRIBE_RESPONSE_MESSAGE_WAIT_TIME = "subscribe_response_message_wait_time";
//�������Դ���
const char* const HTSC_INSIGHT_CONFIG_SUBSCRIBE_TRY_COUNT = "subscribe_try_count";
//MDQuery����ȴ�Ӧ��ʱ�䣬��λ����
const char* const HTSC_INSIGHT_CONFIG_MDQUERY_RESPONSE_MESSAGE_WAIT_TIME = "mdquery_response_message_wait_time";
//Playback����ȴ�Ӧ��ʱ�䣬��λ����
const char* const HTSC_INSIGHT_CONFIG_PLAYBACK_RESPONSE_MESSAGE_WAIT_TIME = "playback_response_message_wait_time";
//�����̳߳��̳߳�ʼ�߳���
const char* const HTSC_INSIGHT_CONFIG_THREAD_POOL_INIT_SIZE = "thread_pool_init_size";
//�����̳߳�����߳���
const char* const HTSC_INSIGHT_CONFIG_THREAD_POOL_MAX_SIZE = "thread_pool_max_size";
//ÿ�λ�ȡ�������Ϣ��
const char* const HTSC_INSIGHT_CONFIG_MAX_FETCH_MESSAGE_COUNT = "max_fetch_message_count";
//�յ��ط�״̬Ϊ�����ı��ĵȴ�payload�������ʱ�䣬��λ����
const char* const HTSC_INSIGHT_CONFIG_PLAYBACK_END_STATUS_WAIT_PAYLOAD_TIME = "playback_end_status_wait_payload_time";
//�߳���sleepʱ�䳤�ȣ���λ��΢��
const char* const HTSC_INSIGHT_CONFIG_THREAD_SLEEP_TIME = "thread_sleep_time";
//������־��������������
const char* const HTSC_INSIGHT_CONFIG_TRAFFIC_CHECK_GAP = "traffic_check_gap";
//����������������ʱ�䣬��λ������
const char* const HTSC_INSIGHT_CONFIG_NODATA_CHECK_GAP = "nodata_check_gap";
//������־����
const char* const HTSC_INSIGHT_CONFIG_HEARTBEAT_TRACE = "heartbeat_trace";
//������־����
const char* const HTSC_INSIGHT_CONFIG_GLOBAL_TRACE = "global_trace";
//��ѹ���߳̿���
const char* const HTSC_INSIGHT_CONFIG_COMPRESS_SWITCH = "compress_switch";
//�ظ����ƿ��أ���ʱ�ظ�ͨ���ص����أ������ߴ���ظ���Ϣ
const char* const HTSC_INSIGHT_CONFIG_RESPONSE_CALLBACK = "response_callback";
//��¼�������س��Դ���
const char* const HTSC_INSIGHT_CONFIG_LOGIN_TRY_COUNT = "login_try_count";
//��¼���������س��Դ���
const char* const HTSC_INSIGHT_CONFIG_LOGIN_DISCOVERY_TRY_COUNT = "login_discovery_try_count";
//OnMarketdata�ص����������ݽ�ֹ����
const char* const HTSC_INSIGHT_CONFIG_FORBID_DESTRUCT = "forbid_destruct";
//��������ʱ����ֽ��ճ�ʱ������
const char* const HTSC_INSIGHT_CONFIG_ETIME_RECONNECT = "etime_reconnect";

//=======================================================================================================
//  Ĭ�ϲ�������
//=======================================================================================================
//���ӵȴ���ʱʱ�䣬��λ����
const int CONNECT_WAIT_TIME = 30;
//ÿ�η��͵ȴ���ʱʱ�䣬��λ:��
const int SEND_WAIT_TIMEOUT = 30;
//ÿ�ν��յȴ���ʱʱ�䣬��λ����
const int RECV_WAIT_TIMEOUT = 30;
//���ν���(reveive)�����������ȴ�ʱ�䣬��λ����
const int RECV_N_TIMEOUT = 3;
//��������·�ȴ���ʱʱ�䣬��λ����
const int DISCOVERY_SERVER_WAIT_TIMEOUT = 30;
//�����ȴ�ʱ�䣬��λ:��
const int HEARTBEAT_SLEEP_TIME = 3;
//���������ʧЧʱ�䣬��λ:��
const int SERVER_HEARTBEAT_TIMEOUT = 60;
//���ĵȴ�Ӧ��ʱ�䣬��λ����
const int SUBSCRIBE_RESPONSE_MESSAGE_WAIT_TIME = 30;
//�������Դ���
const int SUBSCRIBE_TRY_COUNT = 10;
//MDQuery����ȴ�Ӧ��ʱ�䣬��λ����
const int MDQUERY_RESPONSE_MESSAGE_WAIT_TIME = 30;
//Playback����ȴ�Ӧ��ʱ�䣬��λ����
const int PLAYBACK_RESPONSE_MESSAGE_WAIT_TIME = 30;
//�����̳߳��̳߳�ʼ�߳���
const int THREAD_POOL_INIT_SIZE = 5;
//�����̳߳�����߳���
const int THREAD_POOL_MAX_SIZE = 100;
//ÿ�λ�ȡ�������Ϣ��
const int MAX_FETCH_MESSAGE_COUNT = 10;
//�յ��ط�״̬Ϊ�����ı��ĵȴ�payload�������ʱ�䣬��λ����
const int PLAYBACK_END_STATUS_WAIT_PAYLOAD_TIME = 3;
//�߳���sleepʱ�䳤�ȣ���λ��΢��
const int THREAD_SLEEP_TIME = 100;//0.1ms
//������־��������������
const int TRAFFIC_CHECK_GAP = 10000;
//����������������ʱ�䣬��λ������
const int NODATA_CHECK_GAP = 10000;//10s
//������־����
const int HEARTBEAT_TRACE = 1;
//������־����
const int GLOBAL_TRACE = 1;
//��ѹ���߳̿���
const int COMPRESS_SWITCH = 1;
//�ظ����ƿ��أ���ʱ�ظ�ͨ���ص����أ������ߴ���ظ���Ϣ
const int RESPONSE_CALLBACK = 0;
//��¼�������س��Դ���
const int LOGIN_TRY_COUNT = 5;
//��¼���������س��Դ���
const int LOGIN_DISCOVERY_TRY_COUNT = 5;
//OnMarketdata�ص����������ݽ�ֹ����
const int FORBID_DESTRUCT = 0;
//��������ʱ����ֽ��ճ�ʱ������
const int ETIME_RECONNECT = 0;

} //gateway
} //mdc
} //htsc
} //com

#endif //HTSC_MD_GATEWAY_CLIENT_PARAMETER_DEFINE_H