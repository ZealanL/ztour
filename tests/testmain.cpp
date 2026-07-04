#include <ztour/ztour.h>

void sleep(uint32_t ms) {
	//ZT_DLOG("Sleeping for " << ms << "ms on thread " << std::this_thread::get_id());
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int ZT_CC_CDECL func_to_hook(int arg) {
	std::cout << "Normal function called with arg: " << arg << std::endl;
	return 0;
}

ZT_DEFINE_HOOK(
	test_hook,
	func_to_hook,
	int, ZT_CC_CDECL, (int arg), ctx, {
		std::cout << "Detour called with arg " << arg << ", real return: " << ctx.return_address << std::endl;
		ctx.call_original(0);
		return 0;
	}
);

///////////////////////////////////

std::vector<uint8_t> get_cur_function_bytes() {
	auto start_ptr = ztour::Ptr((void*)func_to_hook);
	auto end_ptr = start_ptr + 0x30;
	return std::vector(start_ptr.as_bytes_ptr(), end_ptr.as_bytes_ptr());
}

int main() {

	std::cout << "Starting thread..." << std::endl;
	std::thread thread = std::thread(
		[] {
			while (true) {
				func_to_hook(47);
				sleep(500);
			}
		}
	);
	thread.detach();

	sleep(600);
	std::cout << "Function bytes before: " << ZT_BYTES_TO_STR(get_cur_function_bytes()) << std::endl;
	ztour::install_all_hooks();
	std::cout << "Function bytes after: " << ZT_BYTES_TO_STR(get_cur_function_bytes()) << std::endl;
	sleep(1000);
	ztour::remove_all_hooks();
	sleep(1000);
	return 0;
}