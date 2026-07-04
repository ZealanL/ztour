#include "./decoder.h"

namespace ztour {
	Decoder::Decoder() : zy_decoder() {
#if ZT_IS_64
		ZydisDecoderInit(&zy_decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
#else
		ZydisDecoderInit(&zy_decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_STACK_WIDTH_32);
#endif
	}

	size_t Decoder::decode_instruction_length(Ptr instruction_ptr) {
		// No instruction can exceed 15 bytes under x86/x86_64
		constexpr size_t MAX_INSTRUCTION_LENGTH = 15;

		ZydisDecodedInstruction instruction;
		ZydisDecoderContext context;
		ZyanStatus status = ZydisDecoderDecodeInstruction(
			&zy_decoder, &context, instruction_ptr.as_bytes_ptr(), MAX_INSTRUCTION_LENGTH, &instruction
		);

		if (!ZYAN_SUCCESS(status)) {
			ZT_THROW_ERR("Failed to decode instruction at " << instruction_ptr);
		}

		return instruction.length;
	}
}