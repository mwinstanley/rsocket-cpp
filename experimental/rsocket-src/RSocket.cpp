// Copyright 2004-present Facebook. All Rights Reserved.

#include "rsocket/RSocket.h"
#include <src/NullRequestHandler.h>
#include "src/folly/FollyKeepaliveTimer.h"

namespace rsocket {

std::unique_ptr<RSocketClient> RSocket::createClient(
    std::unique_ptr<ConnectionFactory> connectionFactory) {
  return std::make_unique<RSocketClient>(std::move(connectionFactory));
}

std::unique_ptr<RSocketServer> RSocket::createServer(
    std::unique_ptr<ConnectionAcceptor> connectionAcceptor) {
  return std::make_unique<RSocketServer>(std::move(connectionAcceptor));
}
}
