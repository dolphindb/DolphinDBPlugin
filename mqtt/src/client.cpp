#include "client.h"

#include "ddbplugin/CommonInterface.h"
#ifdef __linux__
#include <poll.h>
#endif
#include "PluginLoggerImp.h"

namespace ddb {
namespace mqtt {

bool checkConnectStatus(int sockfd) {
#ifdef __linux__
    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLOUT;

    int result = poll(fds, 1, 4000);
    if (result > 0 && (fds[0].revents & POLLOUT)) {
        int error = 0;
        socklen_t errorLen = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &errorLen) == 0 && error == 0) {
            return true;
        }
    }
    return false;
#else
    WSAPOLLFD fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLOUT;

    int result = WSAPoll(fds, 1, 4000);
    if (result > 0 && (fds[0].revents & POLLOUT)) {
        int error = 0;
        int errorLen = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&error, &errorLen) == 0 && error == 0) {
            return true;
        }
    }
    return false;
#endif
}

void checkConnack(mqtt_client *client) {
    // handle connack message, wait it up to 5 seconds
    bool connackReply = false;
    for (int i = 0; i < 50; ++i) {
        Util::sleep(100);

        mqtt_sync(client);
        struct mqtt_response response;
        ssize_t n = mqtt_unpack_response(&response, client->recv_buffer.mem_start, client->recv_buffer.curr_sz);
        if (n > 0) {
            if (response.fixed_header.control_type == MQTT_CONTROL_CONNACK) {
                struct mqtt_response_connack *connack = &response.decoded.connack;
                if (connack->return_code != MQTT_CONNACK_ACCEPTED) {
                    string codeStr;
                    switch (connack->return_code) {
                        case MQTT_CONNACK_REFUSED_PROTOCOL_VERSION:
                            codeStr = "MQTT_CONNACK_REFUSED_PROTOCOL_VERSION";
                            break;
                        case MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED:
                            codeStr = "MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED";
                            break;
                        case MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE:
                            codeStr = "MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE";
                            break;
                        case MQTT_CONNACK_REFUSED_BAD_USER_NAME_OR_PASSWORD:
                            codeStr = "MQTT_CONNACK_REFUSED_BAD_USER_NAME_OR_PASSWORD";
                            break;
                        case MQTT_CONNACK_REFUSED_NOT_AUTHORIZED:
                            codeStr = "MQTT_CONNACK_REFUSED_NOT_AUTHORIZED";
                            break;
                        default:
                            codeStr = "NOT_KNOWN_RETURN_CODE";
                            break;
                    }
                    throw RuntimeException(LOG_PRE_STR + " the return code of connack message is " + codeStr + ".");
                }

                connackReply = true;
                break;
            } else {
                throw RuntimeException(LOG_PRE_STR + " not a connack message, this type is: " +
                                       std::to_string(response.fixed_header.control_type) + ".");
            }
        }
    }
    if (!connackReply) {
        throw RuntimeException(LOG_PRE_STR + " wait for connack message timeout.");
    }
}

}  // namespace mqtt
}  // namespace ddb
