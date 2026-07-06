#include "../relocation.h"

#include "../decoder.h"

template <typename T>
void ensure_displacement_reloc_bounds(int64_t displacement) {
    if (displacement < std::numeric_limits<T>::min() || displacement > std::numeric_limits<T>::max()) {
        ZT_THROW_ERR("Relocation failure, displacement of " << displacement << " cannot fit in type " << typeid(T).name());
    }
}

std::vector<uint8_t> ztour::relocation::relocate_code(const std::vector<uint8_t>& code_bytes, Ptr from_base, Ptr to_base) {
    ZT_ASSERT(!code_bytes.empty());

    if (from_base == to_base)
        return code_bytes;

    std::vector<uint8_t> results = {};
    results.reserve(code_bytes.size());

    auto decoder = Decoder();
    size_t offset = 0;
    while (offset < code_bytes.size()) {
        size_t max_len = code_bytes.size() - offset;

        auto ins_og_ptr = code_bytes.data() + offset;
        Ptr remote_address_from = from_base + offset;
        Ptr remote_address_to = to_base + offset;
        auto ins = decoder.disassemble_instruction(ins_og_ptr, max_len, remote_address_from);
        auto ins_bytes = std::vector(ins_og_ptr, ins_og_ptr + ins.info.length);
        for (int i = 0; i < ins.info.operand_count; ++i) {
            auto& operand = ins.operands[i];
            bool is_rip_rel = operand.type == ZYDIS_OPERAND_TYPE_MEMORY && operand.mem.base == ZYDIS_REGISTER_RIP;
            bool is_imm_rel = operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && operand.imm.is_relative;

            if (is_rip_rel || is_imm_rel) {
                uint64_t abs_target_u64 = 0;
                ZydisCalcAbsoluteAddress(&ins.info, &operand, remote_address_from.as_addr(), &abs_target_u64);
                Ptr abs_target_ptr = abs_target_u64;
                uintptr_t rip_addr = remote_address_to.as_addr() + ins.info.length;
                int64_t new_disp = (int64_t)abs_target_ptr.as_addr() - (int64_t)rip_addr;

                if (is_rip_rel) {
                    if (ins.info.raw.disp.size != 32)
                        ZT_THROW_ERR("Encountered weird displacement size: " << ins.info.raw.disp.size);

                    ensure_displacement_reloc_bounds<int32_t>(new_disp);

                    int32_t* displacement_ptr = (int32_t*)(ins_bytes.data() + ins.info.raw.disp.offset);
                    int32_t og_displacement = *displacement_ptr;
                    *displacement_ptr = (int32_t)new_disp;
                    ZT_DLOG("Relocated rip-rel displacement from " << og_displacement << " to " << new_disp);
                }

                if (is_imm_rel) {
                    uint8_t imm_idx = operand.imm.is_relative - 1; // VERY weird, but correct, apparently...
                    void* imm_ptr = ins_bytes.data() + ins.info.raw.imm[imm_idx].offset;
                    uint8_t imm_size_bits = ins.info.raw.imm[imm_idx].size;
                    switch (imm_size_bits) {
                        case 32:
                            ensure_displacement_reloc_bounds<int32_t>(new_disp);
                            *(int32_t*)imm_ptr = (int32_t)new_disp;
                        case 16:
                            ensure_displacement_reloc_bounds<int16_t>(new_disp);
                            *(int16_t*)imm_ptr = (int16_t)new_disp;
                        case 8:
                            ensure_displacement_reloc_bounds<int8_t>(new_disp);
                            *(int8_t*)imm_ptr = (int8_t)new_disp;
                        default:
                            ZT_THROW_ERR("Encountered weird immediate size: " << imm_size_bits);
                    }
                }
            }
        }
        results.insert(results.end(), ins_bytes.begin(), ins_bytes.end());
        offset += ins.info.length;
    }

    return results;
}
