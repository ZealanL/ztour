# ztour
*Ergonomic zero-dependency C++ detour hooking library for x86/x86_64, on any common OS and compiler*

____

I created this library because I was tired of the repetitive, hacky, and unintuitive nature of popular detour hooking libraries like [MinHook](https://github.com/tsudakageyu/minhook) and [Microsoft Detours](https://github.com/microsoft/detours).

In `ztour`, if you want to create a detour hook, you just do:
```cpp
ZT_DEFINE_HOOK(
	your_hook_name, // Name of this hook
	0x556C48004D8, // Address of the target function to hook (can also be a pointer)
	
	/* Return type, calling convention, args, and context identifier */
	int, ZT_CC_STDCALL, (int arg), ctx, 
	{ // Define the actual detour function
	
		std::cout << "Detour called with arg: " << arg << std::endl;
		int output = ctx.call_original(arg); // Call the original function
		return output; // Return to caller
		
	}
);
```
That's it. No duplicate definitions, no boilerplate, no calling registration functions by hand.

When you want to install and activate all your `ZT_DEFINE_HOOK` hooks, simply do:
```cpp
ztour::install_all_hooks();
```

Your defined hooks will all be installed and activated automatically, and will start working immediately.

*It's not magic, I swear...*
______

## Current features
- Automatic initialization-time hook creation
- Uninstall and reinstall specific hooks at any time
- Acquire hook instances by name at any time
- Change a hook's target functions after initializing a hook
- Initialize a hook with an unknown target function
  - *(You do this by passing `nullptr` as the target function)*
- Define hooks without having to write out the function signature more than once
- Effortless original function calling via `ctx.call_original(...)`
- Useful per-call context information such as:
  - `ctx.return_address` - Actual return address from the original function call (undisturbed by wrapping functions)
  - `ctx.stack_base_ptr` - Base pointer of the stack frame (again, ignoring wrapping functions)
- Thread-safe and recursion-compatible detour execution tracking
  - *(See `ztour::HookInst` implementation)*
- Automatic resolution of jump tables when hooking
  - *I.e. if you give a function pointer that actually points to a jump table, the library will resolve the real function address automatically and hook that underlying function*
- Integrated pattern scanning of binary modules via `ztour::pattern_scan_module(...)`
  - *Works on both Windows and Linux, and should be so fast it's irrelevant*
- Guaranteed-safe blocking hook removal (waits for hook to be unused during removal)
  - *This should prevent obnoxious crashes when unloading a module that was using detour hooks*
- Automatic "RIP-relative" instruction relocations for cloned function heads 
  - *This fixes issues where functions with RIP-relative instructions near the start are corrupted when detour libraries move the first few instructions to a new location*
  - *(See `ztour::relocations` implementation)*
## Planned features
- Complete MacOS function-patching implementation

_____

## Dependencies
There are zero external dependencies. 
A minimal version of [Zydis](https://github.com/zyantific/zydis) is included in the codebase purely for decoding instruction lengths.
Supports C++14 and higher.
