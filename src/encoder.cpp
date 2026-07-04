#include "./encoder.h"

namespace ztour {
	Encoder& Encoder::add_ptr(Ptr ptr) {
		static_assert(sizeof(ptr) == ZT_PTR_SIZE);
		this->add_raw(ptr);
		return *this;
	}

	Encoder& Encoder::add_byte(uint8_t byte) {
		this->bytes.push_back(byte);
		return *this;
	}

	Encoder& Encoder::add_bytes(std::initializer_list<uint8_t> new_bytes) {
		for (auto b : new_bytes)
			this->bytes.push_back(b);
		return *this;
	}

	Encoder& Encoder::encode_pure_jmp(Ptr to) {
#if ZT_IS_64
		add_bytes({ 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 }); // jmp [rip + 0] ...
		add_ptr(to); // ... {target address}
#else
		add_bytes({ 0x68 }); // push imm32 ...
		add_ptr(to); // ... {target address}
		add_bytes({ 0xC3 }); // ret (aka pop eip)
#endif
		return *this;
	}

	Encoder& Encoder::encode_rel_jmp(Ptr from, Ptr to) {
		Ptr real_from = from + REL_JMP_SIZE;
		int64_t offset_64 = (to.as_addr() - real_from.as_addr());
		if (offset_64 < std::numeric_limits<int32_t>::min() ||
			offset_64 > std::numeric_limits<int32_t>::max()) {
			ZT_THROW_ERR("Cannot encode relative 32-bit jmp from " << from << " to " << to << " as they are too far apart");
		}
		int32_t offset_32 = offset_64;

		this->add_byte(0xE9);
		this->add_raw<int32_t>(offset_32);
		return *this;
	}

	Encoder& Encoder::encode_inc_dec_atomic_counter(bool inc, size_t* counter_ptr) {
		static_assert(sizeof(*counter_ptr) == ZT_PTR_SIZE, "counter_ptr size is wrong");
		ZT_ASSERT(counter_ptr != 0);

		// pushf(d/q)
		add_byte(0x9C);

#if ZT_IS_64
		// push rax
		add_byte(0x50);

		// mov rax, counter_ptr
		add_bytes({ 0x48, 0xB8 }); // REX.W prefix (64-bit operand)
		add_ptr(counter_ptr);

		// lock (inc/dec) [rax]
		add_bytes({ 0xF0, 0x48, 0xFF });
		add_byte(inc ? 0x00 : 0x08);

		// pop rax
		add_byte(0x58);
#else
		// lock (inc/dec) [absolute_address]
		add_bytes({ 0xF0, 0xFF });
		add_byte(inc ? 0x05 : 0x0D);
		add_ptr(counter_ptr);
#endif

		// popf(d/q)
		add_byte(0x9D);

		return *this;
	}
}