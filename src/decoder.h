#pragma once
#include <ztour/ptr.h>
#include <zydis/Zydis.h>

namespace ztour {
	struct Decoder {
		ZydisDecoder zy_decoder;

		Decoder();
		size_t decode_instruction_length(Ptr instruction_ptr);
	};
}