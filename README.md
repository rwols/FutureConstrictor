# FutureConstrictor

Collection of classes to throttle/constrict the number of concurrent futures.
Requires https://github.com/stlab/libraries.

# Installation

Configure, build and install with cmake. e.g.
```bash
mkdir -p build/release
pushd build/release
cmake ../.. -DCMAKE_BUILD_TYPE=Release
make -j8
make test
make install
popd
```

# Usage

In your CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(MyProject VERSION 0.1.0 LANGUAGES CXX)
find_package(FutureConstrictor REQUIRED CONFIG)
add_executable(foo main.cpp)
target_link_libraries(foo PRIVATE rwols::FutureConstrictor)
```

The main interface class is `IFutureConstrictor`. Three implementation classes
are provided:
- `BoundedConstrictor`: Its default constructor creates a vector of void futures
  the size of the number of logical cores of the machine.
- `UnboundedConstrictor`: A list of void futures that may grow arbitrarily
  large.
- `SingleConstrictor`: Maintain a single void future.

You cannot append void futures via the `IFutureConstrictor` itself. For that,
you first need to lock it and obtain a valid mutator proxy object:

```c++
IFutureConstrictor& constrictor = GetConstrictor();
if (auto mutator = constrictor.Lock()) {
  mutator.Add(stlab::async(stlab::default_executor, SomeProcedure));
} else {
  // There are too many futures in-flight. Do something else.
}
```

When it is time to tear things down, or you need a synchronization point, you
can use the `.WhenAll()` method of the `IFutureConstrictor`. This gives you a
void future that resolves once all held void futures are resolved. So, to
safely destruct the `IFutureConstrictor` itself, you can do something like

```c++
void CleanUp(std::unique_ptr<IFutureConstrictor>& constrictor) {
  auto f = constrictor->WhenAll();
  std::move(f).recover([&](const auto&){ constrictor.reset(); }).detach();
}
```

Of course, that unique_ptr passed by reference should remain valid so long as
all held futures are running.

Here is a minimal C++ example:

```c++
#include <cstdlib>
#include <iostream>
#include <rwols/BoundedConstrictor.hpp>
#include <rwols/IFutureConstrictor.hpp>
#include <stlab/concurrency/default_executor.hpp>
#include <stlab/concurrency/utility.hpp>
#include <thread>

namespace {

std::mutex printMut;

void SayHello(const std::size_t counter) {
  // Simulate work.
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  const std::lock_guard<std::mutex> printLock(printMut);
  std::cout << counter + 1 << ": Hello, world!\n";
}

std::size_t FillUp(rwols::IFutureConstrictor &futs) {
  auto mutator = futs.Lock();
  std::size_t counter = 0;
  while (mutator.HasAvailableSlot()) {
    auto blockingOperation = [=]() { SayHello(counter); };
    mutator.Add(stlab::async(stlab::default_executor, blockingOperation));
    ++counter;
  }
  return counter;
}

} // namespace

int main(int argc, char const *argv[]) {
  auto futs = std::make_unique<rwols::BoundedConstrictor>();
  const auto numFutures = FillUp(*futs);
  {
    const std::lock_guard<std::mutex> printLock(printMut);
    std::cout << "Constrictor filled up with " << numFutures << " futures.\n";
  }
  stlab::blocking_get(futs->WhenAll());
  return EXIT_SUCCESS;
}
```
