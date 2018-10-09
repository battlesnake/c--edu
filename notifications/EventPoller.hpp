#pragma once
#include <functional>
#include <chrono>
#include <mutex>
#include <thread>

/*
 * Reduces event-driven notifications to a periodic notification.
 *
 * If call_always is false, will skip delivering notifications if their value is
 * the same as the one which was previously delivered.
 */
template <typename Key, typename Notification>
struct EventPoller
{
	using key_type = Key;
	using notification_type = Notification;
	using callback_type = std::function<void(const key_type&, const notification_type&)>;

	using clock_type = std::chrono::steady_clock;
	using time_point_type = clock_type::time_point;
	using interval_type = std::chrono::microseconds;

	EventPoller(interval_type interval, callback_type callback, bool call_always = true) :
		EventPoller(clock_type::now(), interval, callback, call_always)
	{
	}

	EventPoller(time_point_type start, interval_type interval, callback_type callback, bool call_always = true) :
		exiting(false),
		poller([&] () { poll_thread(callback, start, interval, call_always); })
	{
	}

	~EventPoller()
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
		current = { key, notification };
	}

private:
	std::mutex mx;
	std::optional<std::pair<key_type, notification_type>> current;
	bool exiting;
	std::thread poller;

	void poll_thread(callback_type callback, time_point_type start, interval_type interval, bool call_always)
	{
		decltype(current) last_sent;
		auto last = start;
		std::unique_lock lock(mx);
		while (!exiting) {
			if (call_always || current != last_sent) {
				/*
				 * Copy value locally, call callback outside of
				 * lock
				 */
				last_sent = current;
				lock.unlock();
				callback(last_sent.first, last_sent.second);
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
