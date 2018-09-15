#include <boost/test/unit_test.hpp>

#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/future.hpp>
#include <stlab/concurrency/utility.hpp>

#include <thread>

#define TEST BOOST_AUTO_TEST_CASE
#define TEST_SUITE BOOST_AUTO_TEST_SUITE
#define TEST_SUITE_END BOOST_AUTO_TEST_SUITE_END
#define WORKTIME std::chrono::milliseconds(100)

// For some reason, if I define this as "CHECK" my compiler craps out.
#define CHCK BOOST_TEST

inline void SimulateWork() { std::this_thread::sleep_for(WORKTIME); }

inline stlab::future<void> AsyncSimulateWork() {
  return stlab::async(stlab::default_executor, SimulateWork);
}

inline stlab::future<void> Throw() {
  return stlab::async(stlab::default_executor, []() {
    SimulateWork();
    throw std::logic_error("just for testing purposes");
  });
}
