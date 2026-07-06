#include <ztour/ztour.h>

void sleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

/////////////////////

int ZT_CC_CDECL func_to_hook(int arg) {
	std::cout << "Normal function called with arg: " << arg << std::endl;
	return 0;
}
ZT_DEFINE_HOOK(
	test_hook,
	NULL,
	int, ZT_CC_CDECL, (int arg), ctx, {
		std::cout << "Detour called with arg " << arg << ", real return: " << ctx.return_address << std::endl;
		ctx.call_original(0);
		return 0;
	}
);

void ZT_CC_CDECL func_to_hook_slow() {
	std::cout << "Slow function called, being slow..." << std::endl;
	sleep(500);
	std::cout << " > Done being slow!" << std::endl;
}
ZT_DEFINE_HOOK(
	test_hook_slow,
	func_to_hook_slow,
	void, ZT_CC_CDECL, (), ctx, {
		ctx.call_original();
	}
);


///////////////////////////////////

std::vector<uint8_t> get_cur_function_bytes() {
	auto start_ptr = ztour::Ptr((void*)func_to_hook);
	auto end_ptr = start_ptr + 0x30;
	return std::vector<uint8_t>(start_ptr.as_bytes_ptr(), end_ptr.as_bytes_ptr());
}

void test_pattern_scanning() {
	const char* bytes_to_find = "\x45\x42\x31\x20\x59";

	std::cout << "Testing pattern scanning..." << std::endl;
	std::cout << "Bytes to find: \"" << bytes_to_find << "\"" << std::endl; // Printing to ensure it is compiled
	auto found_address_a = ztour::pattern_scan_module("ztour_test", "45 42 31 20 59");
	auto found_address_b = ztour::pattern_scan_module("ztour_test", "?? 45 42 31 20 59 ??", 1);
	auto found_address_c = ztour::pattern_scan_module("ztour_test", "45 42 31 ? 59");
	ZT_ASSERT(found_address_a == bytes_to_find);
	ZT_ASSERT(found_address_b == bytes_to_find);
	ZT_ASSERT(found_address_c == bytes_to_find);
}

void test_basic_hooking() {
	auto hook_inst = ztour::find_hook_inst("test_hook");
	hook_inst->change_target_func((void*)func_to_hook);

	func_to_hook(1);
	std::cout << "Function bytes before: " << ZT_BYTES_TO_STR(get_cur_function_bytes()) << std::endl;
	hook_inst->install();
	func_to_hook(2);
	std::cout << "Function bytes after: " << ZT_BYTES_TO_STR(get_cur_function_bytes()) << std::endl;
	hook_inst->uninstall();
	func_to_hook(3);
}

void test_safe_unhooking() {
	auto hook_inst = ztour::find_hook_inst("test_hook_slow");
	hook_inst->install();
	auto thread_1 = std::thread(func_to_hook_slow);
	auto thread_2 = std::thread(func_to_hook_slow);
	sleep(100);
	hook_inst->uninstall();
	func_to_hook_slow();
	std::cout << "Safe hooking works!" << std::endl;
}

int main() {
	//test_pattern_scanning();
	//test_basic_hooking();
	test_safe_unhooking();
	return EXIT_SUCCESS;
}