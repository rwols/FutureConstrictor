#pragma once

#include <rwols/IFutureConstrictor.hpp>

#include <list>

namespace rwols {

class UnboundedConstrictor final : public IFutureConstrictor {
public:
  UnboundedConstrictor() = default;

  ~UnboundedConstrictor() override;

  stlab::future<void> WhenAll() override;

private:
  bool UnsafeHasAvailableSlot() const noexcept override;
  void UnsafeAdd(stlab::future<void> &&) override;
  std::mutex &GetMutex() noexcept override;
  void Clear(std::list<stlab::future<void>>::const_iterator);

  std::list<stlab::future<void>> mFuts;
  std::mutex mMutex;

  friend class Mutator;
};

} // namespace rwols
