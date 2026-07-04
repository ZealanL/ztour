#pragma once
#include <ztour/base.h>

namespace ztour {
	// Rust-style mutex lock thingy
	template <typename T>
	class Mutex {
	public:
		class Guard {
		public:
			Guard(T& data, std::mutex& mtx) : _data(data), _lock(mtx) {}

			Guard(const Guard&) = delete;
			Guard& operator=(const Guard&) = delete;

			Guard(Guard&&) = default;
			Guard& operator=(Guard&&) = default;

			T* operator->() { return &this->_data; }
			T& operator*() { return this->_data; }

		private:
			T& _data;
			std::unique_lock<std::mutex> _lock;
		};

		template <typename... Args>
		explicit Mutex(Args&&... args) : _data(std::forward<Args>(args)...) {}

		Guard lock() {
			return Guard(_data, _mtx);
		}

	private:
		T _data;
		std::mutex _mtx;
	};
}