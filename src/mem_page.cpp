#include "mem_page.h"

#include "memory.h"

namespace ztour {
    MemPage::MemPage(Ptr near_addr) : _write_offset(0) {
        this->_size = memory::page_size();
        this->_memory = memory::alloc_page_near(near_addr);
    }

    MemPage::~MemPage() {
        this->make_writable(); // Needed to be freed
        memory::free(this->_memory);
    }

    void MemPage::make_writable() {
        memory::set_mem_writable(this->_memory, this->_size);
    }

    void MemPage::make_executable() {
        memory::set_mem_executable(this->_memory, this->_size);
    }

    Ptr MemPage::write_bytes(Ptr base, size_t num) {
        if (this->_write_offset + num > this->_size)
            ZT_THROW_ERR("Tried to write outside of page bounds (num=" << num << ")");

        Ptr write_target = this->_memory + this->_write_offset;
        std::memcpy(write_target.as_bytes_ptr(), base.as_bytes_ptr(), num);
        this->_write_offset += num;
        return write_target;
    }
}