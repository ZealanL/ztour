#pragma once

#include <ztour/ptr.h>

namespace ztour {
	namespace memory {
		void set_mem_writable(Ptr addr, size_t size);
		void set_mem_executable(Ptr addr, size_t size);
		size_t page_size();

		void overwrite_executable_mem(Ptr from, Ptr to, size_t size);

		std::vector<uint8_t> read_bytes_vec(Ptr addr, size_t size);

		Ptr try_alloc_page_at(Ptr exact_addr);

		// Allocate a page of memory as close to the given address as possible
		Ptr alloc_page_near(Ptr near_addr);

		Ptr alloc(size_t size, size_t alignment = 1);
		void free(Ptr ptr);

		Ptr executable_mem_from_bytes(Ptr bytes, size_t num);
		Ptr executable_mem_from_bytes(const std::vector<uint8_t>& bytes);

		// Sometimes function pointers go to a jump-table,
		//	and this helper function auto-resolves them if that is the case
		Ptr resolve_func_ptr(Ptr func_ptr);


		typedef std::vector<int16_t> SignedBytePattern;
		struct MemoryRegion {
			Ptr base;
			size_t size;

			MemoryRegion(Ptr base, size_t size) : base(base), size(size) {}
		};

		SignedBytePattern parse_pattern_str(const std::string& pattern_str);
		std::vector<MemoryRegion> get_module_bin_regions(const std::string& module_name);
		std::vector<Ptr> signed_pattern_scan_region(MemoryRegion region, const SignedBytePattern& pattern);
	}
}