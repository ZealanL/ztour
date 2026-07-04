#pragma once
#include <ztour/ptr.h>

namespace ztour {
	struct Encoder {
		std::vector<uint8_t> bytes;

		template<typename T>
		Encoder& add_raw(const T& val) {
			auto as_byte_array = (uint8_t*)&val;
			for (int i = 0; i < sizeof(T); i++)
				this->add_byte(as_byte_array[i]);
			return *this;
		}
		Encoder& add_ptr(Ptr ptr);
		Encoder& add_byte(uint8_t byte);
		Encoder& add_bytes(std::initializer_list<uint8_t> bytes);
		Encoder& encode_pure_jmp(Ptr to);
		Encoder& encode_rel_jmp(Ptr from, Ptr to);
		Encoder& encode_inc_dec_atomic_counter(bool inc, size_t* counter_ptr);

		constexpr static size_t REL_JMP_SIZE = 5;
	};
}