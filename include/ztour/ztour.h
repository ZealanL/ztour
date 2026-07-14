#pragma once
#include <ztour/base.h>
#include <ztour/hook_inst.h>

namespace ztour {

	typedef uint32_t HookId;

	/// Installs all uninstalled hook instances, returning the number installed
	size_t install_all_hooks();
	/// Uninstalls all installed hook instances, returning the number uninstalled
	///
	/// Notably, this function blocks until all detour functions finish running
	size_t uninstall_all_hooks();

	/// Returns null if no hook matched the name
	HookInst* find_hook_inst(const std::string& name);

	/// Returns every registered hook instance
	std::vector<HookInst*> all_hook_insts();

	/// Pattern scans a specific module in the current process, given an IDA-style byte pattern (e.g. "55 8B EC ? 24 84 ? ? 0F").
	///
	/// Returns all addresses that matched the pattern
	std::vector<Ptr> pattern_scan_module_multi(const std::string& module_name, const std::string& pattern_str, int32_t offset = 0);

	/// Pattern scans a specific module in the current process, given an IDA-style byte pattern (e.g. "55 8B EC ? 24 84 ? ? 0F").
	///
	/// Returns the one address matching the pattern, otherwise throws an exception if there were zero or multiple matches
	Ptr pattern_scan_module(const std::string& module_name, const std::string& pattern_str, int32_t offset = 0);
}