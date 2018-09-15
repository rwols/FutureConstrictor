#include <rwols/SingleConstrictor.hpp>

#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/utility.hpp>

namespace rwols {

SingleConstrictor::~SingleConstrictor() {
  assert(!mFut.valid() && "ongoing task while destructing");
}

stlab::future<void> SingleConstrictor::WhenAll() { return mFut; }

bool SingleConstrictor::UnsafeHasAvailableSlot() const noexcept {
  return !mFut.valid();
}

void SingleConstrictor::UnsafeAdd(stlab::future<void> &&fut) {
  assert(!mFut.valid() && "task already in-flight");
  mFut = std::move(fut).then(stlab::default_executor, [=]() { Clear(); });
}

std::mutex &SingleConstrictor::GetMutex() noexcept { return mMutex; }

void SingleConstrictor::Clear() {
  std::lock_guard<std::mutex> lock(mMutex);
  mFut.reset();
}

} // namespace rwols
