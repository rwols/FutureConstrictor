#pragma once

#include <rwols/IFutureConstrictor.hpp>

namespace rwols {

class SingleConstrictor final : public IFutureConstrictor {
public:
  SingleConstrictor() = default;

  ~SingleConstrictor() override;

  stlab::future<void> WhenAll() override;

private:
  bool UnsafeHasAvailableSlot() const noexcept override;
  void UnsafeAdd(stlab::future<void> &&) override;
  std::mutex &GetMutex() noexcept override;
  void Clear();

  stlab::future<void> mFut;
  std::mutex mMutex;

  friend class Mutator;
};

} // namespace rwols
