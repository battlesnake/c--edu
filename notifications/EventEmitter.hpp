#pragma once
#include <unordered_map>
#include <functional>

/*
 * Allows binding observers to keys/channels, and receiving notifications posted
 * to those keys/channels
 */
template <typename Key, typename Notification>
struct EventEmitter
{
	using key_type = Key;
	using notification_type = Notification;
	using callback_type = std::function<void(const key_type&, const notification_type&)>;

	void subscribe(const Key& key, callback_type observer)
	{
		observers.emplace(key, std::move(observer));
	}

	/*
	 * Unsubscribe from unordered_multimap is not trivial, since the
	 * callback may not have a good equality comparison available, e.g. if
	 * a lambda was passed in, or a method via std::bind.  The implicit cast
	 * to callback_type will result in construction of a new type that might
	 * not support equality comparison.
	 */

	void notify(const key_type& key, const notification_type& notification)
	{
		auto range = observers.equal_range(key);
		for (auto& it = range.first; it != range.second; ++it) {
			auto& callback = it->second;
			callback(key, notification);
		}
	}

private:
	std::unordered_multimap<key_type, callback_type> observers;
};
