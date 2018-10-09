#pragma once
#include <functional>
#include <chrono>
#include <mutex>
#include <thread>

/*
 * Like EventPoller, but treats notifications with different keys differently.
 *
 * When call_always is false, if any key's notification has changed since the
 * last update, all notifications (including unchanged ones) are delivered as a
 * std::unordered_map.
 */
template <typename Key, typename Notification>
struct EventMultiPoller
{
	using key_type = Key;
	using notification_type = Notification;
	using callback_type = std::function<void(const std::unordered_map<key_type&, notification_type&>&)>;

	using clock_type = std::chrono::steady_clock;
	using time_point_type = clock_type::time_point;
	using interval_type = std::chrono::microseconds;

	EventMultiPoller(interval_type interval, callback_type callback, bool call_always = true) :
		EventMultiPoller(clock_type::now(), interval, callback, call_always)
	{
	}

	EventMultiPoller(time_point_type start, interval_type interval, callback_type callback, bool call_always = true) :
		exiting(false),
		poller([&] () { poll_thread(callback, start, interval, call_always); })
	{
	}

	~EventMultiPoller()
	{
		{
			std::scoped_lock lock(mx);
			exiting = true;
		}
		poller.join();
	}

	void operator () (const key_type& key, const notification_type& notification)
	{
		std::scoped_lock lock(mx);
		current[key] = notification;
	}

private:
	std::mutex mx;
	bool exiting;
	std::unordered_map<key_type, notification_type> current;
	std::thread poller;

	void poll_thread(callback_type callback, time_point_type start, interval_type interval, bool call_always)
	{
		std::unordered_map<key_type, notification_type> last_sent;
		auto last = start;
		std::unique_lock lock(mx);
		while (!exiting) {
			if (call_always || current != last_sent) {
				/*
				 * Copy values locally, call callback outside of
				 * lock
				 */
				last_sent = current;
				lock.unlock();
				callback(last_sent);
				lock.lock();
			}
			/* Sleep outside of lock */
			last += interval;
			lock.unlock();
			std::this_thread::sleep_until(last);
			lock.lock();
		}
	}
};
