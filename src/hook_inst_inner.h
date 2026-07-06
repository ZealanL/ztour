#pragma once
#include "./mem_page.h"
#include <ztour/hook_inst.h>

#include "mutex_holder.h"

namespace ztour {
	struct HookInst::Inner {
	private:
		bool _is_installed;
	public:
		std::string name;
		std::atomic<size_t> call_counter = 0;
		Ptr target_func;
		Ptr detour_func;

		constexpr static size_t HOP_MEM_SIZE = 0x100;
		std::vector<uint8_t> patched_original_bytes;

		Ptr code_enter_detour;
		Ptr code_call_original;

		Ptr* output_original_func;

		MemPage code_page;

		Inner(const std::string &name, Ptr target_func, Ptr detour_func, Ptr *output_original_func);
		~Inner();

		bool has_target_func() const {
			return target_func != nullptr;
		}

		bool is_installed() const { return _is_installed; }
		ZT_ISOLATE_SECTION void install();
		ZT_ISOLATE_SECTION void uninstall();

		static Mutex<std::vector<HookInst *>>::Guard all_insts();
	};
}
