// Copyright 2004-present Facebook. All Rights Reserved.

#include "src/StreamsFactory.h"
#include "src/ConnectionAutomaton.h"
#include "src/automata/ChannelRequester.h"
#include "src/automata/ChannelResponder.h"
#include "src/automata/RequestResponseRequester.h"
#include "src/automata/RequestResponseResponder.h"
#include "src/automata/StreamRequester.h"
#include "src/automata/StreamResponder.h"

namespace reactivesocket {

StreamsFactory::StreamsFactory(
    ConnectionAutomaton& connection,
    ReactiveSocketMode mode)
    : connection_(connection),
      nextStreamId_(
          mode == ReactiveSocketMode::CLIENT
              ? 1 /*Streams initiated by a client MUST use
                    odd-numbered stream identifiers*/
              : 2 /*streams initiated by the server MUST use
                    even-numbered stream identifiers*/) {}

std::shared_ptr<Subscriber<Payload>> StreamsFactory::createChannelRequester(
    std::shared_ptr<Subscriber<Payload>> responseSink,
    folly::Executor& executor) {
  ChannelRequester::Parameters params = {
      {connection_.shared_from_this(), getNextStreamId()}, executor};
  auto automaton = std::make_shared<ChannelRequester>(params);
  connection_.addStream(params.streamId, automaton);
  automaton->subscribe(std::move(responseSink));
  return automaton;
}

void StreamsFactory::createStreamRequester(
    Payload request,
    std::shared_ptr<Subscriber<Payload>> responseSink,
    folly::Executor& executor) {
  StreamRequester::Parameters params = {
      {connection_.shared_from_this(), getNextStreamId()}, executor};
  auto automaton =
      std::make_shared<StreamRequester>(params, std::move(request));
  connection_.addStream(params.streamId, automaton);
  automaton->subscribe(std::move(responseSink));
}

void StreamsFactory::createRequestResponseRequester(
    Payload payload,
    std::shared_ptr<Subscriber<Payload>> responseSink,
    folly::Executor& executor) {
  RequestResponseRequester::Parameters params = {
      {connection_.shared_from_this(), getNextStreamId()}, executor};
  auto automaton =
      std::make_shared<RequestResponseRequester>(params, std::move(payload));
  connection_.addStream(params.streamId, automaton);
  automaton->subscribe(std::move(responseSink));
}

StreamId StreamsFactory::getNextStreamId() {
  StreamId streamId = nextStreamId_;
  CHECK(streamId <= std::numeric_limits<int32_t>::max() - 2);
  nextStreamId_ += 2;
  return streamId;
}

bool StreamsFactory::registerNewPeerStreamId(StreamId streamId) {
  DCHECK(streamId != 0);
  if (nextStreamId_ % 2 == streamId % 2) {
    // if this is an unknown stream to the socket and this socket is
    // generating
    // such stream ids, it is an incoming frame on the stream which no longer
    // exist
    return false;
  }
  if (streamId <= lastPeerStreamId_) {
    // receiving frame for a stream which no longer exists
    return false;
  }
  lastPeerStreamId_ = streamId;
  return true;
}

std::shared_ptr<ChannelResponder> StreamsFactory::createChannelResponder(
    uint32_t initialRequestN,
    StreamId streamId,
    folly::Executor& executor) {
  ChannelResponder::Parameters params = {
      {connection_.shared_from_this(), streamId}, executor};
  auto automaton = std::make_shared<ChannelResponder>(initialRequestN, params);
  connection_.addStream(streamId, automaton);
  return automaton;
}

std::shared_ptr<Subscriber<Payload>> StreamsFactory::createStreamResponder(
    uint32_t initialRequestN,
    StreamId streamId,
    folly::Executor& executor) {
  StreamResponder::Parameters params = {
      {connection_.shared_from_this(), streamId}, executor};
  auto automaton = std::make_shared<StreamResponder>(initialRequestN, params);
  connection_.addStream(streamId, automaton);
  return automaton;
}

std::shared_ptr<Subscriber<Payload>>
StreamsFactory::createRequestResponseResponder(
    StreamId streamId,
    folly::Executor& executor) {
  RequestResponseResponder::Parameters params = {
      {connection_.shared_from_this(), streamId}, executor};
  auto automaton = std::make_shared<RequestResponseResponder>(params);
  connection_.addStream(streamId, automaton);
  return automaton;
}

} // reactivesocket
