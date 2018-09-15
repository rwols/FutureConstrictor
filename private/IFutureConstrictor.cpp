#include <rwols/IFutureConstrictor.hpp>

namespace rwols {

IFutureConstrictor::Mutator IFutureConstrictor::Lock() { return Mutator(this); }

IFutureConstrictor::Mutator::Mutator(Mutator &&other) noexcept
    : mSelf(other.mSelf) {
  std::swap(mLock, other.mLock);
  other.mSelf = nullptr;
}

IFutureConstrictor::Mutator &IFutureConstrictor::Mutator::
operator=(Mutator &&other) noexcept {
  this->~Mutator(); // Release the current lock.
  std::swap(mLock, other.mLock);
  mSelf = other.mSelf;
  other.mSelf = nullptr;
  return *this;
}

IFutureConstrictor::Mutator::~Mutator() {
  if (mSelf)
    mLock.unlock();
}

bool IFutureConstrictor::Mutator::HasAvailableSlot() const noexcept {
  return mSelf->UnsafeHasAvailableSlot();
}

void IFutureConstrictor::Mutator::Add(stlab::future<void> &&procedure) {
  mSelf->UnsafeAdd(std::move(procedure));
}

IFutureConstrictor::Mutator::operator bool() const noexcept {
  return HasAvailableSlot();
}

IFutureConstrictor::Mutator::Mutator(IFutureConstrictor *self)
    : mSelf(self), mLock(self->GetMutex()) {}

} // namespace rwols
