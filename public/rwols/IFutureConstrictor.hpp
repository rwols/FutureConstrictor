#pragma once

#include <stlab/concurrency/future.hpp>

#include <mutex>

namespace rwols {

class IFutureConstrictor {
public:
  struct Mutator final {
    bool HasAvailableSlot() const noexcept;
    void Add(stlab::future<void> &&procedure);
    operator bool() const noexcept;

    Mutator(Mutator &&) noexcept;
    Mutator &operator=(Mutator &&) noexcept;

    ~Mutator();

  private:
    IFutureConstrictor *mSelf = nullptr;
    std::unique_lock<std::mutex> mLock;
    Mutator(IFutureConstrictor *self);
    friend class IFutureConstrictor;
  };

  virtual ~IFutureConstrictor() = default;

  Mutator Lock();

  virtual stlab::future<void> WhenAll() = 0;

private:
  virtual bool UnsafeHasAvailableSlot() const noexcept = 0;
  virtual void UnsafeAdd(stlab::future<void> &&) = 0;
  virtual std::mutex &GetMutex() noexcept = 0;
};

} // namespace rwols
