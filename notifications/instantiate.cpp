#include "EventEmitter.hpp"
#include "EventDebouncer.hpp"
#include "EventPoller.hpp"
#include "EventMultiPoller.hpp"

#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;
using namespace std::literals;

EventEmitter<int, string> ee{};

void dummy()
{
	ee.subscribe(42, [] (auto, const auto&) { });
	ee.notify(42, "potato");

	ee.subscribe(42, EventDebouncer<int, string>(10ms, [] (auto, const auto&) { }));
	ee.notify(42, "potato");

	ee.subscribe(42, EventPoller<int, string>(100ms, [] (auto, const auto&) { }));
	ee.notify(42, "potato");

	EventMultiPoller<int, string> mp(100ms, [] (const auto&) { });
	ee.subscribe(42, mp);
	ee.subscribe(43, mp);
	ee.notify(42, "lemon");
	ee.notify(43, "melon");
}
