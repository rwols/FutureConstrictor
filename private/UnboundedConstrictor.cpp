#include <rwols/UnboundedConstrictor.hpp>

#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/utility.hpp>

namespace rwols {

UnboundedConstrictor::~UnboundedConstrictor() {
  assert(mFuts.empty() && "Futures are still inflight");
}

stlab::future<void> UnboundedConstrictor::WhenAll() {
  std::lock_guard<std::mutex> lock(mMutex);
  auto range = std::make_pair(mFuts.begin(), mFuts.end());
  return stlab::when_all(stlab::default_executor, []() {}, range);
}

bool UnboundedConstrictor::UnsafeHasAvailableSlot() const noexcept {
  return true;
}

void UnboundedConstrictor::UnsafeAdd(stlab::future<void> &&procedure) {
  mFuts.push_front(std::move(procedure));
  auto iter = mFuts.begin();
  auto clear = [=](auto &&) { Clear(iter); };
  mFuts.front() = std::move(mFuts.front()).recover(clear);
}

std::mutex &UnboundedConstrictor::GetMutex() noexcept { return mMutex; }

void UnboundedConstrictor::Clear(
    std::list<stlab::future<void>>::const_iterator iter) {
  std::lock_guard<std::mutex> lock(mMutex);
  mFuts.erase(iter);
}

} // namespace rwols
