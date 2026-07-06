#pragma once
#include <ztour/ptr.h>
#include <zydis/Zydis.h>

namespace ztour {
	struct Decoder {
		ZydisDecoder zy_decoder;

		Decoder();

		std::vector<ZydisDecodedInstruction> decode_bytes(Ptr base, size_t amount);
		ZydisDecodedInstruction decode_instruction(Ptr instruction_ptr, size_t max_len = std::numeric_limits<size_t>::max());

		ZydisDisassembledInstruction disassemble_instruction(Ptr instruction_ptr, size_t max_len, Ptr remote_address);

		size_t decode_instruction_length(Ptr instruction_ptr, size_t max_len = std::numeric_limits<size_t>::max());
	};
}