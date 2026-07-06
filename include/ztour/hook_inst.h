#pragma once

#include <ztour/base.h>
#include <ztour/ptr.h>

namespace ztour {
	struct HookInst {
		struct Inner;
	private:

		HookInst(std::unique_ptr<Inner> inner);
		~HookInst();

		HookInst(const HookInst& other) = delete;
		HookInst& operator=(const HookInst& other) = delete;

	public:
		std::unique_ptr<Inner> _inner; // For internal library use

		/// Whether this hook has been actually initialized to hook a specific function
		bool has_target_func() const;

		bool is_installed() const;
		std::string name() const;

		/// Install the hook instance (throws an exception if already installed)
		void install();

		/// Uninstall the hook instance (throws an exception if not installed)
		void uninstall();

		/// Change the target function address of a hook (throws an exception if already installed)
		void change_target_func(Ptr new_target_func_ptr);

		/// Checks if this detour is being used by any thread
		bool is_being_called() const;

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
		const ztour::HookInst::ScopeGuard __scope_guard(_hook_inst); \
		const struct { \
			ztour::Ptr return_address = ZT_GET_RET_ADDR_PTR(); \
			ztour::Ptr stack_base_ptr = ZT_GET_STACK_BASE_PTR(); \
			FuncType call_original = (FuncType)_original_ptr.as_bytes_ptr(); \
		} ctx_name; \
		auto user_code = [&]() -> return_type body; \
		return user_code(); \
	} \
}
// New line here to prevent "warning: backslash-newline at end of file"