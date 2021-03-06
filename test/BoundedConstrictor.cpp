#include "Preamble.hpp"

#include <rwols/BoundedConstrictor.hpp>

using namespace rwols;

TEST_SUITE(BoundedConstrictorSuite);

TEST(ConstructAndDestruct1) {
  BoundedConstrictor c;
  CHCK(true); // silence warnings
  // there is an assert check in the destructor. If that does not fire here,
  // then that means the test succeeded.
}

TEST(ConstructAndDestruct2) {
  for (int i = 1; i < 100; ++i) {
    BoundedConstrictor c(i);
    // there is an assert check in the destructor. If that does not fire here,
    // then that means the test succeeded.
  }
  CHCK(true); // silence warnings
}

TEST(Lock) {
  BoundedConstrictor c;
  if (auto mutator = c.Lock()) {
    CHCK(true);
  } else {
    CHCK(false);
  }
}

TEST(HasAvailableSlot) {
  BoundedConstrictor c;
  if (auto mutator = c.Lock()) {
    CHCK(mutator.HasAvailableSlot());
  }
}

TEST(Add1) {
  BoundedConstrictor c;
  if (auto mutator = c.Lock()) {
    CHCK(mutator.HasAvailableSlot());
    mutator.Add(AsyncSimulateWork());
  }
  stlab::blocking_get(c.WhenAll());
}

TEST(Add2) {
  BoundedConstrictor c;
  if (auto mutator = c.Lock()) {
    while (mutator.HasAvailableSlot()) {
      mutator.Add(AsyncSimulateWork());
    }
  }
  stlab::blocking_get(c.WhenAll());
  CHCK(true); // silence warnings
  // there is an assert check in the destructor. If that does not fire here,
  // then that means the test succeeded.
}

TEST(MapDeletionWhenAll) {
  std::map<std::string, std::unique_ptr<IFutureConstrictor>> m;
  m["foo"] = std::make_unique<BoundedConstrictor>();
  m["bar"] = std::make_unique<BoundedConstrictor>();
  m["baz"] = nullptr;
  m["qux"] = nullptr;
  for (auto &keyvalue : m) {
    if (keyvalue.second == nullptr)
      continue;
    if (auto mutator = keyvalue.second->Lock()) {
      while (mutator.HasAvailableSlot()) {
        mutator.Add(AsyncSimulateWork());
      }
    }
  }
  std::mutex mut;
  auto deletion1 = m["foo"]->WhenAll().then([&]() {
    // Have to protect mutation of this global map with a lock
    std::lock_guard<std::mutex> lock(mut);
    m.erase(m.find("foo"));
  });
  auto deletion2 = m["bar"]->WhenAll().then([&]() {
    // Have to protect mutation of this global map with a lock
    std::lock_guard<std::mutex> lock(mut);
    m.erase(m.find("bar"));
  });
  stlab::blocking_get(deletion1);
  stlab::blocking_get(deletion2);
  CHCK(m.size() == 2);
}

TEST(ShouldThrottle) {
  BoundedConstrictor c;
  auto startTime = std::chrono::high_resolution_clock::now();
  const std::size_t numFutures = std::thread::hardware_concurrency() * 3;
  for (std::size_t i = 0; i < numFutures; ++i) {
    if (auto mutator = c.Lock()) {
      mutator.Add(AsyncSimulateWork());
    }
  }
  stlab::blocking_get(c.WhenAll());
  auto duration = std::chrono::high_resolution_clock::now() - startTime;
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
  CHCK(WORKTIME.count() <= ms.count());
  CHCK(ms.count() <= 2 * WORKTIME.count());
}

TEST_SUITE_END();
