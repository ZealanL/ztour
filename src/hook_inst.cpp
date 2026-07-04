#include <ztour/hook_inst.h>

#include "./mutex_holder.h"
#include "./hook_inst_inner.h"

namespace ztour {
	HookInst::HookInst(Inner *inner)
		: _inner(inner) {}

	bool HookInst::is_installed() const {
		return _inner->is_installed();
	}

	HookInst::ScopeGuard::ScopeGuard(HookInst *inst) : _inst(inst) {
		_inst->_inner->call_counter++;
	}

	HookInst::ScopeGuard::~ScopeGuard() {
		_inst->_inner->call_counter--;
	}

	HookInst* HookInst::_create(const std::string& name, Ptr target_func, Ptr detour_func, Ptr* output_original_func) {
		auto inner = new Inner(name, target_func, detour_func);
		*output_original_func = inner->code_call_original;
		auto hook_inst = new HookInst(inner);
		Inner::all_insts()->push_back(hook_inst);
		return hook_inst;
	}
}
