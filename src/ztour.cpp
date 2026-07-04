#include <ztour/ztour.h>
#include "./hook_inst_inner.h"

using namespace ztour;

size_t set_all_hooks_installed(bool install) {
	size_t num_changed = 0;
	auto insts = HookInst::Inner::all_insts();
	for (HookInst* inst : *insts) {
		if (inst->is_installed() != install) {
			if (install) {
				inst->_inner->install();
			} else {
				inst->_inner->uninstall();
			}
			num_changed++;
		}
	}
	return num_changed;
}

size_t ztour::install_all_hooks() {
	return set_all_hooks_installed(true);
}

size_t ztour::remove_all_hooks() {
	return set_all_hooks_installed(false);
}