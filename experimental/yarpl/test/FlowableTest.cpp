#include <future>
#include <vector>

#include <gtest/gtest.h>

#include "yarpl/Flowables.h"

namespace yarpl {

namespace {

template <typename T>
class CollectingSubscriber : public Subscriber<T> {
 public:
  virtual void onSubscribe(Reference<Subscription> subscription) override {
    Subscriber<T>::onSubscribe(subscription);
    subscription->request(100);
  }

  virtual void onNext(const T& next) override {
    Subscriber<T>::onNext(next);
    values_.push_back(next);
  }

  const std::vector<T>& values() const {
    return values_;
  }

 private:
  std::vector<T> values_;
};

/// Construct a pipeline with a collecting subscriber against the supplied
/// flowable.  Return the items that were sent to the subscriber.  If some
/// exception was sent, the exception is thrown.
template <typename T>
std::vector<T> run(Reference<Flowable<T>> flowable) {
  auto collector =
      Reference<CollectingSubscriber<T>>(new CollectingSubscriber<T>);
  auto subscriber = Reference<Subscriber<T>>(collector.get());
  flowable->subscribe(std::move(subscriber));
  return collector->values();
}

} // namespace

TEST(FlowableTest, SingleFlowable) {
  ASSERT_EQ(std::size_t{0}, Refcounted::objects());

  auto flowable = Flowables::just(10);
  EXPECT_EQ(std::size_t{1}, Refcounted::objects());
  EXPECT_EQ(std::size_t{1}, flowable->count());

  flowable.reset();
  EXPECT_EQ(std::size_t{0}, Refcounted::objects());
}

TEST(FlowableTest, JustFlowable) {
  ASSERT_EQ(std::size_t{0}, Refcounted::objects());
  EXPECT_EQ(run(Flowables::just(22)), std::vector<int>{22});
  EXPECT_EQ(
      run(Flowables::just({12, 34, 56, 98})),
      std::vector<int>({12, 34, 56, 98}));
  EXPECT_EQ(
      run(Flowables::just({"ab", "pq", "yz"})),
      std::vector<const char*>({"ab", "pq", "yz"}));
  EXPECT_EQ(std::size_t{0}, Refcounted::objects());
}

TEST(FlowableTest, Range) {
  ASSERT_EQ(std::size_t{0}, Refcounted::objects());
  EXPECT_EQ(
      run(Flowables::range(10, 15)),
      std::vector<int64_t>({10, 11, 12, 13, 14}));
  EXPECT_EQ(std::size_t{0}, Refcounted::objects());
}

TEST(FlowableTest, RangeWithMap) {
  ASSERT_EQ(std::size_t{0}, Refcounted::objects());
  auto flowable = Flowables::range(1, 4)
                      ->map([](int64_t v) { return v * v; })
                      ->map([](int64_t v) { return v * v; })
                      ->map([](int64_t v) { return std::to_string(v); });
  EXPECT_EQ(
      run(std::move(flowable)), std::vector<std::string>({"1", "16", "81"}));
  EXPECT_EQ(std::size_t{0}, Refcounted::objects());
}

TEST(FlowableTest, SimpleTake) {
  ASSERT_EQ(std::size_t{0}, Refcounted::objects());
  EXPECT_EQ(
      run(Flowables::range(0, 100)->take(3)), std::vector<int64_t>({0, 1, 2}));
  EXPECT_EQ(
      run(Flowables::range(10, 15)),
      std::vector<int64_t>({10, 11, 12, 13, 14}));
  EXPECT_EQ(std::size_t{0}, Refcounted::objects());
}

} // yarpl
