#pragma once
#include <ztour/base.h>
#include <ztour/hook_inst.h>

namespace ztour {

	typedef uint32_t HookId;

	/// Installs all hook instances, returning the number installed
	size_t install_all_hooks();
	/// Uninstalls all hook instances, returning the number uninstalled
	size_t remove_all_hooks();
}