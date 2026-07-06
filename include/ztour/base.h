#pragma once
#include <ztour/base_def.h>
#include <ztour/base_inc.h>

#if ZT_IS_MSVC
#include <intrin.h>
#endif

#if ZT_IS_MSVC
#define ZT_GET_RET_ADDR_PTR() __ReturnAddress()
#else
#define ZT_GET_RET_ADDR_PTR() __builtin_return_address(0)
#endif

#if ZT_IS_MSVC
#define ZT_GET_STACK_BASE_PTR() _AddressOfReturnAddress()
#else
#define ZT_GET_STACK_BASE_PTR() __builtin_return_address(0)
#endif

#if ZT_IS_DEBUG
#define ZT_DLOG(strify_args) { std::cout << "[DBG]: " << ZT_STRIFY(strify_args) << std::endl; }
#else
#define ZT_DLOG(strify_args) {}
#endif

#define ZT_STRIFY(args) ([&]{ \
	std::ostringstream _temp_stream; \
	_temp_stream << args; \
	return _temp_stream.str(); \
}())

#define ZT_HEXSTR(int_val) ZT_STRIFY("0x" << std::hex << std::uppercase << int_val)

#define ZT_THROW_ERR(strify_args) ([&]{ \
	std::string __string = ZT_STRIFY(strify_args); \
	ZT_DLOG("FATAL ERROR: " << strify_args); \
	throw std::runtime_error(__string); \
}())

#define ZT_ASSERT(cond) { if (!(cond)) { ZT_THROW_ERR("ASSERSION FAILED: \"" << #cond << "\""); }}

#if ZT_IS_DEBUG
#define ZT_DLOG(strify_args) { std::cout << "[DBG]: " << ZT_STRIFY(strify_args) << std::endl; }
#else
#define ZT_DLOG(strify_args) {}
#endif

std::string __ztour_bytes_to_str(const std::vector<uint8_t>& v);
#define ZT_BYTES_TO_STR __ztour_bytes_to_str