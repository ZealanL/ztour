#pragma once
#include <ztour/base.h>
#include <ztour/hook_inst.h>

namespace ztour {

	typedef uint32_t HookId;

	/// Installs all hook instances, returning the number installed
	size_t install_all_hooks();
	/// Uninstalls all hook instances, returning the number uninstalled
	size_t uninstall_all_hooks();

	/// Returns null if no hook matched the name
	HookInst* find_hook_inst(const std::string& name);

	/// Returns every registered hook instance
	std::vector<HookInst*> all_hook_insts();
}