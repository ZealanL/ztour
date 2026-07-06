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
        mprotect((void *) aligned_addr, aligned_size, prot);
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
            to.as_bytes_ptr(), // Destination address to overwrite
            from.as_bytes_ptr(), // Source buffer containing the JMP/detour
            size, // Number of bytes to write
            &out_bytes_written // Out variable to verify the write count
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

        if (lseek64(self_mem_file, (off64_t) to.as_addr(), SEEK_SET) == -1) {
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
        void *ptr = nullptr;
        if (posix_memalign(&ptr, alignment, size) == 0) {
            return ptr;
        } else {
            return nullptr;
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

    Ptr memory::executable_mem_from_bytes(const std::vector<uint8_t> &bytes) {
        return executable_mem_from_bytes(bytes.data(), bytes.size());
    }

    Ptr memory::resolve_func_ptr(Ptr func_ptr) {
        if (!func_ptr)
            return nullptr;

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

    std::vector<memory::MemoryRegion> memory::get_module_bin_regions(const std::string& module_name) {
#if ZT_IS_WINDOWS
        HMODULE mod = GetModuleHandleA(module_name);
        if (!mod)
            ZT_THROW_ERR("Failed to find module \"" << module_name << "\"");

        auto dos_header = (PIMAGE_DOS_HEADER)mod;
        auto nt_headers = (PIMAGE_NT_HEADERS)((uint8_t*)mod + dos_header->e_lfanew);
        size_t size_of_image = nt_headers->OptionalHeader.SizeOfImage;
        return { MemoryRegion(dos_header, size_of_image) };
#error TODO
#else
#if ZT_IS_LINUX
        std::vector<MemoryRegion> regions = {};

        auto self_maps_stream = std::ifstream("/proc/self/maps");
        if (!self_maps_stream.is_open())
            ZT_THROW_ERR("Cannot open \"/proc/self/maps\"");

        std::string line;
        while (std::getline(self_maps_stream, line)) {
            auto ss = std::stringstream(line);

            uintptr_t start, end;
            char dash;
            std::string perms;
            long offset;
            std::string unk;
            long inode;
            std::string path;

            // Parse hex addresses (e.g., "7f92a1000000-7f92a115a000")
            if (!(
                ss
                >> std::hex >> start >> dash >> std::hex >> end >> perms
                >> std::hex >> offset >> unk >> std::dec >> inode >> path
                )) {
                continue; // Failed to parse
            }

            // Verify permissions
            {
                if (perms.find('r') == std::string::npos)
                    continue; // Unreadable
            }

            // Verify path matches module name
            {
                size_t last_path_slash = path.find_last_of('/');
                if (last_path_slash == std::string::npos)
                    continue; // Invalid path (missing or not actually a path)

                std::string executable_name = path.substr(last_path_slash + 1);
                if (executable_name != module_name)
                    continue;
            }


            ZT_ASSERT(end > start);
            size_t size = end - start;
            regions.push_back(MemoryRegion(start, size));
        }

        if (regions.empty())
            ZT_THROW_ERR("Failed to find module \"" << module_name << "\"");

        return regions;

#else
#error TODO
#endif
#endif
    }

    memory::SignedBytePattern memory::parse_pattern_str(const std::string& pattern_str) {
        SignedBytePattern bytes = {};
        auto ss = std::stringstream(pattern_str);
        std::string byte_str;
        while (ss >> byte_str) {
            if (byte_str == "?" || byte_str == "??") {
                bytes.push_back(-1);
            } else {
                try {
                    auto byte_val = std::stoul(byte_str, nullptr, 16);
                    if (byte_val > 0xFF)
                        ZT_THROW_ERR("Cannot parse pattern byte, \"" << byte_str << "\" exceeds byte limit of 0xFF");
                    bytes.push_back(byte_val);
                } catch (...) {
                    ZT_THROW_ERR("Cannot parse pattern byte, cannot read: \"" << byte_str << "\" as a hex byte");
                }
            }
        }

        return bytes;
    }

    Ptr memory::signed_pattern_scan_region(MemoryRegion region, const SignedBytePattern& pattern) {
        if (pattern.size() == 0 || region.size < pattern.size())
            return nullptr;

        size_t leading_wildcards = 0;
        while (leading_wildcards < pattern.size() && pattern[leading_wildcards] == -1)
            leading_wildcards++;

        if (leading_wildcards == pattern.size())
            ZT_THROW_ERR("Cannot scan with pattern consisting of only wildcards");

        uint8_t first_valid_byte = pattern[leading_wildcards];

        Ptr cur_start_pos = (region.base+ leading_wildcards);
        size_t remaining_size = region.size - leading_wildcards;
        size_t min_size = pattern.size() - leading_wildcards;
        while (remaining_size >= min_size) {
            Ptr next_possible_base = std::memchr(cur_start_pos.as_bytes_ptr(), first_valid_byte, remaining_size);
            if (!next_possible_base)
                return nullptr;

            { // Check pattern
                bool matches = true;
                for (size_t i = leading_wildcards + 1, j = 1; i < pattern.size(); i++,j++) {
                    if (pattern[i] == -1) continue;
                    if (pattern[i] != next_possible_base.as_bytes_ptr()[j]) {
                        matches = false;
                        break;
                    }
                }

                if (matches) {
                    return next_possible_base.as_addr() - leading_wildcards;
                }
            }

            Ptr next_start_pos = next_possible_base + 1;
            size_t step_size = next_start_pos.as_addr() - cur_start_pos.as_addr();
            ZT_ASSERT(step_size <= remaining_size);
            remaining_size -= step_size;
            cur_start_pos = next_start_pos;
        }

        return nullptr;
    }
}