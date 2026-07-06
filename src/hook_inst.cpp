#include <ztour/hook_inst.h>

#include "./mutex_holder.h"
#include "./hook_inst_inner.h"

namespace ztour {
	HookInst::HookInst(std::unique_ptr<Inner> inner)
		: _inner(std::move(inner)) {}

	HookInst::~HookInst() {
		ZT_THROW_ERR("~HookInst should never be called");
	}

	bool HookInst::has_target_func() const {
		return _inner->has_target_func();
	}

	bool HookInst::is_installed() const {
		return _inner->is_installed();
	}

	std::string HookInst::name() const {
		return _inner->name;
	}

	void HookInst::install() {
		_inner->install();
	}

	void HookInst::uninstall() {
		_inner->uninstall();
	}

	void HookInst::change_target_func(Ptr new_target_func_ptr) {
		if (this->is_installed())
			ZT_THROW_ERR("Cannot change hook's target function while installed");

		_inner = std::make_unique<Inner>(
			_inner->name, new_target_func_ptr, _inner->detour_func, _inner->output_original_func);
	}

	bool HookInst::is_being_called() const {
		return _inner->call_gate.num_active() > 0;
	}

	HookInst::ScopeGuard::ScopeGuard(HookInst *inst) : _inst(inst) {
		_inst->_inner->call_gate.on_enter_call();
	}

	HookInst::ScopeGuard::~ScopeGuard() {
		_inst->_inner->call_gate.on_exit_call();
	}

	HookInst* HookInst::_create(const std::string& name, Ptr target_func, Ptr detour_func, Ptr* output_original_func) {
		auto hook_inst = new HookInst(
			std::make_unique<Inner>(name, target_func, detour_func, output_original_func)
			);

		auto insts = Inner::all_insts();
		for (auto existing_inst : *insts)
			if (existing_inst->name() == name)
				ZT_THROW_ERR("HookInst named \"" << name << "\" already exists");

		insts->push_back(hook_inst);
		return hook_inst;
	}
}
