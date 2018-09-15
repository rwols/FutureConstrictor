#pragma once

#include <rwols/IFutureConstrictor.hpp>

#include <vector>

namespace rwols {

class BoundedConstrictor final : public IFutureConstrictor {
public:
  BoundedConstrictor();
  BoundedConstrictor(std::size_t max_number_of_futures);

  ~BoundedConstrictor() override;

  stlab::future<void> WhenAll() override;

private:
  bool UnsafeHasAvailableSlot() const noexcept override;
  void UnsafeAdd(stlab::future<void> &&) override;
  std::mutex &GetMutex() noexcept override;
  void Clear(std::size_t index);

  std::vector<stlab::future<void>> mFuts;
  std::mutex mMutex;

  friend class Mutator;
};

} // namespace rwols
