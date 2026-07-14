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

HookInst* ztour::find_hook_inst(const std::string& name) {
	for (auto hook_inst : all_hook_insts())
		if (hook_inst->name() == name)
			return hook_inst;

	return nullptr;
}

std::vector<HookInst*> ztour::all_hook_insts() {
	return *HookInst::Inner::all_insts();
}

std::vector<Ptr> ztour::pattern_scan_module_multi(const std::string& module_name, const std::string& pattern_str, int32_t offset) {
	std::vector<Ptr> results = {};
	auto regions = memory::get_module_bin_regions(module_name);
	auto signed_pattern = memory::parse_pattern_str(pattern_str);
	for (auto region : regions) {
		auto new_results = memory::signed_pattern_scan_region(region, signed_pattern);
		for (Ptr found_ptr : new_results)
			results.push_back(found_ptr + offset);
	}

	return results;
}

Ptr ztour::pattern_scan_module(const std::string& module_name, const std::string& pattern_str, int32_t offset) {
	auto results = ztour::pattern_scan_module_multi(module_name, pattern_str, offset);
	if (results.size() > 1) {
		ZT_THROW_ERR("Found multiple results (" << results.size() << ") for pattern \"" << pattern_str << "\"");
	} else if (results.empty()) {
		ZT_THROW_ERR("Failed to find pattern \"" << pattern_str << "\"");
	}
	return results[0];
}