// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include "src/Payload.h"
#include "rsocket/RSocket.h"

class TextRequestHandler : public rsocket::RSocketRequestHandler {
public:
    /// Handles a new inbound Stream requested by the other end.
    yarpl::Reference<yarpl::Flowable<reactivesocket::Payload>>
    handleRequestStream(
            reactivesocket::Payload request,
            reactivesocket::StreamId streamId) override;
};
