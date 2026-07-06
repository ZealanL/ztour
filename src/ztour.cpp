#include <ztour/ztour.h>
#include "./hook_inst_inner.h"
#include "memory.h"

using namespace ztour;

size_t set_all_hooks_installed(bool install) {
	size_t num_changed = 0;
	auto insts = HookInst::Inner::all_insts();
	for (HookInst* inst : *insts) {
		if (inst->is_installed() != install) {
			if (install) {
				inst->install();
			} else {
				inst->uninstall();
			}
			num_changed++;
		}
	}
	return num_changed;
}

size_t ztour::install_all_hooks() {
	return set_all_hooks_installed(true);
}

size_t ztour::uninstall_all_hooks() {
	return set_all_hooks_installed(false);
}

HookInst* ztour::find_hook_inst(const std::string &name) {
	for (auto hook_inst : all_hook_insts())
		if (hook_inst->name() == name)
			return hook_inst;

	return nullptr;
}

std::vector<HookInst*> ztour::all_hook_insts() {
	return *HookInst::Inner::all_insts();
}

Ptr ztour::pattern_scan_module(const std::string &module_name, const std::string &pattern_str, int32_t offset) {
	auto regions = memory::get_module_bin_regions(module_name);
	auto signed_pattern = memory::parse_pattern_str(pattern_str);
	for (auto region : regions) {
		Ptr found_addr = memory::signed_pattern_scan_region(region, signed_pattern);
		if (found_addr != nullptr) {
			return found_addr.as_addr() + offset;
		}
	}

	return nullptr;
}
