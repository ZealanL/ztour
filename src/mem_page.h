#pragma once
#include <ztour/ptr.h>

namespace ztour {
	struct MemPage {
	private:
		size_t _size;
		Ptr _memory;
		size_t _write_offset;
	public:
		MemPage();
		~MemPage();
		MemPage(const MemPage& other) = delete;
		MemPage& operator=(const MemPage& other) = delete;

		size_t size() const { return _size; }

		void make_writable();
		void make_executable();

		// Returs the base address of where the `base` bytes ended up
		Ptr write_bytes(Ptr base, size_t num);
		Ptr write_bytes(const std::vector<uint8_t>& bytes) {
			ZT_DLOG("Writing bytes to codepage (at " << Ptr(bytes.data() + bytes.size()) << "): \"" << ZT_BYTES_TO_STR(bytes) << "\"");
			return write_bytes(bytes.data(), bytes.size());
		}
	};
}