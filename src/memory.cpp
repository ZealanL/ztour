#include "./memory.h"

#if ZT_IS_WINDOWS
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace ztour {
	void set_mem_prot(Ptr base_ptr, size_t size, bool executable) {
#if ZT_IS_WINDOWS
		DWORD old;

		DWORD new_prot = executable ? PAGE_EXECUTE_READ : PAGE_READWRITE;
		if (!VirtualProtect(base_ptr.as_bytes_ptr(), size, new_prot, &old)) {
			ZT_THROW_ERR("VirtualProtect failed to set memory to " << ZT_HEXSTR(new_prot) << " at " << base_ptr);
		}
#else

		size_t page_size = memory::page_size();
		uintptr_t addr = base_ptr.as_addr();
		uintptr_t aligned_addr = addr & ~(page_size - 1);
		size_t aligned_size = size + (addr - aligned_addr);
		int prot = executable ? (PROT_READ | PROT_EXEC) : (PROT_READ | PROT_WRITE);
		mprotect((void*)aligned_addr, aligned_size, prot);
#endif
	}

	void memory::set_mem_writable(Ptr addr, size_t size) {
		set_mem_prot(addr, size, false);
	}

	void memory::set_mem_executable(Ptr addr, size_t size) {
		set_mem_prot(addr, size, true);
	}

	size_t memory::page_size() {
#if ZT_IS_WINDOWS
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		size_t page_size = si.dwPageSize;
#else
		size_t page_size = sysconf(_SC_PAGESIZE);
#endif
		if ((page_size & (page_size - 1)) != 0)
			ZT_THROW_ERR("Page size is not a power of 2: " << ZT_HEXSTR(page_size));

		if (page_size < 0x100)
			ZT_THROW_ERR("Page size is way too small: " << ZT_HEXSTR(page_size));

		return page_size;
	}

	void memory::overwrite_executable_mem(Ptr from, Ptr to, size_t size) {
#if ZT_IS_WINDOWS
		// Use WriteProcesMemory to not have to worry about page protections

		HANDLE process_handle = GetCurrentProcess();
		size_t out_bytes_written = 0;
		bool success = WriteProcessMemory(
			process_handle,
			to.as_bytes_ptr(),   // Destination address to overwrite
			from.as_bytes_ptr(),      // Source buffer containing the JMP/detour
			size,       // Number of bytes to write
			&out_bytes_written    // Out variable to verify the write count
		);

		if (!success || bytes_written != patch_size) {
			ZT_THROW_ERR("Failed to write " << ZT_HEXSTR(size) << " [" << from << "->" << to << "]");
		}

		// TODO: may be unnecessary
		FlushInstructionCache(process_handle, to.as_bytes_ptr(), size);

#else
#if ZT_IS_LINUX

		// Use kernel to avoid user-mode limitations (`process_vm_writev` does not work for this, fyi)
		int self_mem_file = open("/proc/self/mem", O_WRONLY);
		if (self_mem_file == -1)
			ZT_THROW_ERR("Failed to open own memory file");

		if (lseek64(self_mem_file, (off64_t)to.as_addr(), SEEK_SET) == -1) {
			close(self_mem_file);
			ZT_THROW_ERR("Failed to seek to target virtual address in own memory file");
		}

		ssize_t bytes_written = write(self_mem_file, from.as_bytes_ptr(), size);
		close(self_mem_file);

		if (bytes_written != size) {
			ZT_THROW_ERR(
				"Failed to write " << ZT_HEXSTR(size) << " [" << from << "->" << to << "]" <<
				" (written: " << ZT_HEXSTR(bytes_written) << ")");
		}

#endif
#if ZT_IS_MACOS
#error TODO
#endif
#endif
	}

	std::vector<uint8_t> memory::read_bytes_vec(Ptr addr, size_t size) {
		return std::vector(addr.as_bytes_ptr(), (addr + size).as_bytes_ptr());
	}

	Ptr memory::alloc(size_t size, size_t alignment) {

		if (size < 1)
			ZT_THROW_ERR("Cannot allocate with size=" << ZT_HEXSTR(size));
		if (alignment < 1)
			ZT_THROW_ERR("Cannot align with alignment=" << ZT_HEXSTR(alignment));

		constexpr size_t ALLOC_LIMIT = 0x10000;
		if (size > ALLOC_LIMIT)
			ZT_THROW_ERR(
				"Cannot allocate over " << ZT_HEXSTR(ALLOC_LIMIT) << " bytes at a time"
				<< " (tried " << ZT_HEXSTR(size) << ")"
			);

		if (alignment <= 1)
			return std::malloc(size);

#if ZT_IS_WINDOWS
		return _aligned_malloc(size, alignment);
#else
		void* ptr = NULL;
		if (posix_memalign(&ptr, alignment, size) == 0) {
			return ptr;
		} else {
			return NULL;
		}
#endif
	}

	void memory::free(Ptr ptr) {
#if ZT_IS_WINDOWS
		return _aligned_free(ptr.as_bytes_ptr());
#else
		std::free(ptr.as_bytes_ptr());
#endif
	}

	Ptr memory::executable_mem_from_bytes(Ptr bytes, size_t num) {
		Ptr mem = memory::alloc(num, memory::page_size());
		memory::set_mem_writable(mem, num);
		std::memcpy(mem.as_bytes_ptr(), bytes.as_bytes_ptr(), num);
		memory::set_mem_executable(mem, num);
		return mem;
	}

	Ptr memory::executable_mem_from_bytes(const std::vector<uint8_t>& bytes) {
		return executable_mem_from_bytes(bytes.data(), bytes.size());
	}

	Ptr memory::resolve_func_ptr(Ptr func_ptr) {
		constexpr uint8_t JMP_REL32_MNEMONIC = 0xE9;
		constexpr size_t JMP_REL32_LEN = 5;
		if (func_ptr.read<uint8_t>() == JMP_REL32_MNEMONIC) {
			int32_t relative_offset = (func_ptr + 1).read<int32_t>();
			Ptr real_ptr = func_ptr + relative_offset + JMP_REL32_LEN;
			ZT_DLOG("Function pointer " << func_ptr << " resolved as a rel jmp to " << real_ptr);
			return real_ptr;
		}

		return func_ptr;
	}
}