#pragma once

#include <ztour/ptr.h>


namespace ztour::memory {
	ZT_ISOLATE_SECTION void set_mem_writable(Ptr addr, size_t size);
	ZT_ISOLATE_SECTION void set_mem_executable(Ptr addr, size_t size);
	ZT_ISOLATE_SECTION size_t page_size();

	ZT_ISOLATE_SECTION void overwrite_executable_mem(Ptr from, Ptr to, size_t size);

	std::vector<uint8_t> read_bytes_vec(Ptr addr, size_t size);

	ZT_ISOLATE_SECTION Ptr alloc(size_t size, size_t alignment = 1);
	ZT_ISOLATE_SECTION void free(Ptr ptr);


	Ptr executable_mem_from_bytes(Ptr bytes, size_t num);
	Ptr executable_mem_from_bytes(const std::vector<uint8_t>& bytes);

	// Sometimes function pointers go to a jump-table,
	//	and this helper function auto-resolves them if that is the case
	Ptr resolve_func_ptr(Ptr func_ptr);
}