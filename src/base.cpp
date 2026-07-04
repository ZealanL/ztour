#include <ztour/base.h>

std::string __ztour_bytes_to_str(const std::vector<uint8_t>& bytes) {
	std::stringstream stream = {};
	for (int i = 0; i < bytes.size(); i++) {
		if (i > 0)
			stream << ' ';
		stream << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << (int)bytes[i];
	}
	return stream.str();
}