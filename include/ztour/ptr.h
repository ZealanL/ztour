#pragma once

#include <ztour/base.h>

namespace ztour {
	struct Ptr {
	private:
		void* _ptr;
	public:
		Ptr() : _ptr(NULL) {}
		Ptr(const void* v) : _ptr((void*)v) {}

#if ZT_IS_64
		Ptr(uint64_t addr) : _ptr((void*)(uintptr_t)addr) {}
		Ptr(int64_t addr) : _ptr((void*)(uintptr_t)addr) {}
#endif
		Ptr(uint32_t addr) : _ptr((void*)(uintptr_t)addr) {}
		Ptr(int32_t addr) : _ptr((void*)(uintptr_t)addr) {}

		uint8_t* as_bytes_ptr() const {
			return (uint8_t*)_ptr;
		}

		uintptr_t as_addr() const {
			return (uintptr_t)_ptr;
		}

		bool operator==(Ptr other) const {
			return this->_ptr == other._ptr;
		}
		bool operator!=(Ptr other) const {
			return this->_ptr != other._ptr;
		}
		bool operator!() const {
			return !this->_ptr;
		}

		Ptr operator+(Ptr other) const {
			return Ptr(this->as_addr() + other.as_addr());
		}
		Ptr& operator+=(Ptr other) {
			return *this = *this + other;
		}

		template<typename T, typename = std::enable_if<std::is_integral<T>::value, T>::type>
		T read() const {
			return *(T*)(this->_ptr);
		}

		friend std::ostream& operator<<(std::ostream& os, const Ptr& ptr) {
			os << ZT_HEXSTR(ptr.as_addr());
			return os;
		}
	};
	static_assert(sizeof(Ptr) == sizeof(void*));
}