#include "./hook_inst_inner.h"
#include "./memory.h"
#include "./encoder.h"
#include "./decoder.h"

namespace ztour {
	HookInst::Inner::Inner(const std::string& name, Ptr target_func, Ptr detour_func, Ptr* output_original_func)
	: name(name), code_page(), output_original_func(output_original_func) {
		this->_is_installed = false;
		ZT_ASSERT(name.length() > 0);
		ZT_ASSERT(detour_func != nullptr);
		ZT_ASSERT(output_original_func != nullptr);

		target_func = memory::resolve_func_ptr(target_func);
		detour_func = memory::resolve_func_ptr(detour_func);
		this->target_func = target_func;
		this->detour_func = detour_func;

		if (!target_func) {
			*output_original_func = nullptr;
			return;
		}

		ZT_DLOG("Creating detour, target=" << target_func << ", detour=" << detour_func);

		Ptr target_func_bytes = target_func;

		code_page.make_writable();

		// Calculate patch length needed, then back up those bytes
		{
			Decoder decoder = {};
			size_t patch_length = 0;
			while (patch_length < Encoder::REL_JMP_SIZE)
				patch_length += decoder.decode_instruction_length(target_func_bytes + patch_length);
			this->patched_original_bytes = memory::read_bytes_vec(target_func_bytes, patch_length);
			ZT_DLOG("To-patch function bytes: " << ZT_BYTES_TO_STR(this->patched_original_bytes));
		}

		// Copy original function bytes we will patch over
		{
			this->code_call_original = code_page.write_bytes(this->patched_original_bytes);
			// Add on a jmp back to the unpatched rest of the function
			this->code_call_original = code_page.write_bytes(
				Encoder().encode_pure_jmp(target_func_bytes + this->patched_original_bytes.size()).bytes
			);
		}

		// Make entrance trampoline (jmps to `detour_func`)
		{
			this->code_enter_detour = code_page.write_bytes(
				Encoder()
				.encode_pure_jmp(detour_func)
				.bytes
			);
		}

		code_page.make_executable();

		*output_original_func = code_call_original;
	}

	HookInst::Inner::~Inner() = default;

	void HookInst::Inner::install() {
		if (_is_installed)
			ZT_THROW_ERR("Cannot install hook \"" << name << "\" (already installed)");

		if (!has_target_func())
			ZT_THROW_ERR("Cannot install hook \"" << name << "\" because no target function has been specified yet");

		auto jmp_patch_bytes = Encoder().encode_rel_jmp(
			this->target_func,
			this->code_enter_detour
		).bytes;

		{
			size_t patch_size = this->patched_original_bytes.size();

			// Encode rel jmp to code_enter_detour
			ZT_ASSERT(jmp_patch_bytes.size() <= patch_size);
			while (jmp_patch_bytes.size() < patch_size) {
				constexpr uint8_t OPCODE_NOP = 0x90;
				jmp_patch_bytes.push_back(OPCODE_NOP);
			}
			ZT_ASSERT(jmp_patch_bytes.size() == patch_size);
			memory::overwrite_executable_mem(jmp_patch_bytes.data(), target_func, patch_size);
			ZT_DLOG("Patched: " << ZT_BYTES_TO_STR(jmp_patch_bytes));
		}
		ZT_DLOG("Installed hook (" << target_func << " -> " << detour_func << ")");
		_is_installed = true;

	}

	void HookInst::Inner::uninstall() {
		if (!_is_installed)
			ZT_THROW_ERR("Cannot uninstall hook \"" << name << "\" (not installed)");

		{
			size_t patch_size = this->patched_original_bytes.size();
			memory::overwrite_executable_mem(this->patched_original_bytes.data(), target_func, patch_size);
			ZT_DLOG("Unpatched: " << ZT_BYTES_TO_STR(this->patched_original_bytes));
		}
		ZT_DLOG("Uninstalled hook (" << target_func << " -> " << detour_func << ")");
		_is_installed = false;
	}

	// TODO: Use map instead of vector for faster by-name lookups
	Mutex<std::vector<HookInst*>>::Guard HookInst::Inner::all_insts() {
		static auto g_hook_insts = Mutex<std::vector<HookInst*>>();
		return g_hook_insts.lock();
	}
}
