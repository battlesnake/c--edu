#pragma once
#include <chrono>
#include <functional>

/*
 * Debounces notifications
 */
template <typename Key, typename Notification>
struct EventDebouncer
{
	using key_type = Key;
	using notification_type = Notification;
	using callback_type = std::function<void(const key_type&, const notification_type&)>;

	using clock_type = std::chrono::steady_clock;
	using interval_type = std::chrono::microseconds;

	EventDebouncer(interval_type threshold, callback_type callback) :
		callback(callback),
		threshold(threshold),
		previous()
	{
	}

	void operator () (const key_type& key, const notification_type& notification)
	{
		auto now = clock_type::now();
		auto since_last = now - previous;
		if (since_last < threshold) {
			return;
		}
		previous = now;
		callback(key, notification);
	}

private:
	callback_type callback;
	interval_type threshold;
	clock_type::time_point previous;
};
