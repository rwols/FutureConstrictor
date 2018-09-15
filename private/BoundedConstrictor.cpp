#include <rwols/BoundedConstrictor.hpp>

#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/utility.hpp>

#include <thread>

namespace rwols {

BoundedConstrictor::BoundedConstrictor()
    : mFuts(std::thread::hardware_concurrency()) {
  assert(mFuts.size() > 0 && "This is a weird platform");
}

BoundedConstrictor::BoundedConstrictor(std::size_t max_number_of_futures)
    : mFuts(max_number_of_futures) {
  assert(mFuts.size() > 0 &&
         "Cannot function properly if I cannot hold more than one future");
}

BoundedConstrictor::~BoundedConstrictor() {
  for (auto &fut : mFuts) {
    assert((!fut.valid() || fut.get_try()) &&
           "Unfinished futures while destructing");
  }
}

stlab::future<void> BoundedConstrictor::WhenAll() {
  std::lock_guard<std::mutex> lock(mMutex);
  auto &executor = stlab::default_executor;
  for (auto &fut : mFuts) {
    if (!fut.valid())
      fut = stlab::make_ready_future(executor);
  }
  auto resetAll = [this]() {
    std::lock_guard<std::mutex> lock(mMutex);
    for (auto &fut : mFuts) {
      fut.reset();
    }
  };
  auto range = std::make_pair(mFuts.begin(), mFuts.end());
  return stlab::when_all(executor, resetAll, range);
}

bool BoundedConstrictor::UnsafeHasAvailableSlot() const noexcept {
  for (const auto &fut : mFuts) {
    if (!fut.valid())
      return true;
  }
  return false;
}

void BoundedConstrictor::UnsafeAdd(stlab::future<void> &&fut) {
  for (std::size_t index = 0; index < mFuts.size(); ++index) {
    if (!mFuts[index].valid()) {
      auto reset = [=]() { Clear(index); };
      mFuts[index] = std::move(fut).then(stlab::default_executor, reset);
      return;
    }
  }
  assert(false && "Unable to find an available slot for the given future");
}

std::mutex &BoundedConstrictor::GetMutex() noexcept { return mMutex; }

void BoundedConstrictor::Clear(std::size_t index) {
  std::lock_guard<std::mutex> lock(mMutex);
  mFuts[index].reset();
}

} // namespace rwols
