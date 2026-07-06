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
It's not magic, I swear...
______

## Current features
- Automatic initialization-time hook creation
- Uninstall and reinstall specific hooks at any time
- Change a hook's target functions after initializing a hook
- Initialize a hook with an unknown target function (via passing `nullptr`)
- Define hooks without having to write out the function signature more than once
- Effortless original function calling via `ctx.call_original(...)`
- Additional per-call information such as:
  - `ctx.return_address`
- Thread-safe and recursion-compatible detour execution tracking
- Automatic resolution of jump tables when hooking

## Planned features
- More useful per-call information in `ctx`
- Guaranteed-safe hook uninstallations via detour execution tracking
- Integrated pattern scanning (maybe?)
- Complete MacOS function-patching implementation

_____

## Dependencies
There are zero external dependencies. 
A minimal version of [Zydis](https://github.com/zyantific/zydis) is included in the codebase purely for decoding instruction lengths.
Supports C++14 and higher.
