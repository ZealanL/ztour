#include "./decoder.h"

namespace ztour {
	Decoder::Decoder() : zy_decoder() {
#if ZT_IS_64
		ZydisDecoderInit(&zy_decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
#else
		ZydisDecoderInit(&zy_decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_STACK_WIDTH_32);
#endif
	}

	std::vector<ZydisDecodedInstruction> Decoder::decode_bytes(Ptr base, size_t amount) {
		std::vector<ZydisDecodedInstruction> results = {};

		size_t cur_offset = 0;
		while (cur_offset < amount) {
			size_t max_left = amount - cur_offset;
			auto instruction_ptr = base + cur_offset;

			ZydisDecodedInstruction instruction;
			ZydisDecoderContext context;
			ZyanStatus status = ZydisDecoderDecodeInstruction(
				&zy_decoder, &context, instruction_ptr.as_bytes_ptr(), max_left, &instruction
			);

			if (!ZYAN_SUCCESS(status)) {
				ZT_THROW_ERR("Failed to decode instruction at " << instruction_ptr);
			}

			results.push_back(instruction);
		}

		return results;
	}

	ZydisDecodedInstruction Decoder::decode_instruction(Ptr instruction_ptr, size_t max_len) {
		constexpr size_t MAX_INSTRUCTION_LENGTH = 15;

		ZydisDecodedInstruction instruction;
		ZydisDecoderContext context;
		ZyanStatus status = ZydisDecoderDecodeInstruction(
			&zy_decoder, &context, instruction_ptr.as_bytes_ptr(), std::min(max_len, MAX_INSTRUCTION_LENGTH), &instruction
		);

		if (!ZYAN_SUCCESS(status)) {
			ZT_THROW_ERR("Failed to decode instruction at " << instruction_ptr);
		}

		return instruction;
	}

	ZydisDisassembledInstruction Decoder::disassemble_instruction(Ptr instruction_ptr, size_t max_len, Ptr remote_address) {

		ZydisDisassembledInstruction instruction;
		auto status = ZydisDisassembleIntel(
				ZYDIS_MACHINE_MODE_LONG_64,
				remote_address.as_addr(),
				instruction_ptr.as_bytes_ptr(),
				max_len,
				&instruction
			);
		if (!ZYAN_SUCCESS(status)) {
			ZT_THROW_ERR("Failed to decode instruction at " << instruction_ptr);
		}

		return instruction;
	}

	size_t Decoder::decode_instruction_length(Ptr instruction_ptr, size_t max_len) {
		return decode_instruction(instruction_ptr, max_len).length;
	}
}