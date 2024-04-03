#include "client.h"
#include "ddbplugin/CommonInterface.h"

#ifdef LINUX
#include <poll.h>
#endif

namespace mqtt {

bool checkConnectStatus(int sockfd) {
#ifdef LINUX
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
	return true;
#endif
}

}    // namespace mqtt
