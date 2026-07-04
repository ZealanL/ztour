#pragma once

#include <ztour/base.h>
#include <ztour/ptr.h>

namespace ztour {
	struct HookInst {
		struct Inner;
	private:
		HookInst(Inner* inner);

		HookInst(const HookInst& other) = delete;
		HookInst& operator=(const HookInst& other) = delete;
		~HookInst() = delete;

	public:
		Inner* _inner = nullptr; // For internal library use

		bool is_installed() const;

		struct ScopeGuard {
		private:
			HookInst* _inst;
		public:
			ScopeGuard(HookInst* inst);
			ScopeGuard(const ScopeGuard& other) = delete;
			ScopeGuard& operator=(const ScopeGuard& other) = delete;
			~ScopeGuard();
		};

		static HookInst* _create(const std::string& name, Ptr target_func, Ptr detour_func, Ptr* output_original_func);
	};
}

/// All-in-one macro to define a hook (meant to be used in a .cpp file, NOT inside a function)
#define ZT_DEFINE_HOOK(name, target_func_ptr, return_type, calling_convention, args, ctx_name, body) \
namespace name { \
	return_type calling_convention outer_func args; \
	typedef decltype(&outer_func) FuncType; \
	ztour::Ptr _original_ptr = nullptr; \
	ztour::HookInst* _hook_inst = ztour::HookInst::_create( \
		#name, \
		(void*)target_func_ptr, \
		(void*)outer_func, \
		&_original_ptr \
	); \
	return_type calling_convention outer_func args { \
		const auto __scope_guard = ztour::HookInst::ScopeGuard(_hook_inst); \
		const struct { \
			ztour::Ptr return_address = ZT_GET_RET_ADDR_PTR(); \
			FuncType call_original = (FuncType)_original_ptr.as_bytes_ptr(); \
		} ctx_name; \
		auto user_code = [&]() -> return_type body; \
		return user_code(); \
	} \
}
// New line here to prevent "warning: backslash-newline at end of file"